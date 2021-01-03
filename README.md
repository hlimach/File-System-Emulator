FILE MANAGAMENT SYSTEM

Protocol:

When client program is launched, the client asks for a user name and the IP address for the server and validates the IP address. Then the program makes the connection with the provided IP address

When the connection is made, the client sends the username and the server stores it in an array. Then server sends the current path of the user to the corresponding client and waits for the command. It is then up to the client to send a command for processing.
 
When the command is received, the server does the necessary processing and stores all the appropriate responses and then sends the response along with the path of user to the corresponding client and waits for next command.
 
The protocol applied in our project is that whenever a response is sent from client or server, it is divided into chunks of 24 bytes each and the final response is concatenated with a sequence of special characters known as terminating characters (“#$”) to indicate end of response.

When receiving the response, the program reads the response in chunks of 24 bytes and concatenates it into a final string until the final characters are the terminating characters.

When the server requires data to be written in a particular file, it prepends the response with a set of special characters (“%^”) to indicate that the next response from client will be data that is to be written in the file. In the client side, when the first 2 characters of response from server are “%^”, the client launches the function to get data from user that is going to be written in the file.

In case if a client program is crashed, the server launches a command to override close the file if opened and end the thread for that particular user.

In case if the serever crashes, the client will try sending a message and when no response is recieved, the client program will display an appropriate message and end.

commands are accessible using help command at the client side.