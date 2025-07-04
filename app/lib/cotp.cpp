#include "cotp.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <inttypes.h>

#include <openssl/rand.h>

const char* OTPType_asString(OTPType type)
{
	switch (type)
	{
		case OTP: return "OTP";
		case TOTP: return "TOTP";
		case HOTP: return "HOTP";
	}
	return NULL;
}


OTPData* otp_new(OTPData* data, const char* base32_secret, COTP_ALGO algo, uint32_t digits)
{
	data->digits = digits ? digits : 6;
	data->interval = 0;
	data->count = 0;
	
	data->method = OTP;
	data->algo = algo;
	data->time = NULL;
	
	data->base32_secret = &base32_secret[0];
	
	return data;
}

OTPData* totp_new(OTPData* data, const char* base32_secret, COTP_ALGO algo, COTP_TIME time, uint32_t digits, uint32_t interval)
{
	OTPData* tdata = otp_new(data, base32_secret, algo, digits);
	tdata->interval = interval;
	tdata->time = time;
	tdata->method = TOTP;
	
	return data;
}

OTPData* hotp_new(OTPData* data, const char* base32_secret, COTP_ALGO algo, uint32_t digits, uint64_t count)
{
	OTPData* hdata = otp_new(data, base32_secret, algo, digits);
	hdata->method = HOTP;
	hdata->count = count;
	
	return data;
}

void otp_free(OTPData* data)
{
	free(data);
}

COTPRESULT otp_byte_secret(OTPData* data, char* out_str) {
	if (out_str == NULL || strlen(data->base32_secret) % 8 != 0) {
		return OTP_ERROR;
	}
	
	size_t base32_length = strlen(data->base32_secret);
	size_t num_blocks = base32_length / 8;
	size_t output_length = num_blocks * 5;
	
	if (output_length == 0) {
		return OTP_OK;
	}
	
	int valid = 1;
	
	for (size_t i = 0; i < num_blocks; i++) {
		unsigned int block_values[8] = { 0 };
		
		for (int j = 0; j < 8; j++) {
			char c = data->base32_secret[i * 8 + j];
			unsigned int value = (unsigned char) c < 256 ? OTP_DEFAULT_BASE32_OFFSETS[(unsigned char) c] : -1;
			block_values[j] = value & 31;
			valid &= (value >= 0);
		}
		
		out_str[i * 5] = (block_values[0] << 3) | (block_values[1] >> 2);
		out_str[i * 5 + 1] = (block_values[1] << 6) | (block_values[2] << 1) | (block_values[3] >> 4);
		out_str[i * 5 + 2] = (block_values[3] << 4) | (block_values[4] >> 1);
		out_str[i * 5 + 3] = (block_values[4] << 7) | (block_values[5] << 2) | (block_values[6] >> 3);
		out_str[i * 5 + 4] = (block_values[6] << 5) | block_values[7];
	}

	return valid ? OTP_OK : OTP_ERROR;
}

COTPRESULT otp_num_to_bytestring(uint64_t integer, char* out_str)
{
	if (out_str == NULL)
		return OTP_ERROR;
	
	size_t i = 7;
	while  (integer != 0)
	{
		out_str[i] = integer & 0xFF;
		i--;
		integer >>= 8;
	}
	
	return OTP_OK;
}

COTPRESULT otp_random_base32(size_t len, char* out_str)
{
	if (out_str == NULL)
		return OTP_ERROR;
	
	len = len > 0 ? len : 16;
	
	unsigned char* rand_buffer = (unsigned char*)malloc(len);
	if (rand_buffer == NULL)
		return OTP_ERROR;
	if (RAND_bytes(rand_buffer, len) != 1)
		return OTP_ERROR;
	
	for (size_t i=0; i<len; i++)
	{
		out_str[i] = OTP_DEFAULT_BASE32_CHARS[rand_buffer[i] % 32];
	}
	
	return OTP_OK;
}

COTPRESULT totp_compare(OTPData* data, const char* key, int64_t offset, uint64_t for_time)
{
	if (key == NULL || data == NULL)
		return OTP_ERROR;

	char* time_str = (char*)malloc(data->digits + 1);
	if (time_str == NULL)
		return OTP_ERROR;  

	memset(time_str, 0, data->digits + 1);

	if (totp_at(data, for_time, offset, time_str) == 0) {
		free(time_str);
		return OTP_ERROR;
	}

	int invalid = 0;
	for (size_t i = 0; i < data->digits; i++) {
		invalid |= key[i] ^ time_str[i];
	}

	free(time_str); 

	if (invalid != 0)
		return OTP_ERROR;

	return OTP_OK;
}

COTPRESULT totp_at(OTPData* data, uint64_t for_time, int64_t offset, char* out_str)
{
	return otp_generate(data, totp_timecode(data, for_time) + offset, out_str);
}

COTPRESULT totp_now(OTPData* data, char* out_str)
{
	return otp_generate(data, totp_timecode(data, data->time()), out_str);
}


COTPRESULT totp_next(OTPData* data, char* out_str)
{
	return otp_generate(data, totp_timecode(data, data->time()) + 1, out_str);
}

COTPRESULT totp_verify(OTPData* data, const char* key, uint64_t for_time, int64_t valid_window)
{
	if (key == NULL || valid_window < 0)
		return OTP_ERROR;
	
	if (valid_window > 0)
	{
		int wins = 0;
		for (int64_t i=-valid_window; i<valid_window+1; i++)
		{
			int cmp = totp_compare(data, key, i, for_time);
			if (cmp == OTP_OK)
				wins++;
		}
		
		return (COTPRESULT) wins >= 1;
	}
	
	return totp_compare(data, key, 0, for_time);
}

uint64_t totp_valid_until(OTPData* data, uint64_t for_time, int64_t valid_window)
{
	return for_time + (data->interval * valid_window);
}

uint64_t totp_timecode(OTPData* data, uint64_t for_time)
{
	if (data->interval <= 0)
		return OTP_ERROR;
	
	return for_time / data->interval;
}

int hotp_compare(OTPData* data, const char* key, uint64_t counter)
{
	if (key == NULL || data == NULL)
		return OTP_ERROR;

	char* cnt_str = (char*)malloc(data->digits + 1);
	if (cnt_str == NULL)
		return OTP_ERROR; 

	memset(cnt_str, 0, data->digits + 1);

	if (hotp_at(data, counter, cnt_str) == 0) {
		free(cnt_str);
		return OTP_ERROR;
	}

	int invalid = 0;
	for (size_t i = 0; i < data->digits; i++) {
		invalid |= key[i] ^ cnt_str[i];
	}

	free(cnt_str);  

	if (invalid != 0)
		return OTP_ERROR;

	return OTP_OK;
}

int hotp_at(OTPData* data, uint64_t counter, char* out_str)
{
	return otp_generate(data, counter, out_str);
}

int hotp_next(OTPData* data, char* out_str)
{
	return otp_generate(data, data->count++, out_str);
}

COTPRESULT otp_generate(OTPData* data, uint64_t input, char* out_str)
{
	if (out_str == NULL || data == NULL || data->base32_secret == NULL)
		return OTP_ERROR;

	char byte_string[9]; 
	memset(byte_string, 0, sizeof(byte_string));

	size_t base32_len = strlen(data->base32_secret);
	if (base32_len < 8)  
		return OTP_ERROR;

	size_t bs_len = (base32_len / 8) * 5;
	char* byte_secret = (char*)malloc(bs_len + 1);
	if (byte_secret == NULL)
		return OTP_ERROR;

	memset(byte_secret, 0, bs_len + 1);

	char hmac[64];
	memset(hmac, 0, sizeof(hmac));

	if (otp_num_to_bytestring(input, byte_string) == 0 || otp_byte_secret(data, byte_secret) == 0) {
		free(byte_secret);
		return OTP_ERROR;
	}

	int hmac_len = (*(data->algo))(byte_secret, bs_len, byte_string, hmac);
	free(byte_secret);  

	if (hmac_len == 0 || hmac_len > 64)
		return OTP_ERROR;

	uint64_t offset = (hmac[hmac_len - 1] & 0xF);

	if (offset + 3 >= (uint64_t)hmac_len)
		return OTP_ERROR;

	uint64_t code =
		(((hmac[offset] & 0x7F) << 24)
			| ((hmac[offset + 1] & 0xFF) << 16)
			| ((hmac[offset + 2] & 0xFF) << 8)
			| ((hmac[offset + 3] & 0xFF)));

	static const uint64_t POWERS[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

	if (data->digits > 10)  
		return OTP_ERROR;

	code %= POWERS[data->digits];

	snprintf(out_str, data->digits + 1, "%0*" PRIu64, data->digits, code); 

	return OTP_OK;
}

