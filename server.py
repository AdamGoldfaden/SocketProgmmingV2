import socket
import select
import sys
import time
import random

def main():
	udpSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	tcpSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	serverUdpPort = 4000
	serverTcpPort = 4001

	mtu = 1500
	packetSize = 1024
	password = "93"
	PasswordLen = 2
	image = "face.jpg"
	f = open(image, 'rb')
	
	IOCHUNK = 1024
	imageData = ""
	while 1:
		chunk = f.read(IOCHUNK)
		print "File chunk read",len(chunk), "B"
		if chunk:
			imageData = imageData + chunk
		else:
			break
		
	
	imageSize = len(imageData) # 3 packets
	print "Total file size:", imageSize
	
	imageSizeStr = "%9d" % imageSize
	imageSizeStr =imageSizeStr.replace(' ', '0')
	
	print "Binding sockets.."
	udpSock.bind(("0.0.0.0", serverUdpPort))
	
	tcpSock.bind(("0.0.0.0", serverTcpPort))
	tcpSock.listen(1024)
	
	print "Server service started"
	print "Udp sock:", udpSock, "Tcp sock:", tcpSock
	
	clientSock = None
	clientAddr = None
	cleintUdpAddr = None
	clientRecvBuf = ""
	clientSendBuf = ""
	clientCmd = -1
	numLogins = 0
	
	udpPacketQueue = []
	while 1:
		#check sockets for reading:
		readCheck = [udpSock, tcpSock]
		if clientSock: readCheck.append(clientSock)
		writeCheck = [udpSock]
		if clientSock: writeCheck.append(clientSock)
		errCheck = []
		if clientSock: errCheck.append(clientSock)
		
		canRead, canWrite, inError = select.select(readCheck, writeCheck, errCheck, 0.1)
		
		if udpSock in canRead:
			print "Reading UDP data",
			data, addr = udpSock.recvfrom(mtu)
			cleintUdpAddr = addr
			print "from:", addr, "data:", data
			if data.lower() == "where":
				print '  Received "where" request, sending "here" to', addr
				udpSock.sendto("here", addr)
			else:
				iPacket = int(data.lower())
				print '  Received request for file packet, scheduling send', iPacket
				udpPacketQueue.append(iPacket)
		if udpSock in canWrite and len(udpPacketQueue) > 0:
			iPacket = udpPacketQueue[0]
			udpPacketQueue = udpPacketQueue[1:]
			print "Sending file packet", iPacket
			dataStart = packetSize * iPacket
			dataEnd = min(packetSize * (iPacket+1), len(imageData))
			packetData = imageData[dataStart: dataEnd]
			packetHeader = ("%9d" % iPacket).replace(' ', '0')
			packetData = packetHeader + packetData
			print "  Sending udp packet of size", len(packetData), "B", "to", cleintUdpAddr
			if random.randint(0, 5) == 0:
				print "  Packet Dropped!"
			else:
				sent = udpSock.sendto(packetData, cleintUdpAddr)
				print "  Sent", sent, "B"
		if tcpSock in canRead:
			print "Accepting client"
			clientSock = tcpSock.accept()
			print "  Accepted:", clientSock
			clientAddr = clientSock[1]
			clientSock = clientSock[0]
		
		if clientSock:
			if clientSock in inError:
				print "Disconnecting client, flushing buffer.."
				clientSock, clientRecvBuf, clientSendBuf, clientCmd, numLogins = None, "", "", -1, 0
		
			if clientSock in canRead:
				buf = clientSock.recv(1024)
				if buf == '':
					print "Disconnecting client, flushing buffer.."
					clientSock, clientRecvBuf, clientSendBuf, clientCmd, numLogins = None, "", "", -1, 0
				else:
					clientRecvBuf = clientRecvBuf + buf
		
		if clientSock and len(clientSendBuf) > 0:
			#print have stuff to send
			if clientSock in canWrite:
				print "Sending tcp stream to client..",
				sent = clientSock.send(clientSendBuf)
				print "sent", sent, "B"
				clientSendBuf = clientSendBuf[sent:]
		
		#parse input stream
		if len(clientRecvBuf) > 0:
			print "Parsing Tcp input stream.."
			if clientCmd == -1:
				#no command yet, could be login: or image:
				if len(clientRecvBuf) >= 6:
					if clientRecvBuf[:6] == "login:":
						clientRecvBuf = clientRecvBuf[6:]
						if numLogins > 0:
							print "  Too many login tries!"
							print "  Disconnecting client, flushing buffer.."
							clientSock, clientRecvBuf, clientSendBuf, clientCmd, numLogins = None, "", "", -1, 0
						else:
							print "  Starting client command login:"
							numLogins += 1
							clientCmd = 1
					elif clientRecvBuf[:6] == "image:":
						clientRecvBuf = clientRecvBuf[6:]
						print "  Starting client command image:"
						clientCmd = 2
			elif clientCmd == 1:
				#passwords
				if not clientRecvBuf[0] in '0123456789':
					print "  Done parsing passwords"
					clientCmd = -1
				else:
					if len(clientRecvBuf) >= PasswordLen:
						tryPassword = clientRecvBuf[:PasswordLen]
						clientRecvBuf = clientRecvBuf[PasswordLen:]
						print "  Checking password:", tryPassword,
						clientSendBuf = clientSendBuf + "yes" if tryPassword == password else "no"
						print 'result:  "%s"' % clientSendBuf, "added to Tcp stream"
			elif clientCmd == 2:
				#need to check correct password
				if len(clientRecvBuf) >= PasswordLen:
					tryPassword = clientRecvBuf[:PasswordLen]
					clientRecvBuf = clientRecvBuf[PasswordLen:]
					if tryPassword == password:
						print '  Password OK, adding image size to Tcp output stream: "%s"' % imageSizeStr
						clientSendBuf += imageSizeStr
					else:
						print "  Incorrect password for image, disconnecting!"
						print "  Disconnecting client, flushing buffer.."
						clientSock, clientRecvBuf, clientSendBuf, clientCmd, numLogins = None, "", "", -1, 0
			
		time.sleep(0.01)
if __name__ == "__main__":
	main()
	
