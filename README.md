# SocketProgmmingV2
This program is about hacking an image server and retrieving a secret image file using networking sockets.  It is split into the 5 following sections.

Section 1 - An image server sits somewhere on the network at an unknown ip and can be reached by a UDP broadcast message containing data "where" at server udp port 4000.  Once the server receives "where" it will send a udp packet back with data "here".  The address the packet came from is the address of the Image Server. Once we have that address, we an establish a Tcp connection at that address and server tcp port 4001

Section 2 - Using the Tcp connection, we can send two commands: 
"login:<password>", and the server will reply "yes" or "no" 
"image:<password>", and the server will reply with a 9 digit number
But we can only send each one of these commands once because if the server gets a command twice it will block our ip and will not allow us to do it again by dropping the connection.  The password is 2 digit number and we can find it by puting all possible password into the login command so we can figure out what the password is by just looking at sequence of replies.  

Section 3 - Once we know the password we just need to send "image:<password>" 8B to image server.  The reply from the server will be a 9 digit number.  The number is the size of the secret image .jpg file. 
  
Section 4 - The server also supports a UDP file chunk request protocol. If Udp packet is sent with data "<9 digit number>", a corresponding 1024B chunk will be sent as a reply preceded with the same number. Also, if we send an invalid request, we are likely to crash the server.  The server actually will drop packets on purpose so a timeout protocol has also been implemented here so if we don't get a packet within 5 seconds, another request is sent.

Section 5 - Once we have all the chunks, they are dumped into a new binary file called "result.jpg", which should look exactly like "face.jpg"
