.POSIX:

BINS = \
	cat \
	echo \
	ed \
	false \
	ls \
	tee \
	true \
	wc \

CC = cc
XCFLAGS = $(CFLAGS) -Wall -Wextra -Wpedantic -g
XLDFLAGS = $(LDFLAGS)

.PHONY: clean

all: bin $(BINS:%=bin/%)

bin:
	mkdir -p bin

$(BINS:%=bin/%): $(BINS:=.c)
	$(CC) $(XCFLAGS) $(XLDFLAGS) -o $@ $(@:bin/%=%.c)

clean:
	rm -fr bin
