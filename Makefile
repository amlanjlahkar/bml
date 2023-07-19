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

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean run
clean:
	rm -f $(OBJECTS) $(BIN)

run: $(OBJECTS)
	$(BIN)

