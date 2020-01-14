#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

int send_mail(const char* code, const char* to, int debug);