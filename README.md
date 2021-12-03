Implementation of an IRC bot with a few sample commands in C.

Basic configuration is in main.h (nick, host, user, etc.) and the list of server commands to respond to is at the top of main.c (fixing this is a todo).

Additionally, the bot can respond to PRIVMSG commands prefixed with a command character (by default, '!'). The table for these commands is at the top of act_funcs.c (fixing this is also a todo).

This bot should NOT be used in the wild because of the use of unsafe C functions such as strcmp and sprintf. I think I check often enough whether the target buffer has enough space for the string and buffers are big enough for most IRC interactions, but I wouldn't trust that the code is hardened against an attacker.
