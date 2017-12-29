#ifndef MANAGER_H
#define MANAGER_H

#include <exception>
#include <vector>
#include <fstream>

class Password{
public:
	bool operator==(const Password&)const;
	bool operator<(const Password&)const;
	std::string name()const;
	std::string password()const;
	void set_name(const std::string&);
	void set_password(const std::string&);
	std::string serialize()const;
	void deserialize(const std::string&);

private:
	static std::string escape(const std::string&);
	static std::string strip(const std::string&);

	std::string nm;
	std::string pass;
};

class Manager{
public:
	Manager(const std::string&);
	Manager(const Manager&) = delete;
	void open(const std::string&);
	const std::vector<Password> &get()const;
	void add(const Password&);
	const Password &find(const std::string&)const;
	void edit(const std::string&, const std::string&, const std::string&);
	void remove(const std::string&);
	void master(const std::string&);
	std::string get_master()const;
	std::string gen_memorable();
	static std::string gen_random();
	static void generate(const std::string&, const std::string &master);

private:
	void save()const;
	void write(const std::string&)const;
	std::string getword();
	static std::vector<Password> read(const std::string&, const std::string&);
	static std::string getline(std::string&);
	static std::string real_db_path(const std::string&);
	static std::vector<std::string> get_backups(const std::string&);

	const std::string dbname;
	const std::string dbdir;
	std::string masterp;
	std::vector<Password> entries;
	std::ifstream words;

public:
	class IncorrectPassword:public std::exception{
	public:
		virtual const char *what()const noexcept{
			return "incorrect master password";
		}
	};

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

	class ManagerException:public std::exception{
	public:
		ManagerException(const std::string &error)
			:reason(error){}
		virtual const char *what()const noexcept{
			return reason.c_str();
		}
	private:
		const std::string reason;
	};
};

#endif // MANAGER_H
