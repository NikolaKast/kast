CC = gcc
CFLAGS = -I./include -Wall -Wextra
LDFLAGS = -L./lib -lglfw3 -lGL -lm -lX11 -lpthread -ldl
SRC = main.c
EXEC = krestiki-noliki
MISSING_LIB_MSG = "libgl1-mesa-dev is not installed. Please install it with: sudo apt install libgl1-mesa-dev"

all: $(EXEC)

$(EXEC):
	@if ! dpkg -l | grep -q libgl1-mesa-dev; then \
		echo $(MISSING_LIB_MSG); \
		exit 1; \
	fi
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

clean:
	rm -f $(EXEC)

.PHONY: all clean