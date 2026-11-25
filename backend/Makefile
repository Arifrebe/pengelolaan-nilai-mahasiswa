CC     = gcc
CFLAGS = -Wall -Wextra -O2 -Ilib -Isrc
SRC    = src/server.c src/mahasiswa.c src/nilai.c src/graph.c src/persist.c lib/mongoose.c

server: $(SRC) src/akademik.h
	$(CC) $(CFLAGS) -o server $(SRC)

run: server
	./server ../frontend

clean:
	rm -f server
