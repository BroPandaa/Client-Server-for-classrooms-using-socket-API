# Client-Server-for-classrooms-using-socket-API
This is a client and server app made using Socket API for the college subject Computer Networks.
The idea of the app is for server to store the data in the table which contains two type of data, students' IDs and hall numbers. Server processes each client in its own thread. We have three types of comunication between the client and the server:
1. Client send info to the server that the student with the given ID is in the given hall. Server sends the message which confirms the that the message is received and updates the data
2. Client asks where is the student with the given ID and the server replies with sending the hall number
3. Client asks how many students are in the hall with the given ID and the server replies by sending the number of the students
