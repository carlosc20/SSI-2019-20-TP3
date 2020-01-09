#include "security_code.h"
#include "mail.h"


#include <sys/types.h> /* pid_t */
#include <sys/wait.h> 

typedef struct User_struct {
    char* name;
    char* email;
} *User;


static User init_user(char* name, char* email)
{
    User new_user = malloc(sizeof(struct User_struct));
    new_user->name = strdup(name);
    new_user->email = strdup(email);
    return new_user;
}

static void free_user(User user) 
{
    free(user->name);
    free(user->email);
    free(user);
}

// preenche o array com users, NULL terminated
int read_users(User* users)
{
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    FILE* fp = fopen("users", "r");
    if (fp == NULL)
        return -1;

    int i = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        char username[32];
        char email[32];
        sscanf(line, "%[^:]:%s", username, email);
        users[i] = init_user(username, email);
        i++;
    }
    users[i] = NULL;

    if (line)
        free(line);

    fclose(fp);
}

char* get_email(const User* users, const char* name)
{
    for(int i = 0; users[i]; i++) {
        if(!strcmp(users[i]->name, name))
            return users[i]->email;
    }
}


char* get_current_username()
{
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        return pw->pw_name;
    }
    return NULL;
}


static char* rand_string(char* str, size_t size)
{
    srand(time(0)); // melhor random?
    const char charset[] = "0123456789QWERTYUIOPASDFGHJKLZXCVBNM";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

char* get_rand_code(int length) {
    return rand_string(malloc(sizeof(char[length + 1])), length + 1);
}

int main(void)
{

    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        printf("%s : %s\n", pw->pw_name, pw->pw_shell);
    }

	// gnome-terminal
	id_t pid = fork();
    if (pid==0)
    { /* child process */
            execl ("/bin/bash", "bash", NULL);
            exit(127); /* only if execv fails */
    }
    else
    { /* parent process */
           waitpid(pid,0,0); /* wait for child to exit */
    }
	return 0;


    // lê ficheiro users
    User users[64];
    if(read_users(users) != 0)
        return -1;
     
    // vê o email do utilizador atual
    char* email = get_email(users,get_current_username());
    printf("%s\n", email);

    // gera código
    char* code = get_rand_code(5);
    
    return send_mail(code, email);
}