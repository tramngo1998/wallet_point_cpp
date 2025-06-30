#include "otp_utils.h"
#include <openssl/hmac.h>
#include <chrono>
#include <cstring>

extern "C"
{
#include <openssl/evp.h>
}

using namespace std;

static const int32_t SHA1_BYTES = 160 / 8;

int hmac_algo_sha1(const char* key, int key_length, const char* input, char* output) {
	unsigned int len = SHA1_BYTES;

	unsigned char* result = HMAC(
		EVP_sha1(),							
		(unsigned char*)key, key_length,	
		(unsigned char*)input, 8,		
		(unsigned char*)output,				
		&len
	);

	// Return the HMAC success
	return result == 0 ? 0 : len;
}

uint64_t getCurrentTime() {
	using namespace chrono;

	auto now = system_clock::now();
	auto dur = now.time_since_epoch();

	return duration_cast<chrono::seconds>(dur).count();
}