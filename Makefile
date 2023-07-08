CC=zig
CFLAGS=cc -Wall -g -std=c17
LDFLAGS=-ledit -lm

SRCDIR=src
BINDIR=bin

SOURCES:=$(wildcard $(SRCDIR)/*.c)
OBJECTS:=$(patsubst $(SRCDIR)/%.c, $(BINDIR)/%, $(SOURCES))


ifndef verbose
.SILENT:
endif


all: $(OBJECTS)

$(BINDIR)/%: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

.PHONY: clean run
clean:
	rm -f $(OBJECTS)

run: $(OBJECTS)
	./$(BINDIR)/parsing

