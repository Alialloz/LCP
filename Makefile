all: lcp

lcp: lcp.c checksum.o
	$(CC) -Wextra -Wall -o lcp lcp.c checksum.o

checksum.o:
	$(CC) -c checksum.c

inject.so: inject.c
	$(CC) -shared -fPIC -o inject.so inject.c -ldl

check: all inject.so
	bats tests/check.bats

check_priv: all inject.so
	bats tests/priv.bats

check_tp0: all inject.so
	bats tests/tp0_*.bats

check_all: all inject.so
	bats -r .

clean:
	rm lcp checksum.o inject.so
