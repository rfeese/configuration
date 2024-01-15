CC=$(CROSS)gcc
PKG_CONFIG=$(CROSS)pkg-config
CFLAGS=-g -Wall

.PHONY: all clean test test_clean

#binaries
all: example

example: example.c src/configuration.o
	$(CC) -g $(LDFLAGS) example.c src/configuration.o $(LIBS) -o $@

install:
	cd src && make install

#delete compiled binaries
clean:
	make --directory src $@
	- rm example

#buid and run tests
test:
	$(MAKE) --directory test $@

test_clean:
	$(MAKE) --directory test $@

