# Gavin Lee - ECE 3220
# Project 4 Makefile

CC = gcc
CFLAGS = -Wall -g

all: notjustcats

notjustcats: notjustcats.c
	$(CC) $(CFLAGS) notjustcats.c -o notjustcats

clean:
	rm -f notjustcats

submit:
	tar cvzf project4.tgz README Makefile notjustcats.c
	git add notjustcats.c Makefile README
	git commit -m "Auto Project Update"
	git push