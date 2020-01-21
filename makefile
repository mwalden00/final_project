all: multi_thread_server.o chat_client.o networking.o chat_funcs.o
	gcc -o server multi_thread_server.o networking.o chat_funcs.o
	gcc -o client chat_client.o networking.o chat_funcs.o

multi_thread_server.o: multi_thread_server.c networking.h chat_funcs.h
	gcc -c multi_thread_server.c

chat_client.o: chat_client.c networking.h chat_funcs.h
	gcc -c chat_client.c

chat_funcs.o: chat_funcs.c chat_funcs.h networking.h
	gcc -c chat_funcs.c

networking.o: networking.c networking.h
	gcc -c networking.c

clean:
	rm *.o
	rm server client
