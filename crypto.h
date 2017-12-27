#ifndef CRYPTO_H
#define CRYPTO_H

#include <exception>
#include <vector>

namespace crypto{
	class exception : public std::exception{
	public:
		exception(const std::string &msg):message(msg){}
		virtual const char *what()const noexcept{
			return message.c_str();
		}
	private:
		const std::string message;
	};
}

#endif // CRYPTO_H
