#Got this from the official Website of Alexis Delis
OBJS    = myport.o vessel.o monitor.o port-master.o
OUT  	= myport vessel monitor port-master
CC		= gcc -g
# -g option enables debugging mode 
# make the executables

all : $(OUT)

myport : myport.o vessel.h
	$(CC) -o myport myport.c

vessel : vessel.o vessel.h
	$(CC) -o vessel vessel.c

monitor : monitor.o
	$(CC) -o monitor monitor.c

port-master : port-master.o
	$(CC) -o port-master port-master.c

# clean house
clean:
	rm -f $(OUT) $(OBJS)
