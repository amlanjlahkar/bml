CC=zig
CFLAGS=cc -Wall -g -std=c17
LDFLAGS=-ledit

SRCDIR=src
OBJDIR=obj
BINDIR=bin

SOURCES:=$(wildcard $(SRCDIR)/*.c)
OBJECTS:=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

BIN:=$(BINDIR)/toosty

ifndef vb
.SILENT:
endif

.PHONY: clean build run all
all: run

$(BIN): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(BIN)

build: $(BIN)

run: $(BIN)
	$(BIN)

