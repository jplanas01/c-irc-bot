#include "main.h"
#include <stdio.h>
#include <string.h>

void cmd_pens(char *sender, char *target, char *data) {
	privmsg(target, "This is a test message");
}

void cmd_join(char *sender, char *target, char *data) {
	char *space1, *space2;
	privmsg(sender, "Joining chan...");

	space1 = strchr(data, ' ');
	if (space1 == NULL) {
		return;
	}
	space1++;
	space2 = strchr(space1, ' ');
	if (space2 != NULL) {
		*space2 = '\0';
	}

	printf("%s\n", space1);
	join_chan(space1);
}

void cmd_part(char *sender, char *target, char *data) {
	char *space1, *space2;
	privmsg(sender, "Parting chan...");

	space1 = strchr(data, ' ');
	if (space1 == NULL) {
		return;
	}
	space1++;
	space2 = strchr(space1, ' ');
	if (space2 != NULL) {
		*space2 = '\0';
		part_chan(space1, ++space2);
	} else {
		part_chan(space1, NULL);
	}

}

void cmd_quit(char *sender, char *target, char *data) {
	char *space; 

	space = strchr(data, ' ');
	if (space == NULL) {
		quit(NULL);
	} else {
		quit(++space);
	}
}

