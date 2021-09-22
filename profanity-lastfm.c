#include <curl/curl.h>
#include <profapi.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define USERNAME "basedtho"
#define APIKEY "insert-your-apikey"
#define TIMEOUT 5 // seconds

char *get_token(char *response, char *token) {
  char copy[4096];
  strcpy(copy, response);
  char *result;
  result = strstr(copy, token);
  while (*result != '>') {
    result++;
  }
  result++;
  result = strtok(result, "<");
  return result;
}

size_t write_cb(void *ptr, size_t size, size_t nmemb, char *res) {
  memcpy(res, ptr, size * nmemb);
  return size * nmemb;
}

void *cb(void *unused) {
  (void)unused; // cast to void to supress an unused variable warning
  char res[1024];
  char last_res[1024];
  CURLcode ret;
  char *url =
      "https://ws.audioscrobbler.com/2.0/"
      "?method=user.getrecenttracks&user=" USERNAME "&limit=1&api_key=" APIKEY;
  char response[4096];
  CURL *hnd;
  hnd = curl_easy_init();
  curl_easy_setopt(hnd, CURLOPT_URL, url);
  curl_easy_setopt(hnd, CURLOPT_WRITEDATA, response);
  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_cb);
  while (1) {
    ret = curl_easy_perform(hnd);
    char *nowplaying;
    if ((nowplaying = strstr(response, "nowplaying"))) {
      sprintf(res, "<presence><status>🎶%s - ", get_token(response, "artist"));
      strcat(res, get_token(response, "name"));
      strcat(res, "</status></presence>");
      if (strcmp(last_res, res) != 0) {
        prof_send_stanza(res);
      }
    }
    strcpy(last_res, res);
    struct timespec ts;
    ts.tv_sec = TIMEOUT;
    ts.tv_nsec = 0;
    nanosleep(&ts, NULL);
  }
}

void prof_on_connect(const char *const account_name,
                     const char *const fulljid) {
  (void)account_name;
  (void)fulljid;
  pthread_t thread;
  if (pthread_create(&thread, NULL, cb, NULL)) {
    fprintf(stderr, "Error creating thread\n");
  }
}
