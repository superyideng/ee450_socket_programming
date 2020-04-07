CC = g++
CFLAGS = -g -Wall

all: serverAoutput serverBoutput awsoutput client

serverAoutput: serverA.cpp
	$(CC) $(CFLAGS) -o serverA.o -c serverA.cpp
	$(CC) $(CFLAGS) -o serverAoutput serverA.o

serverBoutput: serverB.cpp
	$(CC) $(CFLAGS) -o serverB.o -c serverB.cpp
	$(CC) $(CFLAGS) -o serverBoutput serverB.o

awsoutput: aws.cpp
	$(CC) $(CFLAGS) -o aws.o -c aws.cpp
	$(CC) $(CFLAGS) -o awsoutput aws.o

client: client.cpp
	$(CC) $(CFLAGS) -o client.o -c client.cpp
	$(CC) $(CFLAGS) -o client client.o

serverA:
	@./serverAoutput
serverB:
	@./serverBoutput
aws:
	@./awsoutput
clean:
	@rm -f *.o *output client
