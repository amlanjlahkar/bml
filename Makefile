CC=zig
CFLAGS=cc -Wall -Wextra -g -std=c17
LDFLAGS=-ledit -lm

SRCDIR=src
BINDIR=bin

SOURCES:=$(wildcard $(SRCDIR)/*.c)
OBJECTS:=$(patsubst $(SRCDIR)/%.c, $(BINDIR)/%, $(SOURCES))

# Default rule
all: $(OBJECTS)

$(BINDIR)/%: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS)

