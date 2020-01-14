#include "security_code.h"
#include "mail.h"


#include <sys/types.h> /* pid_t */
#include <sys/wait.h> 
#include <sys/random.h>
#include <stdlib.h>

#include <pthread.h>

#include <gtk/gtk.h>

#define CODE_LENGTH 5 // code length
#define SEC_WAIT 30 // seconds before code expires

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
    unsigned int buf[5]; 
    getrandom(&buf, sizeof(buf), GRND_RANDOM);
    const char charset[] = "0123456789QWERTYUIOPASDFGHJKLZXCVBNM";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            printf("%d\n", buf[n]);
            int key = buf[n] % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

char* get_rand_code(int length) {
    return rand_string(malloc(sizeof(char[length + 1])), length + 1);
}


int code_inserted;
char *code;
GtkApplication *app;

static void enter_callback(GtkWidget *widget, GtkWidget *window)
{
    const gchar *entry_text = gtk_entry_get_text (GTK_ENTRY (widget));
    printf ("Entry: %s\nCode: %s\n", entry_text, code);

    if(strcmp(code, entry_text) == 0) {
        code_inserted = 1;
        printf ("Bom codigo\n");
        gtk_window_close(GTK_WINDOW (window));
    } else {
        printf ("Mau codigo\n");
    }

}

static void activate (GtkApplication* app, gpointer user_data)
{
    GtkWidget *window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "Insert Security Code");
    gtk_window_set_default_size (GTK_WINDOW (window), 250, 20);

    GtkWidget *entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (entry), CODE_LENGTH);
    g_signal_connect (entry, "activate", G_CALLBACK (enter_callback), window);
    gtk_entry_set_text (GTK_ENTRY (entry), "code");

    gtk_container_add (GTK_CONTAINER (window), entry);

    gtk_widget_show_all (window);
}

void timer(int c) {
    g_application_quit(G_APPLICATION(app));
}

int main(void)
{
    // lê ficheiro users
    User users[64];
    if(read_users(users) != 0)
        return -1;
     
    // vê o email do utilizador atual
    char* email = get_email(users,get_current_username());

    // gera código
    code = get_rand_code(CODE_LENGTH);
    code_inserted = 0;

    // envia email
    //int email_status = send_mail(code, email);
    //if(email_status != 0)
    //    return email_status; 

    signal(SIGALRM, timer);

    // abre janela input
    app = gtk_application_new ("org.security_code", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    alarm(SEC_WAIT);
    g_application_run (G_APPLICATION (app), 0, NULL);
    alarm(0);
    g_object_unref(app);


    if(code_inserted == 0) {
        printf("Failed\n");
    } else {
        printf("Success\n");
    }

	return 0;
}