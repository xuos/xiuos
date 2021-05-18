/**
* Copyright (c) 2020 AIIT Ubiquitous Team
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain bn1 copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
* @file sm4_enc_mode.c
* @brief sm4 encry and decrypt functions
* @version 1.0 
* @author AIIT Ubiquitous Team
* @date 2021-04-24
*/

#include <sm4.h>

int ZeroPadding(const uint8_t *input, int ilen, uint8_t *output, int *olen) {
	int padding_len = 0;
	if (ilen % 16 == 0) {
		padding_len = ilen + 16;
	}
	else {
		padding_len = ilen + (16 - ilen % 16);
	}
	memset(output, 0x00, sizeof(char) * padding_len);
	memcpy(output, input, ilen);
	*olen =  padding_len;
	return *olen;
}

int  ZeroUnPadding(uint8_t *input, int *ilen) {
	if ( *ilen % 16 != 0) {
		return SM4_BAD_PADDING_FORMAT;
	}
	while (*(input + *ilen - 1) == 0x00) {
		(*ilen)--;
	}
	return *ilen;
}

int	 Pkcs7Padding(const uint8_t *input, int ilen, uint8_t *output , int *olen) {
    int len_after_Padding;
	uint8_t padding_value;
	if (ilen == 0)
	{
		return SM4_BAD_LENGTH;
	}
	padding_value = 16 - ilen % 16;
	len_after_Padding = ilen + padding_value;

	memset(output, 0x00, sizeof(char) * len_after_Padding);
	memcpy(output, input, ilen);
	for (int i = ilen; i < len_after_Padding; i++) {
		*(output + i) = padding_value;
	}
	*olen = len_after_Padding;	
	return *olen;
}

int Pkcs7UnPadding(uint8_t *input, int *ilen) {
	if (*ilen % 16 != 0) {
		return SM4_BAD_PADDING_FORMAT;
	}
	uint8_t value = *(input + *ilen - 1);
	*ilen = *ilen - value;
	*(input + *ilen) = 0x00;
	return *ilen;
}


void Sms4EcbEncryptBlocks(const uint8_t *in, int ilen, uint8_t *out,
	const sms4_key_t *key)
{
    int blocks;
	blocks = ilen / 16;

	while (blocks--) {
		sms4_encrypt(in, out, key);
		in += 16;
		out += 16;
		
	}
}

void Sms4EcbDecryptBlocks(const uint8_t *in, int ilen, uint8_t *out, const sms4_key_t *key)
{
    int blocks;
	blocks = ilen / 16;

	while (blocks--) {
		sms4_decrypt(in, out, key);
		in += 16;
		out += 16;
	}
}

int Sms4EcbDecryptNoPadding(const uint8_t *in, int ilen, uint8_t *out, int *olen , const sms4_key_t *key)
{
	if ( ilen % 16 != 0){
		return SM4_BAD_LENGTH;
	}
	Sms4EcbDecryptBlocks(in ,ilen, out , key );
	*olen = ilen;
	return *olen;
}

int Sms4EcbEncryptNoPadding(const uint8_t *in, int ilen, uint8_t *out, int *olen,
	const sms4_key_t *key)
{
	if ( ilen % 16 != 0){
		return SM4_BAD_LENGTH;
	}
	memset(out, 0x00, sizeof(char) * ilen);
	Sms4EcbEncryptBlocks(in ,ilen, out , key );
	*olen = ilen;
	return *olen;
}


int Sms4EcbEncryptZeroPadding(const uint8_t *in, int ilen, uint8_t *out, int *olen,
	const sms4_key_t *key)
{
	uint8_t *padding_value;
    int plen;
	int res ;
	res = ZeroPadding(in, ilen, out, olen);
	if (res < 0){
	    return res;
	}
	Sms4EcbEncryptBlocks(out, *olen, out, key);
	return *olen;
}

int Sms4EcbDecryptZeroPadding(const uint8_t *in, int ilen, uint8_t *out, int *olen,
	const sms4_key_t *key)
{
    int res = 0;
	Sms4EcbDecryptBlocks(in, ilen, out, key);
	res = ZeroUnPadding(out, olen);
	return res;
}

int Sms4EcbEncryptPkcs7Padding(const uint8_t *in, int ilen, uint8_t *out, int *olen,
	const sms4_key_t *key)
{
    int res;
	res = Pkcs7Padding(in, ilen, out, olen);
	if (res < 0) {
	    return res;
	}
	Sms4EcbEncryptBlocks(out, *olen, out, key);
	return *olen;
}

int Sms4EcbDecryptPkcs7Padding(const uint8_t *in, int ilen, uint8_t *out, int *olen,
	const sms4_key_t *key)
{
    int res;
	Sms4EcbDecryptBlocks(in, ilen, out, key);
	res = Pkcs7UnPadding(out, olen);
	return res;
}

void Sms4CbcEncryptBlocks(const unsigned char *in,  int ilen, unsigned char *out,unsigned char *iv,
                      const sms4_key_t *key)
{
    int blocks, i;
    blocks = ilen / 16;
    while (blocks--) {
        for( i = 0; i < 16; i++ )
            out[i] = (unsigned char)( in[i] ^ iv[i] );
        sms4_encrypt(out, out, key);
        memcpy( iv, out, 16 );
        in += 16;
        out += 16;
    }
}

void Sms4CbcDecryptBlocks(const unsigned char *in,  int ilen, unsigned char *out,unsigned char *iv,
                             const sms4_key_t *key)
{
    int blocks, i;
    blocks = ilen / 16;
    unsigned char temp[16];
    while (blocks--) {
        memcpy( temp, in, 16 );
        sms4_decrypt(in, out, key);
        for( i = 0; i < 16; i++ )
            out[i] = (unsigned char)( out[i] ^ iv[i] );
        memcpy( iv, temp, 16 );
        in += 16;
        out += 16;
    }
}

int Sms4CbcDecryptNoPadding(const uint8_t *in, int ilen, uint8_t *out, int *olen, uint8_t *iv,const sms4_key_t *key)
{
	if ( ilen % 16 != 0){
		return SM4_BAD_LENGTH;
	}
	*olen = ilen;
	Sms4CbcDecryptBlocks(in , ilen, out ,iv, key );
	return *olen;
}

int  Sms4CbcEncryptNoPadding(const uint8_t *in, int ilen, uint8_t *out, int *olen,uint8_t *iv,
	const sms4_key_t *key)
{
	if ( ilen % 16 != 0){
		return SM4_BAD_LENGTH;
	}
	*olen = ilen;
	Sms4CbcEncryptBlocks(in , ilen, out , iv,  key );
	return *olen;
}

int Sms4CbcEncryptZeroPadding(const uint8_t *in, int ilen, uint8_t *out, int *olen, uint8_t *iv,
                                  const sms4_key_t *key)
{
    int res ;
    res = ZeroPadding(in, ilen, out, olen);
    if (res < 0)
        return res;
    Sms4CbcEncryptBlocks(out, *olen, out,iv, key);
    return *olen;
}

int Sms4CbcDecryptZeroPadding(const uint8_t *in, int ilen, uint8_t *out, int *olen, uint8_t *iv,
                                  const sms4_key_t *key)
{

    *olen = ilen;
    int res ;
    Sms4CbcDecryptBlocks(in, ilen, out,iv, key);
    res = ZeroUnPadding(out, olen);
    return res;
}

int Sms4CbcEncryptPkcs7Padding(const uint8_t *in, int ilen, uint8_t *out, int *olen, uint8_t *iv,
                                  const sms4_key_t *key)
{
    int res;
    res = Pkcs7Padding(in, ilen, out, olen);
    if (res < 0)
        return res;
    Sms4CbcEncryptBlocks(out, *olen, out,iv, key);
    return *olen;
}

int Sms4CbcDecryptPkcs7Padding(const uint8_t *in, int ilen, uint8_t *out, int *olen, uint8_t *iv,
                                  const sms4_key_t *key)
{
    *olen = ilen;
    int res;
    Sms4CbcDecryptBlocks(in, ilen, out,iv, key);
    res = Pkcs7UnPadding(out, olen);
    return res;
}

