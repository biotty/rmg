CFLAGS=-Wextra -Werror -std=c11

bluf: bluf.o librgb.a
	$(CC) -o $@ $^

librgb.a: color.o photo.o image.o
	$(AR) rc $@ $^
