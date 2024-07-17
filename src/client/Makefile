CC = g++
FLAGS = -lssl -lcrypto

all: main.o encode.o table.o
	$(CC) main.o encode.o table.o $(FLAGS) -o db

main.o: main.cpp
	$(CC) -c main.cpp $(FLAGS)

encode.o: encode.cpp
	$(CC) -c encode.cpp $(FLAGS)

table.o: table.cpp
	$(CC) -c table.cpp $(FLAGS)

clean:
	  rm *.o db
