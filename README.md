# share-it
Shareit is a lightweight, terminal-based file sharing application for Linux systems, inspired by the original ShareIt app. It allows seamless file transfers across systems within the same local network using a hybrid UDP + TCP architecture.

## Features
1. LAN Discovery with UDP Broadcast
Receivers broadcast a discovery message over the network; all available senders respond with their details.

2. Multi-Server Awareness
The receiver lists all available servers (senders) on the LAN, allowing the user to choose which one to connect to.

3. TCP-Based File Transfer
After selecting a sender, a TCP connection is established to reliably transfer the file.

4.  Supports Any File Type
Transfer images, documents, audio files, and more — all handled in binary mode.

5. Simultaneous Transfers
supports multiple receivers connecting to the same sender at once. Each receiver gets the file through a separate TCP connection, enabling efficient, concurrent transfers.

## How It Works
### Sender Mode:
The sender runs the app and selects a file to share. It listens for discovery requests via UDP and responds with its IP and hostname.

### Receiver Mode:
The receiver runs the app in "receive" mode. It sends out a UDP broadcast to discover active senders on the network.

### User Selection:
The user is shown a list of available servers (senders). Upon choosing one, the receiver connects to that server via TCP and receives the file's metadata and file content.

## Help
```
To send a file 	: shareit -s <file>
To receive from	: shareit -r
```

## Server Demo : 
https://github.com/user-attachments/assets/87a21c11-8010-4171-ac92-7a0d37d8c61b

## Client Demo:
https://github.com/user-attachments/assets/b0af6470-f9d3-49b2-b141-2dc433fbae21





