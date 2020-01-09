#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h> 

int send_mail(const char* code, const char* to);