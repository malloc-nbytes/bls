DESTDIR ?= /usr/local/bin

main: ../*.c ../include/*.h
	cc -I../include -O2 -o bls ../*.c
	mkdir out
	mv ./bls ./out

install: out/bls
	echo "installing to: $(DESTDIR)"
	sudo cp out/bls $(DESTDIR)

help:
	@echo "make <help|clean|install [DESTDIR=<path> (default to /usr/local/bin)]>"

clean:
	rm -r ./out
