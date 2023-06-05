CC=gcc
CFLAGS=-std=gnu99 -Wall -pedantic-errors
VFLAGS=-g -Og -std=gnu99

libuv: libuv_timer.c libuv_poll_timer.c libuv_chat.c
	$(CC) $(CFLAGS) libuv_timer.c -o libuv_timer -luv
	$(CC) $(VFLAGS) libuv_timer.c -o libuv_timer_valgrind -luv
	$(CC) $(CFLAGS) libuv_poll_timer.c -o libuv_poll_timer -luv
	$(CC) $(VFLAGS) libuv_poll_timer.c -o libuv_poll_timer_valgrind -luv
	$(CC) $(CFLAGS) libuv_chat.c -o libuv_chat -luv
	$(CC) $(VFLAGS) libuv_chat.c -o libuv_chat_valgrind -luv


clean:
	rm -f libuv_timer
	rm -f libuv_timer_valgrind
	rm -f libuv_poll_timer
	rm -f libuv_poll_timer_valgrind
	rm -f libuv_chat
	rm -f libuv_chat_valgrind

valgrind_timer:
	valgrind --leak-check=full --track-origins=yes ./libuv_timer_valgrind

valgrind_chat:
	valgrind --leak-check=full --track-origins=yes ./libuv_chat_valgrind

valgrind_poll:
	valgrind --leak-check=full --track-origins=yes ./libuv_poll_timer_valgrind ./flow.txt ./libuv_chat.c ./libuv_timer.c ./libuv_poll_timer.c

touch:
	touch ./flow.txt
	touch ./libuv_chat.c
	touch ./libuv_timer.c
	touch ./libuv_poll_timer.c	


