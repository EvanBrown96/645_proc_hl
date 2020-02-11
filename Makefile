#

z80: main.o z80.o register.o
	g++ -o z80 main.o z80.o register.o -g

main.o: main.cpp z80.h
	g++ -c main.cpp -g

z80.o: z80.cpp z80.h register.h
	g++ -c z80.cpp -g

register.o: register.cpp register.h
	g++ -c register.cpp -g

clean:
	rm *.o z80 z80.exe
