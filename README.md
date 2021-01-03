#FILE MANAGAMENT SYSTEM

##Client Server Model:

###Client side:

When client program is launched, user is prompted for:
- User name 
- IP address

Client validates the IP address for the server, and establishes connection with the provided IP address if possible.
The client then sends its username as the first message to server, and on receiving response can send further commands for processing. 


###Server side:

The server stores the username in an array, and responds with the current path of the user to the client. 
It is then open to incoming commands from client, and it is then upto to the client to send a command for processing.
 
When a command is received, the server:
- Processes the command
- Stores the result
- Sends the response along with the updated path of the user
- Waits for next command.
 

###Message protocol:

####General packet protocol:
Messages from both server and client side are divided into packets of data (currently 64 bytes).
The message is marked with terminating characters (“#$”) to indicate end of response.
On packet receival, the program reads the response packets and concatenates until the terminating characters are encountered.

####File writing protocol:
If client requests file writing permissions, the server responds with special characters (“%^”) prepended to response to indicate permission granted.
IF client receives permission granted characters, it launches the function to get data from user that is going to be written into the file.
The next message from client will then be the data that is to be written in the file. 


###Connection lost error handling:
If a client program is crashed, the server launches a command to close (if any) file opened by client, and end the session for that particular user.
If the serever crashes, the client side checks connection status before sending messgaes, and will end the connection if error is encountered.

commands are accessible using help command at the client side.
