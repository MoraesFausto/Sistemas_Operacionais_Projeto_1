CC = gcc
CFLAGS = -Wall -pthread

all: yoda_monitor

yoda_monitor: yoda_monitor.c
	$(CC) $(CFLAGS) -o yoda_monitor yoda_monitor.c

clean:
	rm -f yoda_semaforo yoda_monitor
