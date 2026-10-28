CC ?= gcc
CFLAGS ?= -std=c99 -Wall -Wextra -O2
RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
RAYLIB_LIBS = $(shell pkg-config --libs raylib) -lm

SRC_DIR = src
OBJ_DIR = obj
BIN = highway-fighter

SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/player.c \
       $(SRC_DIR)/road.c \
       $(SRC_DIR)/traffic.c \
       $(SRC_DIR)/items.c \
       $(SRC_DIR)/render.c \
       $(SRC_DIR)/audio.c \
       $(SRC_DIR)/scenery.c

OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $@ $(RAYLIB_LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/game.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -I$(SRC_DIR) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN)

run: $(BIN)
	./$(BIN)

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

install: $(BIN)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(BIN) $(DESTDIR)$(BINDIR)/$(BIN)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(BIN)

.PHONY: all clean run install uninstall
