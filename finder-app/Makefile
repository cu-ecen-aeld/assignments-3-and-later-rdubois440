#make CROSS_COMPILE=/opt/arm/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-

writer : writer.c
	$(CROSS_COMPILE)gcc -o writer writer.c

clean :
	rm -f writer
	rm -f *.o
