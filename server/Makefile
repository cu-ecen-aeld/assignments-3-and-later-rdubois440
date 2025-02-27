#make CROSS_COMPILE=/opt/arm/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-

all : aesdsocket

default : aesdsocket


aesdsocket : aesdsocket.c
	$(CC) -o aesdsocket aesdsocket.c -Wall -Werror -Wno-use-after-fre

clean :
	rm -f aesdsocket
	rm -f *.o
