# Multi-Threaded Chat Server
by Maximilian Walden and Winston Peng
Pd. 5

## TO USE:

First, within the directory of our project, type <code>make</code> to build the server and client executables.

### SERVER:
1. On your desired server system, type <code>./server</code> to execute the server.
   The server simply allows clients to connect and communicate. Typing <code>/exit</code> while the server runs
   will shutdown the server.

### CLIENT:
1. On your desired client system, type <code>./client [server ip] [username]</code>, where <code>[server ip]</code>
   is the target server's ip address and <code>[username]</code> is your server-wide username. Typing <code>/exit</code>
   will shutdown the client process and will log the client out of the server.
2. Clients can send messages to the server by typing them in and hitting enter. Once a message is sent to the server,
   the server will send the message to all other clients.

## DESCRIPTION:

  Because the chat was going to be the main bulk of our mafia program, we decided to ditch mafia and focus entirely on a chat s
  server. The main challenge of this project is being able to simultaneously service all clients while having full access to each
  client's communicating socket, as well as being able to have all clients receive and send messages at the same time.

  I found that forking proved to be too impractical for what I had in mind. The server uses multi-threading, a method for a
  process to do multiple things at once. While similar to forking, threading does not create a new memory space or process ID for
  each new thread, allowing threads to share memory and have the same process ID. To have synchronize thread, mutexes are used.
  Mutexes allow a thread to block all other threads from accessing a piece of memory, allowing threads to "take turns" editing a
  piece of memory. This was particularly useful for this project, as I also had to implement a queue structure for queuing up
  messages. First, a connecting thread and message handling thread is created. Upon receiving a new connection, the connection
  thread accepts the connection of the client and creates a new thread to queue up the messages received from that specific
  client. The client message handling thread pops messages off of the queue and sends them to all clients.

  On the client side, doing this was simply an extension of Mr. Dyrland-Weavers select_client.c code. <code>select()</code> is a
  nifty method of recognizing when any file descriptor in a list of file descriptors is ready to be read from / written to. It
  allows us to circumvent the usual blocking that is caused by <code>read()</code> and <code>fgets()</code>. The system waits for
  either the standard input or server socket file descriptors to be ready for reading, and then does the required task from
  there. However, I had to implement a way for a client to break connection without breaking my server code. To do this, I
  implemented a way for the client to exit safely using <code>signal()</code>. If the client ever does a terminal interrupt
  (<code>SIGINT</code>), the system will first send <code>"/exit"</code> to the server (signaling a server logout) and will kill
  itself. Likewise, if a client types <code>"/exit"</code> into the terminal as its message, it will send the <code>SIGINT</code>
  to itself using <code>kill()</code>.    

## LIMITATIONS:

  - When a user is typing out a message, if they receive a message the received message is written over the message currently     
    being typed. On top of this, because a carriage return is used any input that was being typed out will still be written into
    standard in and read into a message.

  - The server itself has a hard time communicating with any client using a different protocol.

  - Clients receive messages they sent due to the structure of the server program.

  - If the server shutdowns before the client(s), the clients will get stuck reading and printing the last message received from  
    the server socket without stopping.

## DEVLOG:

  - Jan 14: Attempt to implement client/client communication over a forking server.
  - Jan 16: Attempt to implement client/client communication over a multithreading server. Learn multithreading.
  - Jan 17: Learn and test multithreading. Successfully implemented a threading program that connects clients.
  - Jan 18: Learn and test multithreading. Successfully implemented a mutex utilizing struct-editing program.
            Attempted to utilize forking in the chat system one last time before giving up.
            Implemented chat functions using queue.
  - Jan 19: Modified Mr. Dyrland-Weaver's networking code to accommodate threading.
            Successfully implemented message-handling thread and connecting thread in tandem with only a couple major bugs.
  - Jan 20: Fixed critical issues with networking code (client exiting issues, accessing broken pipes, attempting to write into
            non-existent file descriptors).
  - Jan 21: Fixed minor bugs and prepped for submission.
