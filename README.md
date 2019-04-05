# SocketProgmmingV2
This program is about 'hacking' an image server and retrieving a secret image file using networking sockets.  It is split into the 5 following sections.

Section 1 - An mage server sits somewhere on the network and it can be reached by a UDP broadcast message containing data "where" at server udp port 4000.  Once thenserver receives "where" it will send a udp packet back with data "here".  The address the packet came from is the address of the Image Server. Once we have that address, we an establish a Tcp connection at that address and server tcp port 4001

Section 2 - Using the Tcp connection, we can send two commands: 
"login:<password>", and the server will reply "yes" or "no" 
"image:<password>", andn the server will reply with a 9 digit number
But we can only send each one of these commands once! If the server gets a command twice it will block our ip and will not allow to do it again. (In reality it will just drop the connection ).  Obviously we need to know the password. Password is 2 digit number, "00" or "01" ... "99". Here's where hacking comes in: we found out that the programmer who wrote "login:<password>" protocol, dealing with Tcp buffers, wrote a while loop to check incoming stream, meaning if more than one password is provided within single login command, he will send more than one reply. So if data sent is "login:000503" the Tcp stream reply will be "noyesno" if 05 is the password.  This means if we put all possible password into the login command you can figure out what the password is, by just looking at sequence of replies.  So to summarize, section 2 is: send login command with all possible passwords, and based on Tcp reply, figure out what the password is.
  
Section 3 - Once we know the password, retrieving the image size if easy, so we just need to send "image:<password>" 8B to image server.  The reply from the server will be a 9 digit number. Leading digits can be zero, as in "000123456", but the string will always be 9 digits. The number is the size of the secret image .jpg file. 
  
Section 4 - The server also supports a UDP file chunk request protocol. If Udp packet is sent with data "<9 digit number>", a corresponding 1024B chunk will be sent as a reply preceded with the same number. So if UDP packet sent is "000000001" the reply will be "000000001<up to 1024B of data>". Also, if we send an invalid request, we are likely to crash the server, that's why it is very important to know the size of the file from section 3.  The server actually drops packets on purpose so a timeout protocol has also been implemented here so if we don't get a packet within 5 seconds, another request is sent.  Once we get all of the packets, they are stored in a large data buffer. 

Section 5 - Once we have all the chunks, they are dumped into a new binary file called "result.jpg", which should look exactly like "face.jpg"
