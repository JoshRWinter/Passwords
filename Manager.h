#ifndef MANAGER_H
#define MANAGER_H

#include <exception>
#include <vector>

class Password{
public:
	std::string name()const;
	std::string password()const;
	void set_name(const std::string&);
	void set_password(const std::string&);
	std::string serialize()const;
	void deserialize(const std::string&);

private:
	static std::string escape(const std::string&);

	std::string nm;
	std::string pass;
};

class Manager{
public:
	Manager(const std::string&);
	void open(const std::string&);
	static bool generate(const std::string&, const std::string &master);

private:
	static std::vector<Password> read(const std::string&, const std::string&);

	const std::string dbname;
	std::vector<Password> entries;

public:
	class NotFound:public std::exception{
	public:
		virtual const char *what()const noexcept{
			return "the requested file does not exist";
		}
	};

	class Corrupt:public std::exception{
	public:
		virtual const char *what()const noexcept{
			return "file is corrupt";
		}
	};
};

#endif // MANAGER_H
