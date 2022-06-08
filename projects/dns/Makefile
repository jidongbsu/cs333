CC = gcc
CFLAGS = -Wall -Wextra -Wpointer-arith -Wstrict-prototypes -std=gnu89 -fPIC -MMD -MP

all: dnsserver

#This builds an executable 
dns-server: dnsserver.o
	$(CC) $(CFLAGS) -o $@ $?

.PHONY: clean
clean:
	/bin/rm -f *.o *.d dnsserver
