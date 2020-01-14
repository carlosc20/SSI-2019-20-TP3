#include "security_code.h"


typedef struct User_struct 
{
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


static void free_users(User users[])
{
    for(int i = 0; users[i]; i++) {
        free_user(users[i]);
    }
}

// preenche o array com users, NULL terminated
// abre o ficheiro users
static int read_users(User users[], size_t size ,char* path)
{
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    FILE* fp = fopen(path, "r");
    if (fp == NULL)
        return -1;

    int i = 0;
    while ((read = getline(&line, &len, fp)) != -1 && i < size - 1) {
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

    return i;
}

static char* get_email(const User* users, const char* name)
{
    for(int i = 0; users[i]; i++) {
        if(!strcmp(users[i]->name, name))
            return users[i]->email;
    }
    return NULL;
}

static char* get_current_username()
{
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        return pw->pw_name;
    }
    return NULL;
}

// devolve a str passada
static char* rand_string(char* str, size_t size)
{
    const char charset[] = "0123456789QWERTYUIOPASDFGHJKLZXCVBNM";
    unsigned int buf[5]; 
    getrandom(&buf, sizeof(buf), GRND_RANDOM);
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = buf[n] % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

static int MAX_TRIES;

static int code_inserted;
static char *code;
static int tries;

GtkWidget *g_window;

static void enter_callback(GtkWidget *widget, GtkWidget *window)
{
    const gchar *entry_text = gtk_entry_get_text (GTK_ENTRY (widget));

    if(strcmp(code, entry_text) == 0) {
        code_inserted = 1;
        gtk_window_close(GTK_WINDOW (g_window));
    } else {
        printf("%p", g_window);
        tries++;
        if(tries >= MAX_TRIES)
            gtk_window_close(GTK_WINDOW (g_window));
    }
}

static void activate (GtkApplication* app, int* code_length)
{
    g_window = gtk_application_window_new(app);
    gtk_window_set_title (GTK_WINDOW (g_window), "Insert Security Code");
    gtk_window_set_default_size (GTK_WINDOW (g_window), 250, 20);

    GtkWidget *entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (entry), *code_length);
    g_signal_connect (entry, "activate", G_CALLBACK (enter_callback), g_window);
    gtk_entry_set_text (GTK_ENTRY (entry), "code");

    gtk_container_add (GTK_CONTAINER (g_window), entry);

    gtk_widget_show_all (g_window);
}


void timer(int c) 
{
    gtk_window_close(GTK_WINDOW (g_window));
}

int check_code(int code_length, int sec_wait, int max_tries)
{
    MAX_TRIES = max_tries;

    // lê ficheiro users
    User users[64];
    if(read_users(users, sizeof(users), "/home/carlos/users") <= 0)
        return -1;
     
    // vê o email do utilizador atual
    char* email = get_email(users, get_current_username());


    // gera código
    code = rand_string(malloc(sizeof(char[code_length + 1])), code_length + 1);
    code_inserted = 0;
    tries = 0;

    // envia email
    int email_status = send_mail(code, email, 1);
    if(email_status != 0)
        return email_status; 
    free_users(users);
    signal(SIGALRM, timer);

    // abre janela input
    g_app = gtk_application_new ("org.security_code", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (g_app, "activate", G_CALLBACK (activate), &code_length);
    alarm(sec_wait);
    g_application_run (G_APPLICATION (g_app), 0, NULL);
    alarm(0);
    g_object_unref(g_app);

	return code_inserted;
}

