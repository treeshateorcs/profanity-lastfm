CC = clang
CFLAGS = -shared -fpic -g3 -O0 -Wextra -pedantic -pthread

all:
	$(CC) $(CFLAGS) -lcurl -lprofanity -o profanity-lastfm.so profanity-lastfm.c
