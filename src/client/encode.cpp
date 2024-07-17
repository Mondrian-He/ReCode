#include "encode.h"

#include <string.h>
#include <openssl/aes.h>

static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

string base64_encode(char const* bytes_to_encode, int in_len) {
	string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';

	}

	return ret;

}

string base64_decode(string & encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	string ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

string aes_256_cbc_encode(const string& password,const string& iv, const string& data)
{
	unsigned char ivv[AES_BLOCK_SIZE];
    memcpy(ivv,iv.c_str(),AES_BLOCK_SIZE);
	AES_KEY aes_key;
	if (AES_set_encrypt_key((const unsigned char*)password.c_str(), password.length() * 8, &aes_key) < 0)
	{
		return "";
	}
	string strRet;
	string data_bak = data;
	unsigned int data_length = data_bak.length();

	// ZeroPadding
	int padding = 0;
	if (data_bak.length() % (AES_BLOCK_SIZE) > 0)
	{
		padding = AES_BLOCK_SIZE - data_bak.length() % (AES_BLOCK_SIZE);
	}
	
	data_length += padding;
	while (padding > 0)
	{
		data_bak += '\0';
		padding--;
	}

	for (unsigned int i = 0; i < data_length / (AES_BLOCK_SIZE); i++)
	{
		string str16 = data_bak.substr(i*AES_BLOCK_SIZE, AES_BLOCK_SIZE);
		unsigned char out[AES_BLOCK_SIZE];
		memset(out, 0, AES_BLOCK_SIZE);
		AES_cbc_encrypt((const unsigned char*)str16.c_str(), out, AES_BLOCK_SIZE, &aes_key, ivv, AES_ENCRYPT);
		strRet += string((const char*)out, AES_BLOCK_SIZE);
	}
	return strRet;
}

string aes_256_cbc_decode(const string& password,const string& iv, const string& strData)
{
	unsigned char ivv[AES_BLOCK_SIZE];
    memcpy(ivv,iv.c_str(),AES_BLOCK_SIZE);

	AES_KEY aes_key;
	if (AES_set_decrypt_key((const unsigned char*)password.c_str(), password.length() * 8, &aes_key) < 0)
	{
		return "";
	}
	string strRet;
	for (unsigned int i = 0; i < strData.length() / AES_BLOCK_SIZE; i++)
	{
		string str16 = strData.substr(i*AES_BLOCK_SIZE, AES_BLOCK_SIZE);
		unsigned char out[AES_BLOCK_SIZE];
		memset(out, 0, AES_BLOCK_SIZE);
		AES_cbc_encrypt((const unsigned char*)str16.c_str(), out, AES_BLOCK_SIZE, &aes_key, ivv, AES_DECRYPT);
		strRet += string((const char*)out, AES_BLOCK_SIZE);
	}
	return strRet;
}