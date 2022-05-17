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
	rmdir \
	sort \
	tee \
	true \
	tty \
	uname \
	wc \

CC = cc
XCFLAGS = $(CFLAGS) -Wall -Wextra -Wpedantic -g
XLDFLAGS = $(LDFLAGS)

.PHONY: clean

all: $(BINS)

.c:
	$(CC) $(XCFLAGS) $(XLDFLAGS) -o $@ $<

clean:
	rm -f $(BINS)
