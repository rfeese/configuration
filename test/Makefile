SHELL=/bin/sh
CC=$(CROSS)gcc
PKG_CONFIG=$(CROSS)pkg-config
CFLAGS=-g -Wall
UNITY=../../Unity/src/unity.c

.PHONY: all test clean

# default - run tests
all test:  test_configuration
	-./test_configuration

test_internal: test_configuration_internal
	-./test_configuration_internal

# build tests
test_configuration: $(UNITY) test_configuration.c ../src/configuration.h ../src/configuration.c
	$(CC) $(CFLAGS) $(UNITY) -fno-builtin-printf test_configuration.c ../src/configuration.c -o test_configuration

test_configuration_internal: $(UNITY) test_configuration_internal.c ../src/configuration.h ../src/configuration.c
	$(CC) $(CFLAGS) $(UNITY) -fno-builtin-printf test_configuration_internal.c -o test_configuration_internal

# delete compiled binaries
clean test_clean:
	- rm test_configuration
	- rm test_configuration_internal
