# Gavin Lee - ECE 3220
# Project 4 Makefile

CC = gcc
CFLAGS = -Wall -g
TESTDIR = testFiles

all: notjustcats

notjustcats: notjustcats.c
	$(CC) $(CFLAGS) notjustcats.c -o notjustcats

clean:
	rm -f notjustcats

test: notjustcats
	rm -rf $(TESTDIR)/*
	mkdir -p $(TESTDIR)
	mkdir -p $(TESTDIR)/blankOut
	mkdir -p $(TESTDIR)/simpleOut
	mkdir -p $(TESTDIR)/simple2Out
	mkdir -p $(TESTDIR)/evenlesssimpleOut
	./notjustcats blankfloppy.img $(TESTDIR)/blankOut
	./notjustcats simple.img $(TESTDIR)/simpleOut
	./notjustcats simple2.img $(TESTDIR)/simple2Out
	./notjustcats evenlesssimple.img $(TESTDIR)/evenlesssimpleOut

submit:
	tar cvzf project4.tgz README Makefile notjustcats.c
	git add notjustcats.c Makefile README
	git commit -m "Auto Project Update"
	git push