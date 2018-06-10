CC		:= clang
RM		:= rm

CFLAGS	:= -std=c11 -pedantic -Weverything -pthread
LDFlAGS :=
LIBS	:= -pthread

SRCS	:= $(wildcard *.c) generic/generic_list.c
OBJS	:= $(SRCS:.c=.o)

SERVER	:= server.app

.PHONY: all clean

all: $(SERVER)

debug: CLFAGS += -DDEBUG
debug: all

$(SERVER): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

clean:
	$(RM) -f $(OBJS) $(SERVER)
