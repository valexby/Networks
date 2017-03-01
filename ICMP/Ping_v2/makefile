CC=gcc
CFLAGS=-c
PINGSRC=ping.c ping_utils.c
PINGOBJ=$(PINGSRC:.c=.o)
PINGNAME=myping

all: ping

ping: $(PINGOBJ)
	$(CC) $(PINGOBJ) -o $(PINGNAME)

$(PINGOBJ): $(PINGSRC)
	$(CC) $(CFLAGS) $(PINGSRC)

clean:
	rm *.o
	rm $(PINGNAME)
