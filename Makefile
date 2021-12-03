CC = gcc
CFLAGS = -std=c89 -Wall -pedantic -g

TARGET = irc
SOURCES = main.c act_funcs.c commands.c
OBJECTS = $(SOURCES:.c=.o)
DEPS = main.h act_funcs.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(OBJECTS) irc.exe
