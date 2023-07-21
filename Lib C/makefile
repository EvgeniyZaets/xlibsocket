#Uncomment the next line for SysV
#LIBS = -lsocket -lnsl -letcp -lsysv
LIBS = -letcp
MAKE = make
CC = gcc

EXECUTABLES =   tselect
		\ Server 	#сюда необходимо указать название приложения к которому необхоимо подключить библиотеку

.c.o :
	$(CC) -c -g -O -Wall $< -o $@

all	  : $(EXECUTABLES)

lib/libetcp.a :
	( cd lib; $(MAKE) )


Server: Server.o lib/libetcp.a tselect.o
	$(CC) -o Server -L lib Server.o tselect.o $(LIBS)

tselect.o : etcp.h
Server: etcp.h heartbeat.h

clean :
	-rm *.o
	-rm *.core
	-rm *.scr
	-rm $(EXECUTABLES)

cleanlib :
	( cd lib; $(MAKE) clean )

cleanall : clean cleanlib
