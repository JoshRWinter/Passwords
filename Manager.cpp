#include <fstream>
#include <cctype>

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "Manager.h"
#include "crypto.h"

#define AMERICAN_ENGLISH_BYTES 102400

Manager::Manager(const std::string &fname)
	:dbname(fname)
	,words("american-english", std::ifstream::binary)
{
	// check to make sure the file exists
	std::ifstream in(fname);
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

void Manager::edit(const std::string &name, const std::string &newname, const std::string &newpass){
	// find it
	for(Password &pass : entries){
		if(name == pass.name()){
			pass.set_name(newname);
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
	std::ofstream out(path, std::ofstream::binary);
	if(!out)
		throw Manager::ManagerException("Could not open " + path + " in write mode!");

	Manager m(path);
	m.master(master);
	m.save();
}

void Manager::save()const{
	std::string data = "passwordsdb\n";
	for(const Password &pw : entries){
		data += pw.serialize();
	}

	std::vector<unsigned char> raw;
	std::vector<unsigned char> ciphertext;
	raw.resize(data.length());
	memcpy(&raw[0], data.c_str(), data.length());
	try{
		crypto::encrypt(masterp, raw, ciphertext);
	}catch(const crypto::exception&){
		throw Corrupt();
	}

	// checksum
	unsigned long long checksum = 0;
	for(const auto c : ciphertext)
		checksum += c;

	std::ofstream out(dbname, std::ofstream::binary);
	if(!out)
		throw ManagerException("Could not open \"" + dbname + "\" for writing!");

	out.write((char*)&checksum, sizeof(checksum));
	out.write((char*)&ciphertext[0], ciphertext.size());
}

std::vector<Password> Manager::read(const std::string &name, const std::string &master){
	std::vector<Password> entries;

	std::ifstream in(name, std::ifstream::binary | std::ifstream::ate);
	if(!in)
		throw Manager::NotFound();
	const int filelen = in.tellg();
	in.seekg(0);

	std::vector<unsigned char> raw;
	raw.resize(filelen - sizeof(unsigned long long));

	unsigned long long checksum;
	in.read((char*)&checksum, sizeof(checksum));
	in.read((char*)&raw[0], filelen - sizeof(checksum));

	// validate checksum
	unsigned long long chk = 0;
	for(const auto c : raw)
		chk += c;
	if(chk != checksum)
		throw Corrupt();

	// decrypt
	std::vector<unsigned char> plaintextdata;
	try{
		crypto::decrypt(master, raw, plaintextdata);
	}catch(const crypto::exception&){
		throw IncorrectPassword();
	}
	plaintextdata.push_back(0);
	std::string csv = (char*)&plaintextdata[0];

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

bool Password::operator==(const Password &rhs)const{
	return nm == rhs.nm && pass == rhs.pass;
}

bool Password::operator<(const Password &rhs)const{
	return tolower(nm.at(0)) < tolower(rhs.nm.at(0));
}

std::string Password::name()const{
	return nm;
}

std::string Password::password()const{
	return pass;
}

void Password::set_name(const std::string &n){
	nm = n;
}

void Password::set_password(const std::string &p){
	pass = p;
}

std::string Password::serialize()const{
	const std::string field_name = Password::escape(nm);
	const std::string field_pass = Password::escape(pass);

	return field_name + "," + field_pass + "\n";
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
