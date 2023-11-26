#binaries
all: example

example: example.c src/configuration.o
	$(CC) -g $(LDFLAGS) example.c src/configuration.o $(LIBS) -o $@

#delete compiled binaries
clean:
	- rm src/*.o
	- rm example

#buid and run tests
test:
	$(MAKE) --directory test $@

test_clean:
	$(MAKE) --directory test $@

.PHONY: all clean test test_clean
