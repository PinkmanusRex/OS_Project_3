CC = gcc
CFLAGS = -g -c -m32 -lpthread
AR = ar -rc
RANLIB = ranlib

all: my_vm.a

64: 
	$(CC) -g -c -lpthread -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast my_vm64.c
	$(AR) libmy_vm.a my_vm64.o
	$(RANLIB) libmy_vm.a

my_vm.a: my_vm.o
	$(AR) libmy_vm.a my_vm.o
	$(RANLIB) libmy_vm.a

my_vm.o: my_vm.h

	$(CC)	$(CFLAGS)  my_vm.c

clean:
	rm -rf *.o *.a
