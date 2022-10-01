.POSIX:

BINS = \
	cat \
	echo \
	ed \
	false \
	grep \
	head \
	ls \
	mkdir \
	nice \
	pwd \
	rm \
	rmdir \
	sleep \
	sort \
	tee \
	true \
	tty \
	uname \
	wc \

CC = cc
XCFLAGS = $(CFLAGS) -Wall -Wextra -std=c99 -pedantic -g -Og
XLDFLAGS = $(LDFLAGS)

all: $(BINS)

.c:
	$(CC) $(XCFLAGS) $(XLDFLAGS) -o $@ $<

clean:
	rm -f $(BINS)
