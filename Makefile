CFLAGS= -Wall -Wextra -g -pthread
TARGETS = serveur client1 client2

all: $(TARGETS)

clean:
	- rm -f $(TARGETS)

.PHONY: all clean