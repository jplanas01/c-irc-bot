#ifndef MAIN_H
#define MAIN_H

#define PORT "6667"
#define HOST "127.0.0.1"
#define NICK "QWERTER"
#define USER "asdkjfh"
#define CHAN "#sick"
#define CMDCHAR '!'
#include "act_funcs.h"

struct messages {
	char *name;
	void (*func)(char *sender, char *target, char *str);
};

struct bot_info {
	char *nick;
	char *user;
	char **chans;
	unsigned int nchans;
	int joined;
	int sockfd;
};

struct bot_info information;

int buffer_send(const char *data);
void privmsg(const char *target, const char *msg);
void join_chan(const char *chan);
void part_chan(const char *chan, const char *msg);
void quit(const char *msg);
#endif
