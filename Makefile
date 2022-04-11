CC = gcc
CFLAGS = -g -c -m32 -lpthread
AR = ar -rc
RANLIB = ranlib

all: my_vm.a

64:
	$(CC) -g -c -lpthread my_vm64.c
	ar -rc libmy_vm.a my_vm64.o
	ranlib libmy_vm.a

my_vm.a: my_vm.o
	$(AR) libmy_vm.a my_vm.o
	$(RANLIB) libmy_vm.a

my_vm.o: my_vm.h

	$(CC)	$(CFLAGS)  my_vm.c

clean:
	rm -rf *.o *.a
