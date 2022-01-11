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
	pwd \
	sort \
	tee \
	true \
	tty \
	wc \

CC = cc
XCFLAGS = $(CFLAGS) -Wall -Wextra -Wpedantic -g -D_XOPEN_SOURCE=700
XLDFLAGS = $(LDFLAGS)

.PHONY: clean

all: bin $(BINS:%=bin/%)

bin:
	mkdir -p bin

$(BINS:%=bin/%): $(BINS:=.c)
	$(CC) $(XCFLAGS) $(XLDFLAGS) -o $@ $(@:bin/%=%.c)

clean:
	rm -fr bin
