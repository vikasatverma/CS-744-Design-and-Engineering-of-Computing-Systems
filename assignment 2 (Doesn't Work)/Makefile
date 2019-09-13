# Default to debugging flags. Set to -O3 if you want optimisations
# (either change it here, or type 'make CFLAGS=-O3' in the shell).
CFLAGS?=-g -Wall -Wno-unused-value

all: metro reaction

metro: metro_runner.c metro.c cs744_thread.h
	$(CC) $(CFLAGS) -o metro metro_runner.c -lpthread 

reaction: reaction-runner.c reaction.c cs744_thread.h
	$(CC) $(CFLAGS) -o reaction reaction-runner.c -lpthread 

run: metro reaction
	./metro
	./metro
	./metro
	./reaction 0
	./reaction 0
	./reaction 20
	./reaction 20
	./reaction 40
	./reaction 40
	./reaction 60
	./reaction 60
	./reaction 80
	./reaction 80
	./reaction 100
	./reaction 100

clean:
	$(RM) metro reaction
	$(RM) -r *.dSYM
