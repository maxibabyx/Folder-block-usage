UsoDisco: dir_queue.o
	gcc UsoDisco.c dir_queue.o -pthread -o UsoDisco

dir_queue.o: dir_queue.c dir_queue.h
	gcc -c dir_queue.c

clean:
	rm UsoDisco dir_queue.o