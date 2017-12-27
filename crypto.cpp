#include <array>
#include <iostream>

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <string.h>

#include "crypto.h"

#define DEBUG(x) (std::string("[ ") + __FILE__ + ": " + __func__ + ": " + std::to_string(__LINE__) + "] " + x)

// back-end encrypt & decrypt functions
static void scramble(const std::vector<unsigned char>&, const std::array<unsigned char, 32>&, const std::array<unsigned char, 16>&, std::vector<unsigned char>&);
//static void unscramble(const std::vector<unsigned char>&, const std::array<unsigned char, 32>&, const std::array<unsigned char, 16>&, std::vector<unsigned char>&);

// turn passphrase into raw key
static void stretch(const std::string &pass, std::array<unsigned char, 32> &bytes, std::array<unsigned char, 16> &iv){
	const int ret = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), NULL, (unsigned char*)pass.c_str(), pass.length(), 1, &bytes[0], &iv[0]);
	if(ret == 0)
		throw crypto::exception("Could not stretch the key");
}

namespace crypto{
// encryption interface
void encrypt(const std::string &password, const std::vector<unsigned char> &plaintext, std::vector<unsigned char> &ciphertext){
	std::array<unsigned char, 32> key;
	std::array<unsigned char, 16> iv; // init vector
	memset(&key[0], 0, key.size());
	memset(&iv[0], 0, iv.size());

	// compute the key and iv
	stretch(password, key, iv);

	const int cipherlen = plaintext.size() + 256;
	ciphertext.resize(cipherlen);

	scramble(plaintext, key, iv, ciphertext);
}}

void scramble(const std::vector<unsigned char> &plaintext, const std::array<unsigned char, 32> &key, const std::array<unsigned char, 16> &iv, std::vector<unsigned char> &ciphertext){
	EVP_CIPHER_CTX *ctx;

	// construct the context
	if(!(ctx = EVP_CIPHER_CTX_new()))
		throw crypto::exception(DEBUG("couldn't construct evp cipher context"));

	// init the encryption operation
	if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, &key[0], &iv[0]))
		throw crypto::exception(DEBUG("couldn't init evp cipher operation"));

	// encryption block loop
	int cipherlen = 0; // length of ciphertext
	int len;
	for(std::vector<unsigned char>::size_type i = 0; i < plaintext.size(); i += 256){
		if(1 != EVP_EncryptUpdate(ctx, &ciphertext[i], &len, &plaintext[i], plaintext.size() - i))
			throw crypto::exception(DEBUG("couldn't encrypt " + std::to_string(i) + " block"));

		cipherlen += len;
		std::cout<<"len is "<<len<<std::endl;
	}

	// finalize the encryption operation
	if(1 != EVP_EncryptFinal_ex(ctx, &ciphertext[cipherlen], &len))
		throw crypto::exception(DEBUG("couldn't finalize the encryption operation"));
	cipherlen += len;
	ciphertext.resize(cipherlen);

	// finalize the evp context
	EVP_CIPHER_CTX_free(ctx);
}

/*
void unscramble(const std::vector<unsigned char> &ciphertext, const std::array<unsigned char, 32> &key, const std::array<unsigned char, 16> &iv, std::vector<unsigned char> &plaintext){
}
*/

#ifdef CRYPTO_TEST
#include <fstream>
#include <iostream>
static void save(const std::vector<unsigned char> &data){
	std::ofstream out("/home/josh/crypto_out.txt", std::ofstream::binary);
	if(!out)
		throw 0;

	out.write((char*) &data[0], data.size());
}
int main(){
	const std::string raw = "ayy there hello my name is joshh";
	std::vector<unsigned char> plain;
	plain.resize(raw.length());
	memcpy(&plain.at(0), &raw.at(0), raw.size());

	std::vector<unsigned char> cipher;

	crypto::encrypt("pooperino", plain, cipher);
	std::cout<<"size is "<<cipher.size()<<std::endl;
	save(cipher);

	return 0;
}
#endif // CRYPTO_TEST
