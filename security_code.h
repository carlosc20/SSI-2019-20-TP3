#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/random.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "mail.h"

int check_code(int code_length, int sec_wait, int max_tries);