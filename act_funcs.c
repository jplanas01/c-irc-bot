#include "main.h"
#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern struct bot_info information;
int authed = 0;

struct command {
	char *name;
	void (*func)(char *sender, char *target, char *data);
};

struct command Commands[] = {
	{"pens", cmd_pens},
	{"join", cmd_join},
	{"part", cmd_part},
	{"quit", cmd_quit},
};

void act_ping(char *sender, char *target, char *data) {
	char buf[256];
	sprintf(buf, "PONG %.*s\r\n", 245, data);
	buffer_send(buf);
}

void act_privmsg(char *sender, char *target, char *str) {
	int i;
	char buf[256];
	char *tmp;

	str++;
	printf("%s: <%s> %s\n", target, sender, str);

	tmp = strchr(str, ' ');
	if (tmp != NULL) {
		memcpy(buf, str, (size_t)(tmp - str));
		buf[tmp - str] = '\0';
	} else {
		memcpy(buf, str, strlen(str) + 1);
	}

	if (buf[0] == CMDCHAR) {
		for (i = 0; i < sizeof(Commands)/sizeof(Commands[0]); i++) {
			if (strcmp(Commands[i].name, buf + 1) == 0) {
				printf("command: %s\n", buf + 1);
				Commands[i].func(sender, target, str);
			}
		}

	}
}

/* Executed on first join to server */
void act_001(char *sender, char *target, char *str) {
	join_chan(CHAN);
	authed = 1;
}

/* Nickname too long */
void act_432(char *sender, char *target, char *str) {
	char buf[128], *newnick;

	printf("Nick too long, chomping...\n\n");
	sprintf(buf, "NICK %.9s\r\n", information.nick);
	buffer_send(buf);

	free(information.nick);
	newnick = malloc(10);
	sprintf(newnick, "%.9s", NICK);
	information.nick = newnick;
}
