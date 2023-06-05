# libuv_examples

## libuv_timer
Simple timer that counts down given number of seconds

- how to use: number of seconds is defined with #define TIMER_SET macro
- how i tested: "make valgrind_timer" in the directory

## libuv_chat
A chat server that accepts strings from one client and sends them to other clients.
It remains active for a specified amount of time and then shuts off. The number of clients is
restricted, and the server does not allow more clients to connect than the specified amount.
The server keeps track of client disconnects.

- how to use: 
1. timer is set with #define SET_TIMER
2. server port is specified with #define DEFAULT_PORT
3. no. of clients is determined with #define MAX_CLIENTS
4. server is active on localhost adress

-how i tested:
1. "make valgrind_chat" in the directory
2. I connected with nc (netcat) over multiple terminals using the command "netcat localhost 7000"
  
## libuv_poll_timer
A program that tracks changes in files specified by the user as command line arguments (argv[]).
The program remains active for a given amount of time specified by the user. When the timer expires,
the user has the option to restart the process. While active, it prints the file name of any changed file.
If the program receives more files to track than defined, it will ignore the files that are out of scope.

- how to use: 
1. max number of files is defined with #define MAX_WATCH
2. max time that user can input is defined with #define MAX_TIME
3. poll rate is defined with #define POLL_EVERY_MS

-how i tested: "make valgrind_poll" in the directory, and in the other terminal use "make touch" in the directory
  
