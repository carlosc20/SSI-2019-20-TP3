#include "mail.h"

/* 
 * Usa libcurl para enviar um email com SMTP e TLS
 */
       
#define USERNAME    "psd.fuse.auth@gmail.com"
#define PASSWORD    "WaTnsXNY4qNu7Fv"


static const char *payload_text[5] = {
  "To: utilizador\r\n"
  "From: Gestor de ficheiros\r\n",
  "Subject: Código de verificação\r\n",
  "\r\n", /* empty line to divide headers from body, see RFC5322 */
  NULL, // replaced with code
  NULL
};

struct upload_status {
  int lines_read;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;

  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }

  data = payload_text[upload_ctx->lines_read];

  if(data) {
    size_t len = strlen(data);
    memcpy(ptr, data, len);
    upload_ctx->lines_read++;

    return len;
  }

  return 0;
}

int send_mail(const char* code, const char* to) 
{

    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;

    upload_ctx.lines_read = 0;

    char str[strlen(code) + 2 + 1];
    strcpy(str, code);
    strcat(str, "\r\n");
    payload_text[3] = str;

    curl = curl_easy_init();
    if(curl) {
        // username and password
        curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD);

        // mailserver: port 587 (see RFC4403)
        curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");

        // use TLS
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

        // sender
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, USERNAME);

        // recipient
        recipients = curl_slist_append(recipients, to);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        // payload
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        // debug
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        // Send the message 
        res = curl_easy_perform(curl);

        // Check for errors 
        if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

        // free recipients
        curl_slist_free_all(recipients);

        // always cleanup
        curl_easy_cleanup(curl);
        
  }

  return (int)res;
}