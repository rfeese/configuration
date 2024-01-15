.PHONY: install clean

libconfiguration.a: configuration.o
	ar cr $@ configuration.o

install: libconfiguration.a configuration.h
	cp libconfiguration.a $(DESTDIR)/lib/
	cp configuration.h $(DESTDIR)/include/

clean:
	- rm *.a
	- rm *.o
