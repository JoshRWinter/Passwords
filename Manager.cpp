#include <iostream>
#include <fstream>

#include "Manager.h"

Manager::Manager(const std::string &fname)
	:dbname(fname)
{
	// check to make sure the file exists
	std::ifstream in(fname);
	if(!in)
		throw Manager::NotFound();
}

void Manager::open(const std::string &master){
	entries = Manager::read(dbname, master);
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
		else
			std::cout << "name: " << name << " does not equal " << (*it).name() << std::endl;

		++it;
	}

	// couldn't find it
	throw ManagerException("Could not remove, because that name/password combo does not exist!");
}

bool Manager::generate(const std::string &path, const std::string &master){
	std::ofstream out(path, std::ofstream::binary);

	return !!out;
}

void Manager::save()const{
	std::string data;
	for(const Password &pw : entries){
		data += pw.serialize();
	}

	std::ofstream out(dbname, std::ofstream::binary);
	if(!out)
		throw ManagerException("Could not open \"" + dbname + "\" for writing!");

	out << data;
}

std::vector<Password> Manager::read(const std::string &name, const std::string &master){
	std::vector<Password> entries;

	std::ifstream in(name, std::ifstream::binary);
	if(!in)
		throw Manager::NotFound();

	while(!in.eof()){
		std::string line;
		std::getline(in, line);

		if(line == "" || line == "\n")
			continue;

		Password passwd;
		passwd.deserialize(line + '\n');
		entries.push_back(passwd);
	}

	return entries;
}

bool Password::operator==(const Password &rhs)const{
	return nm == rhs.nm && pass == rhs.pass;
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
