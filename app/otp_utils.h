#pragma once

#include <cstdint>

int hmac_algo_sha1(const char* byte_secret, int key_length, const char* byte_string, char* out);
uint64_t getCurrentTime();
