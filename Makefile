appserver: Bank.o appserver.o
	cc -pthread -o appserver Bank.o appserver.o

Bank: Bank.c
	gcc -c Bank.c

server: appserver.c
	gcc -c appserver.c

coarse-server: appserver-coarse.c
	gcc -c appserver-coarse.c

appserver-coarse: Bank.o appserver-coarse.o
	cc -pthread -o appserver-coarse Bank.o appserver-coarse.o

clean:
	rm -f *.o
