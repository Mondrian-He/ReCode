#ifndef _ENCODE_
#define _ENCODE_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

static const string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static inline bool is_base64(unsigned char c);
string base64_encode(char const* bytes_to_encode, int in_len);
string base64_decode(string & encoded_string);
string aes_256_cbc_encode(const string& password,const string& iv, const string& data);
string aes_256_cbc_decode(const string& password,const string& iv, const string& strData);

#endif