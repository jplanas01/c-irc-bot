#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <search.h>
#include "main.h"

extern struct bot_info information;

struct messages Actions[] = {
	{"PRIVMSG", act_privmsg},
	{"PING", act_ping},
	{"432", act_432},
	{"001", act_001},
};

char *strdup(const char *str) {
	size_t len;
	char *temp;

	len = strlen(str);
	temp = malloc(len + 1);
	memcpy(temp, str, len);
	temp[len]= '\0';
	return temp;
}

int buffer_send(const char *tosend) {
	int counter, offset, count;
	char buf[1024];
	counter = strlen(tosend);
	offset = 0;
	count = 0;

	while (counter > 0) {
		memcpy(buf, tosend + offset, sizeof(buf) > counter + 1 ? counter + 1 :
				sizeof(buf));
		count = send(information.sockfd, buf, strlen(buf), 0);
		offset += count;
		counter -= count;
	}
	return offset;
}

void privmsg(const char *target, const char *msg) {
	char buf[1024];
	int maxlen;
	maxlen = 940;

	sprintf(buf, "PRIVMSG %s :%.*s\r\n", target, maxlen, msg);
	buffer_send(buf);
	if (strlen(msg) > maxlen) {
		privmsg(target, msg + maxlen);
	}
}

static int str_comp(const void *ptr1, const void *ptr2) {
	return strcmp(*(char **)ptr1, *(char **)ptr2);
}

static int find_chan(const char *chan) {
	char **old;

	old = lfind(&chan, information.chans, &information.nchans, sizeof(char *),
			str_comp);
	if (!old) {
		return -1;
	}

	return (int)(old - information.chans);
}

void part_chan(const char *chan, const char *msg) {
	int chan_num;
	char **old, *tofree;
	char buf[1024];

	chan_num = find_chan(chan);
	
	/* Channel not found */
	if (chan_num == -1) {
		printf("Chan not found! %s\n", chan);
		return;
	}
	
	old = information.chans + chan_num;
	tofree = *old;
	
	/* Shift other channels downward in array */
	if (information.nchans > 1) {
		if (information.nchans - 1 == chan_num) {
			memset(old, 0, sizeof(char *));
		} else {
			memmove(old, old + 1, sizeof(char *) * (information.nchans -
						chan_num));
		}
		free(tofree);
		/*realloc(information.chans, sizeof(char *) * (information.nchans -
		 * 1));*/
	} else {
		free(information.chans[0]);
		memset(information.chans, 0, sizeof(char *));
	}
	information.nchans--;

	if (msg == NULL) {
		sprintf(buf, "PART %s\r\n", chan);
	} else {
		sprintf(buf, "PART %s :%s\r\n", chan, msg);
	}
	buffer_send(buf);
}

void join_chan(const char *chan) {
	char buf[128];

	if (!chan || *chan != '#') {
		return;
	}

	/* If chan found, already in it, don't need to join */
	if (find_chan(chan) >= 0) {
		return;
	}

	information.chans = realloc(information.chans, sizeof(char *) * (information.nchans + 1));
	information.chans[information.nchans] = strdup(chan);
	information.nchans++;

	sprintf(buf, "JOIN %s\r\n", chan);
	buffer_send(buf);
}

void quit(const char *msg) {
	char buf[1024];

	if (msg == NULL) {
		sprintf(buf, "QUIT :bye\r\n");
	} else {
		sprintf(buf, "QUIT :%.*s\r\n", 1016, msg);
	}
	buffer_send(buf);
	exit(0);
}

static void filter(char *a) {
	size_t end;
	if (a == NULL) {
		return;
	}
	end = strlen(a) - 1;
	if (a[end] == '\r' || a[end] == '\n') {
		a[end] = '\0';
	}
}

static void sign_in(const char *nick, const char *user) {
	char buf[128];
	
	if (strlen(nick) > 63 || strlen(user) > 63) {
		fprintf(stderr, "Error, nick or user too long (>63 chars)\n");
		exit(3);
	}

	sprintf(buf, "NICK %s\r\n", nick);
	buffer_send(buf);
	
	sprintf(buf, "USER %s localhost localhost :%s\r\n", user, user);
	buffer_send(buf);
}

/*Connect a socket, return the descriptor*/
int connect_sock() {
	struct addrinfo hints;
	struct addrinfo *servinfo, *p;
	int sockfd, status;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(HOST, PORT, &hints, &servinfo);

	if (status != 0) {
		fprintf(stderr, "Something went wrong: %s\n",
				gai_strerror(status));
		exit(1);
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {
		sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (sockfd == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: could not connect\n");
		exit(2);
	}
	freeaddrinfo(servinfo);
	return sockfd;
}

static void process_buf(char *data) {
	char *str, *sender, *target, *action;
	int i;

	if (data == NULL || *data == '\0') {
		return;
	}

	str = strtok(data, "\n");
	while (str && *str) {
		filter(str);
		printf("str: %s, ", str);
		
		if (*str == ':') {
			sender = str + 1;
			action = strchr(str, ' ');
			*action++ = '\0';
			target = strchr(action, ' ');
			*target++ = '\0';
			data = strchr(target, ' ');
			if (data != NULL) {
				*data++ = '\0';
			}
		} else {
			action = str;
			data = strchr(str, ' ');
			*data++ = '\0';
		}

		printf("Action: %s\n", action);
		for (i = 0; i < sizeof(Actions)/sizeof(Actions[0]); i++) {
			if (strcmp(Actions[i].name, action) == 0) {
				Actions[i].func(sender, target, data);
			}
		}
		str = strtok(NULL, "\n");
	}
}

static void main_loop() {
	int poll_status, received, already_in;
	char buf[1024];
	struct pollfd poll_fds[1];

	memset(buf, 0, sizeof(buf));
	poll_fds[0].fd = information.sockfd;
	poll_fds[0].events = POLLIN;	
	already_in = 0;

	while (1) {
		poll_status = poll(poll_fds, 1, -1);
		if (poll_status == -1) {
			perror("poll");
		} else if (poll_status == 0) {
			;
		} else {
			if (poll_fds[0].revents & POLLIN) {
				do {
					received = recv(poll_fds[0].fd, buf + already_in,
							sizeof(buf) - 1, 0);
					if (buf[received + already_in - 1] != '\n') {
						already_in = (int)(strrchr(buf, '\n') - buf);
						if (already_in >= 0) {
							buf[already_in ] = '\0';
							process_buf(buf);
							memcpy(buf, buf + already_in + 1, received -
									already_in);
							already_in = received - already_in - 1;
						}
					} else {
						buf[received + already_in] = '\0';
						already_in = 0;
						process_buf(buf);
					}
				} while (received == sizeof(buf));
			}
		}
	}
}

int main(int argc, char *argv[])
{
	int sockfd;

	/* Set up info global struct */
	memset(&information, 0, sizeof(information));
	information.chans = malloc(1);
	information.nick = malloc(strlen(NICK) + 1);
	strcpy(information.nick, NICK);
	information.joined = 0;
	information.user = malloc(strlen(USER) + 1);
	strcpy(information.user, USER);

	/* Actually start doing things now, connect socket and loop */
	sockfd = connect_sock();
	information.sockfd = sockfd;

	sign_in(NICK, USER);
	main_loop();
	close(information.sockfd);
	return 0;
}
