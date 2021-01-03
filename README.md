# FILE MANAGAMENT SYSTEM

## Client Server Model

### Client side

When client program is launched, user is prompted for:
- User name 
- IP address

Client validates the IP address for the server, and establishes connection with the provided IP address if possible.
The client then sends its username as the first message to server, and on receiving response can send further commands for processing. 


### Server side

The server stores the username in an array, and responds with the current path of the user to the client. 
It is then open to incoming commands from client, and it is then upto to the client to send a command for processing.
 
When a command is received, the server:
- Processes the command
- Stores the result
- Sends the response along with the updated path of the user
- Waits for next command.
 
Server stopped by terminating program (CTRL + C in terminal)

### Message protocol

#### General packet protocol
Messages from both server and client side are divided into packets of data (currently 64 bytes).
The message is marked with terminating characters (“#$”) to indicate end of response.
On packet receival, the program reads the response packets and concatenates until the terminating characters are encountered.

#### File writing protocol
If client requests file writing permissions, the server responds with special characters (“%^”) prepended to response to indicate permission granted.
If client receives permission granted characters, it launches the function to get data from user that is going to be written into the file.
The next message from client will then be the data that is to be written in the file. 


### Connection lost error handling
If a client program is crashed, the server launches a command to close (if any) file opened by client, and end the session for that particular user.
If the serever crashes, the client side checks connection status before sending messgaes, and will end the connection if error is encountered.



## File system commands

### General commands
1. List files in current working directory
    ```
    ls
    ```
2. Change directory to given path
    ```
    cd /folder/folder2
    ```
3. Make directory in current working directory
    ``` 
    mkdir folder3
    ```
4. Move file from one location to another
    ```
    mv /path/file1 /path2/file2
    ```
5. Display memory map
    ```
    map
    ```
6. Read .dat file
    ```
    rdat
    ```
7. Delete file
    ```
    del /path/filename
    ```
8. Display help manual
    ```
    help
    ```
9. Create a file in current working directory
    ```
    cr filename
    ```
10. Open a file in current working directory with specific mode
     ```
     open filename.txt read/write
     ```
11. End program
     ```
     end
     ```
   
### File manipulation commands
Note: File must be opened, and in the appropriate mode for these commands to be accessible
1. Write into file (writes from start into new a file, and appends into file with prexisting data)
    ```
    wr
    ```
   - text writing prompt exited by entering -1 in a new line and pressing enter
2. Overwrite from specific position in file
    ```
    wrat position 
    ```
   - eg. wrat 45 starts overwriting from position 45 in file
   - text writing prompt exited by entering -1 in a new line and pressing enter
3. Truncate file to be of a specific size
    ```
    trun size 
    ```
   - eg. trun 300 reduces file size to be 300 bytes
4. Move text from a specific position, of a given size, to a specified position
    ```
    mvin start size to 
    ```
   - eg. mvin 23 40 50 takes 40 bytes of data starting from byte 23 and moves it to position 50
5. Read full file
    ```
    rd
    ```
6. Read file from specific position, upto specified size
    ```
    rf start size
    ```
   - eg. rf 244 50 reads from byte 244 for 50 bytes i.e. upto 294
7. Change mode in which file is opened
    ```
    chmod read/write
    ```
8. Close the opened file
    ```
    close
    ```
