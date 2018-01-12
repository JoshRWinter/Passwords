#include <fstream>
#include <cctype>

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <QDir>
#include <QDate>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif // _WIN32

#include "Manager.h"
#include "crypto.h"

#define AMERICAN_ENGLISH_BYTES 102400

#ifdef _WIN32
#include <windows.h>
static void makefolder(const std::string &name){
	CreateDirectory(name.c_str(), NULL);
}
static std::string get_resource_dir(){
	char path[MAX_PATH];
	ExpandEnvironmentStrings("%USERPROFILE%\\Documents\\PasswordsDB", path, MAX_PATH - 1);

	return path;
}
#else
#include <sys/stat.h>
#include <sys/types.h>
static void makefolder(const std::string &name){
	mkdir(name.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
}
static std::string get_resource_dir(){
	return "/usr/share/Passwords";
}
#endif // _WIN32

Manager::Manager(const std::string &fname)
	:dbname(Manager::real_db_path(fname))
	,dbdir(fname)
	,words(get_resource_dir() + "/american-english", std::ifstream::binary)
{
	// make the folders
	makefolder(fname);

	// check to make sure the file exists
	std::ifstream in(dbname);
	if(!in)
		throw Manager::NotFound();

	srand(time(NULL));
}

void Manager::open(const std::string &mp){
	masterp = mp;
	entries = Manager::read(dbname, masterp);
}

const std::vector<Password> &Manager::get()const{
	return entries;
}

void Manager::add(const Password &pw){
	// make sure it doesn't already exist
	for(const Password &pass : entries){
		if(pass.name() == pw.name())
			throw ManagerException("There is already an entry for \"" + pass.name() + "\" in the database!");
	}

	entries.push_back(pw);
	save();
}

const Password &Manager::find(const std::string &name)const{
	for(const Password &pass : entries){
		if(pass.name() == name)
			return pass;
	}

	throw ManagerException("Could not find a password with name \"" + name + "\"");
}

void Manager::edit(const std::string &name, const std::string &newname, const std::string &newusrname, const std::string &newpass){
	// find it
	for(Password &pass : entries){
		if(name == pass.name()){
			pass.set_name(newname);
			pass.set_username(newusrname);
			pass.set_password(newpass);
			save();
			return;
		}
	}

	// couldn't find it
	throw ManagerException("Could not edit, because that name/password combo does not exist!");
}

void Manager::remove(const std::string &name){
	// find it
	for(auto it = entries.begin(); it != entries.end();){
		if(name == (*it).name()){
			it = entries.erase(it);
			save();
			return;
		}

		++it;
	}

	// couldn't find it
	throw ManagerException("Could not remove, because that name/password combo does not exist!");
}

void Manager::master(const std::string &mp){
	masterp = mp;
	save();
}

std::string Manager::get_master()const{
	return masterp;
}

std::string Manager::gen_random(){
	std::string r;

	for(int i = 0; i < 25; ++i)
		r.push_back('!' + (rand() % ('~' - '!' + 1)));

	return r;
}

std::string Manager::gen_memorable(){
	if(!words)
		throw ManagerException("Could not open the words file");

	std::string phrase;
	for(int i = 0; i < 4; ++i)
		phrase += getword();

	return phrase;
}

void Manager::generate(const std::string &path, const std::string &master){
	{
		std::ofstream out(Manager::real_db_path(path), std::ofstream::binary);
		if(!out)
			throw Manager::ManagerException("Could not open " + Manager::real_db_path(path) + " in write mode!");
	}

	Manager m(path);
	m.master(master);
}

void Manager::save()const{
	const std::vector<std::string> &backups = get_backups(dbdir);

	const QDate &now = QDate::currentDate();

	bool today = false;
	// entries will be of the form YYYY_MM_DD.backup
	for(const std::string &e : backups){
		std::string entry;
		for(const char c : e){
			if(c == '_')
				entry.push_back(' ');
			else
				entry.push_back(c);
		}

		int year;
		int month;
		int day;

		if(3 != sscanf(entry.c_str(), "%d %d %d", &year, &month, &day))
			continue;

		if(now.day() == day && now.month() == month && now.year() == year){
			today = true;
			break;
		}
	}

	if(!today){
		const std::string name = std::to_string(now.year()) + "_" + std::to_string(now.month()) + "_" + std::to_string(now.day()) + ".backup";
		QDir dir(dbdir.c_str());
		if(dir.exists("db") && !dir.rename("db", name.c_str()))
			throw ManagerException("could not move \"db\" to \"" + dbdir + "/" + name + "\"");
	}

	write(dbname);
}

void Manager::write(const std::string &file)const{
	std::string data = "passwordsdb\n";
	for(const Password &pw : entries){
		data += pw.serialize();
	}

	// compile the data
	std::vector<unsigned char> raw;
	std::vector<unsigned char> ciphertext;
	raw.resize(data.length());
	memcpy(raw.data(), data.c_str(), data.length());

	// plaintext checksum
	unsigned long long plain_checksum = 0;
	for(const auto c : data)
		plain_checksum += c;

	// encrypt
	try{
		crypto::encrypt(masterp, raw, ciphertext);
	}catch(const crypto::exception&){
		throw Corrupt();
	}

	// ciphertext checksum
	unsigned long long cipher_checksum = 0;
	for(const auto c : ciphertext)
		cipher_checksum += c;

	std::ofstream out(file, std::ofstream::binary);
	if(!out)
		throw ManagerException("Could not open \"" + file + "\" for writing!");

	out.write((char*)&cipher_checksum, sizeof(cipher_checksum)); // write the ciphertext checksum
	out.write((char*)&plain_checksum, sizeof(plain_checksum)); // write the plaintext checksum
	out.write((char*)ciphertext.data(), ciphertext.size());
}

std::string Manager::getword(){
	words.seekg(rand() % AMERICAN_ENGLISH_BYTES);

	std::string word;

	// read until first newline
	char c = '.';
	while(c != '\n')
		words.read(&c, 1);

	for(;;){
		words.read(&c, 1);

		if(c == '\n'){
			if(word.length() == 0)
				continue;
			break;
		}

		word.push_back(c);
	}

	return word;
}

std::vector<Password> Manager::read(const std::string &name, const std::string &master){
	std::vector<Password> entries;

	const long long filelen = Manager::filesize(name);
	if(filelen == 0)
		throw Corrupt();
	std::ifstream in(name, std::ifstream::binary);
	if(!in)
		throw Manager::NotFound();

	std::vector<unsigned char> raw;
	raw.resize(filelen - sizeof(unsigned long long) - sizeof(unsigned long long));

	unsigned long long cipher_checksum;
	unsigned long long plain_checksum;
	in.read((char*)&cipher_checksum, sizeof(cipher_checksum));
	in.read((char*)&plain_checksum, sizeof(plain_checksum));
	in.read((char*)raw.data(), filelen - sizeof(cipher_checksum) - sizeof(plain_checksum));

	// validate cipher checksum
	unsigned long long chk = 0;
	for(const auto c : raw)
		chk += c;
	if(chk != cipher_checksum)
		throw Corrupt();

	// decrypt
	std::vector<unsigned char> plaintextdata;
	try{
		crypto::decrypt(master, raw, plaintextdata);
	}catch(const crypto::exception&){
		throw IncorrectPassword();
	}
	plaintextdata.push_back(0);
	std::string csv = (char*)plaintextdata.data();

	// validate cipher checksum
	chk = 0;
	for(const auto c : plaintextdata)
		chk += c;
	if(chk != plain_checksum)
		throw IncorrectPassword();

	if(Manager::getline(csv) != "passwordsdb")
		throw IncorrectPassword();

	while(csv.length() > 0){
		std::string line = Manager::getline(csv);

		if(line == "" || line == "\n")
			continue;

		Password passwd;
		passwd.deserialize(line + '\n');
		entries.push_back(passwd);
	}

	return entries;
}

std::string Manager::getline(std::string& stream){
	std::string line;

	for(unsigned i = 0; i < stream.length(); ++i){
		const char c = stream.at(i);

		if(c == '\n'){
			line = stream.substr(0, i);
			stream.erase(0, i + 1);
			return line;
		}
	}

	throw Corrupt();
}

std::string Manager::real_db_path(const std::string &path){
	return path + "/db";
}

std::vector<std::string> Manager::get_backups(const std::string &dir){
	QDir directory(dir.c_str());

	std::vector<std::string> backups;

	auto list = directory.entryList();
	for(const auto &entry : list){
		if(entry != "db" && entry != "." && entry != "..")
			backups.push_back(entry.toStdString());
	}

	return backups;
}

// get filesize
long long Manager::filesize(const std::string &fname){
#ifdef _WIN32
	HANDLE file = CreateFile(fname.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(file == INVALID_HANDLE_VALUE)
		throw ManagerException("could not stat file \"" + fname + "\" (" + std::to_string(GetLastError()) + ")");

	LARGE_INTEGER li;
	GetFileSizeEx(file, &li);
	CloseHandle(file);
	return li.QuadPart;
#else
	struct stat buff;
	if(0 != stat(fname.c_str(), &buff))
		throw ManagerException("could not stat file \"" + fname + "\"");
	return buff.st_size;
#endif // _WIN32
}

bool Password::operator==(const Password &rhs)const{
	return nm == rhs.nm && pass == rhs.pass;
}

bool Password::operator<(const Password &rhs)const{
	return tolower(nm.at(0)) < tolower(rhs.nm.at(0));
}

std::string Password::name()const{
	return nm;
}

std::string Password::username()const{
	return un;
}

std::string Password::password()const{
	return pass;
}

void Password::set_name(const std::string &n){
	nm = n;
}

void Password::set_username(const std::string &u){
	un = u;
}

void Password::set_password(const std::string &p){
	pass = p;
}

std::string Password::serialize()const{
	const std::string field_name = Password::escape(nm);
	const std::string field_pass = Password::escape(pass);
	const std::string field_usrnm = Password::escape(un);

	return field_name + "," + field_usrnm + "," + field_pass + "\n";
}

void Password::deserialize(const std::string &line){
	int field = 0; // current field
	int start = 0; // starting index of the current field

	try{

		for(unsigned i = 0; i < line.length(); ++i){
			const char c = line.at(i);

			if(c == '\\'){
				++i;
				continue;
			}
			else if(c == ',' || c == '\n'){
				// split
				switch(field){
				case 0:
					nm = Password::strip(line.substr(start, i - start));
					break;
				case 1:
					un = Password::strip(line.substr(start, i - start));
					break;
				case 2:
					pass = Password::strip(line.substr(start, i - start));
					break;
				}

				++field;
				start = i + 1;
			}
		}
	}catch(const std::out_of_range &e){
		throw Manager::Corrupt();
	}
}

std::string Password::escape(const std::string &field){
	std::string escaped = field;

	for(unsigned i = 0; i < escaped.length(); ++i){
		const char c = escaped.at(i);

		if(c == ',' || c == '\\'){
			escaped.insert(escaped.begin() + i, '\\');
			++i;
			continue;
		}
	}

	return escaped;
}

// strip escapes
std::string Password::strip(const std::string &field){
	std::string stripped = field;
	for(unsigned i = 0; i < stripped.size(); ++i){
		if(stripped.at(i) == '\\'){
			stripped.erase(stripped.begin() + i);
			continue;
		}
	}

	return stripped;
}
