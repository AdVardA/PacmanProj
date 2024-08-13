build:
	g++ -g main.cpp -o main -lpthread -lncurses
	g++ -g main.cpp -o client -lpthread -lncurses
	#gdb main
	./main -p 8080 -c 1

server: clean
	g++ server.cpp -o server
	g++ client.cpp -o client
	./server -p 8080 -a 1
	
s:
	@./server -p 8080 -a 3

test:clean
	g++ test.cpp -o test
	./test

game:clean
	g++ -g game.cpp -o game -lpthread -lncurses
	#gdb game
	./game

gen:
	g++ map_gen.cpp -o gen
	./gen

clean:
	rm -rf server *.o
	rm -rf test *.o
