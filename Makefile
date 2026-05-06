CC      = x86_64-w64-mingw32-gcc
CFLAGS  = -m64 -Wall -Wextra
LDFLAGS = -lkernel32

all: loader.exe message.exe

loader.exe: loader.c obf.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

message.exe: message.c obf.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

enc: enc.c
	gcc -o $@ $^

