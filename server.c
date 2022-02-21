#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "protocol.h"

#include <pthread.h>

void EndCommunication( void *parameter);
void *CommunicateWithClient(void *parameter);
void DoUpdate( int sock, char *message ); //updates students location
void DoLocation( int sock, char *message ); //sends students location
void DoNumber( int sock, char *message );//sends number of students in a hall
int InHall(char *hallNumber);//given number of students in a hall

#define MAXSTUDENTS 200
#define MAXHALLS 20
#define MAXTHREADS 3

typedef struct infos{
	char JMBAG[20];
	char hallNumber[5];
}info;

typedef struct
{
	int commSocket;
	int threadIndex;
} communicate__parameter;

int numberOfStudents = 0, numberOfHalls = 0;
int activeThreads[MAXTHREADS] = { 0 };
info table[MAXSTUDENTS];

communicate__parameter parameterThread[MAXTHREADS];

pthread_mutex_t lockActiveThreads = PTHREAD_MUTEX_INITIALIZER;

int main( int argc, char **argv )
{
	if( argc != 2 )
		error2( "Usage: %s port\n", argv[0] );
		
	int port; sscanf( argv[1], "%d", &port );
	
	// socket...
    int listenerSocket = socket( PF_INET, SOCK_STREAM, 0 );
	if( listenerSocket == -1 )
		myperror( "socket" );
		
	// bind...
	struct sockaddr_in myAddress;

	myAddress.sin_family = AF_INET;
	myAddress.sin_port = htons( port );
	myAddress.sin_addr.s_addr = INADDR_ANY;
	memset( myAddress.sin_zero, '\0', 8 );
	
	if( bind( listenerSocket,
		      (struct sockaddr *) &myAddress,
		      sizeof( myAddress ) ) == -1 )
		myperror( "bind" );
		
	// listen
	if( listen( listenerSocket, 10 ) == -1 )
		myperror( "listen" );
		
	pthread_t threads[10];

	// accept
	while( 1 )
	{
        struct sockaddr_in clientAddress;
		int lenAddr = sizeof( clientAddress );

		int commSocket = accept( listenerSocket,
			                     (struct sockaddr *) &clientAddress,
			                     &lenAddr );

		if( commSocket == -1 )
			myperror( "accept" );

		char *decadeIP = inet_ntoa( clientAddress.sin_addr );
		printf( "Accepted connection from od %s [socket=%d]\n", decadeIP, commSocket );

		pthread_mutex_lock( &lockActiveThreads );
		int i, indexNonActive = -1;
		for( i = 0; i < MAXTHREADS; ++i )
			if( activeThreads[i] == 0 )
				indexNonActive = i;
			else if( activeThreads[i] == 2 )
			{
				pthread_join( threads[i], NULL );
				activeThreads[i] = 0;
				indexNonActive = i;
			}

		if( indexNonActive == -1 )
		{
			close( commSocket); // no more threads...
			printf( "No more free threads.\n");
		}
		else
		{
		    activeThreads[indexNonActive] = 1;
			parameterThread[indexNonActive].commSocket = commSocket;
			parameterThread[indexNonActive].threadIndex = indexNonActive;
			printf( "Using thread number %d.\n", indexNonActive );

			pthread_create(
				&threads[indexNonActive], NULL,
				CommunicateWithClient, &parameterThread[indexNonActive] );
		}
		pthread_mutex_unlock( &lockActiveThreads );
	}
	
	return 0;
}

void EndCommunication( void *parameter)
{
	communicate__parameter *param = (communicate__parameter *) parameter;
	int commSocket  = param->commSocket;
	int threadIndex = param->threadIndex;

	printf( "End of communication [dretva=%d]\n", threadIndex);

	pthread_mutex_lock( &lockActiveThreads);
	activeThreads[ threadIndex ] = 2;
	pthread_mutex_unlock( &lockActiveThreads);

	close( commSocket);
}

void *CommunicateWithClient(void *parameter)
{	
	communicate__parameter *param = (communicate__parameter *) parameter;
	int sock  = param->commSocket;

	int typeOfMessage, done = 0;
	char *message;
	
	while( !done )
	{
		if( ReceiveMessage( sock, &typeOfMessage, &message ) != OK )
		{
			printf( "Failure while communication with client at [socket=%d]...\n", sock );
			EndCommunication(parameter);
			done = 1;
			continue;
		}
		
		switch( typeOfMessage )
		{
			case UPDATE: DoUpdate( sock, message ); break; 
			case LOCATION: DoLocation( sock, message ); break;
			case NUMBER: DoNumber( sock, message ); break;
			case END: SendMessage( sock, ANSWER, "OK" ); done = 1; break;
			default: SendMessage( sock, ANSWER, "Message code does not exist!\n" );
		}
		
		free( message );
	}
}

void DoUpdate( int sock, char *message){

	char JMBAG[20], hallNumber[5];

	if( sscanf( message, "%s %s\n", JMBAG, hallNumber) != 2)
	{
		SendMessage( sock, ANSWER, "Wrong form of UPDATE command." );
		return;
	}

	if(numberOfStudents==MAXSTUDENTS){
		SendMessage( sock, ANSWER, "Already at the maximum of students!");
		return;
		}

	int j, existID=0, exist=0;

	for(j=0; j<numberOfStudents; j++)
		if(strcmp(table[j].hallNumber, hallNumber)==0){
			existID=1;
			break;
		}

	if(!existID && numberOfHalls==MAXHALLS){
		SendMessage( sock, ANSWER, "Already at the maximum of halls!");
		return;
		}
	if(!existID && numberOfHalls!=MAXHALLS) numberOfHalls++;

	int i;
	for(i=0; i<numberOfStudents; i++){
		if(strcmp(table[i].JMBAG, JMBAG)== 0){
			strcpy(table[i].hallNumber, hallNumber);
			exist=1;
			SendMessage( sock, ANSWER, "Data updated.");
			break;
		}
	}

	if(!exist){
		strcpy(table[i].JMBAG, JMBAG);
		strcpy(table[i].hallNumber, hallNumber);
		numberOfStudents++;
		SendMessage( sock, ANSWER, "Student added into the table.");
	}
	
}

void DoLocation( int sock, char *message){

	const char *JMBAG= message;

	int i, location = 0;
	for( i = 0; i < numberOfStudents; ++i )
		if( strcmp( table[i].JMBAG, JMBAG ) == 0 )
		{
			char msg[200];
			sprintf( msg, "Student with JMBAG %s is in hall number %s.\n", JMBAG, table[i].hallNumber);
			SendMessage( sock, ANSWER, msg );

			location = 1;
			break;
		}

	if( !location ){
		char msg[200];
		sprintf( msg, "Student with JMBAG %s is not in any hall.\n", message);
		SendMessage( sock, ANSWER, msg);
	}
}

void DoNumber( int sock, char *message){

	int number=InHall(message);

	char msg[10];
	sprintf(msg, "%d", number);

	SendMessage( sock, ANSWER, msg);
}

int InHall(char *hallNumber){
	int i, number=0;
	if(numberOfStudents==0) return 0;
	for(i=0; i<numberOfStudents; i++){
		if(strcmp(table[i].hallNumber, hallNumber)==0) number++;
	}
	
	return number;
}