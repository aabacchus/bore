.POSIX:

BINS = \
	cat \
	echo \
	false \
	tee \
	true \

XCFLAGS = $(CFLAGS) -Wall -Wextra -Wpedantic -g

.PHONY: clean

all: bin $(BINS:%=bin/%)

bin:
	mkdir -p bin

$(BINS:%=bin/%): $(BINS:=.c)
	$(CC) $(XCFLAGS) -o $@ $(@:bin/%=%.c)

clean:
	rm -fr bin
