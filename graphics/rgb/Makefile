CFLAGS=-Wall -Wextra -Werror -std=c11 -O2
LDFLAGS=-lm

bluf: bluf.o librgb.a
	$(CC) -o $@ $^ $(LDFLAGS)

librgb.a: color.o photo.o image.o
	$(AR) rc $@ $^

.PHONY:
clean:
	@rm -f *.o

.PHONY:
sweep: clean
	@rm -f bluf librgb.a
