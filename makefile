#Got this from the official Website of Alexis Delis
OBJS    = myport.o vessel.o monitor.o port-master.o
HEADERS = vessel.h
OUT  	= myport vessel monitor port-master
CC		= gcc -g
LIBS    += -lpthread -lrt
# -g option enables debugging mode 
# make the executables

all : $(OUT)

myport : myport.o vessel.h
	$(CC) -o myport myport.c $(LIBS) $(HEADERS)

vessel : vessel.o vessel.h
	$(CC) -o vessel vessel.c $(LIBS) $(HEADERS)

monitor : monitor.o
	$(CC) -o monitor monitor.c $(LIBS) $(HEADERS)

port-master : port-master.o
	$(CC) -o port-master port-master.c $(LIBS) $(HEADERS)

# clean house
clean:
	rm -f $(OUT) $(OBJS)
