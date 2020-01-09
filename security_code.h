#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>

typedef struct User_struct *User;

int read_users(User* users);

char* get_email(const User* users, const char* name);

char* get_rand_code(int length);