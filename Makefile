CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g $(shell pkg-config --cflags ncurses)
LIBS    = $(shell pkg-config --libs ncurses)
SRCS    = src/core/main.c
TARGET  = comscope

all: $(TARGET)
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
clean:
	rm -f $(TARGET)