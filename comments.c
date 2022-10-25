/*
    Generally speaking, here's how the program went.


    For both server and client, a common setup is needed, mainly about the WSADATA, a structure that contains details about the windows implementation of sockets, and about addrinfo structures. Both have two common addrinfo structures: result and hints. With the initial informations that hints gives, we set results and are good to go.

    
    SERVER:
    With a hostent structure, we can get the IP that the client will need to connect to the server. On the page about hostent on learn.microsoft.com: "If using a local hosts file, it is the first entry after the IPv4 address." Thus, the IP returned will be a IPV4 one, and that should be considered when setting the ai_family of hints back there.
    Then, it is created a SOCKET object, ListenSocket, to which we give the value returned by the socket() function.
    After that, we bind it to a network address within the system with the bind() function, free the results structure, and call the listen() function to so the server will listen for incoming connection requests.
    A new SOCKET object, ClientSocket, is then created, and we pass it as a argument to the accept() function. We accept the first request for a connection that appears.
    After that, through the recv() and send() functions, that client and server can both exchange messages between each other through the Command Prompt.

    CLIENT:
    The logic on the client's side is, on the largest part, very similar. After calling the getaddrinfo() function, we create a SOCKET object, ConnectSocket, and call the socket() function to set things up. Then, we call the connect() function so the the ConnectSocket connects to the IP passed as the argv[1]. After that, the client starts to receive and send data. The code in this part is pretty much exactly the same to the server's side.

    If any side wants to end the connection, sending the message "close connection" will do the job.


*/