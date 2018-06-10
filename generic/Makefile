CC		:= clang
RM		:= rm

CFLAGS	:= -std=c89 -pedantic -Weverything -pthread
LDFlAGS :=
LIBS	:= -pthread

SRCS	:= $(wildcard *.c)
OBJS	:= $(SRCS:.c=.o)

BIN		:= generics

.PHONY: all clean

all: $(BIN)

debug: CLFAGS += -DDEBUG
debug: all

$(BIN): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

clean:
	$(RM) -f $(OBJS) $(BIN)
