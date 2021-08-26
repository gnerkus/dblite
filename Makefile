.PHONY: test

clean: dblite.o
	rm -rf .bin
	mkdir .bin
	mv dblite .bin/

dblite.o:
	clang db.c -o dblite

test:
	tclsh tests/test.tcl