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

bool Manager::generate(const std::string &path){
	std::ofstream out(path, std::ofstream::binary);

	return !!out;
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
					nm = line.substr(start, i - start);
					break;
				case 1:
					pass = line.substr(start, i - start);
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

		if(c == ','){
			escaped.insert(escaped.begin() + i, '\\');
			++i;
			continue;
		}
	}

	return escaped;
}
