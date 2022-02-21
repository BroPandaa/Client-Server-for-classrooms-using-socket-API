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

void DoUpdate( int sock);
void DoLocation( int sock);
void DoNumber( int sock);
void DoEnd( int sock);

int main(int argc, char **argv){
    if(argc != 3){
        printf("Usage: %s IP-address port", argv[0]);
        exit(1);
    }

    char decadeIP[20];
    strcpy(decadeIP, argv[1]);
    int port;
    sscanf(argv[2], "%d", &port);

    //socket
    int mySocket= socket(PF_INET, SOCK_STREAM, 0);
    if(mySocket== -1){
        perror("socket");
    }
    
    //connecting
    struct sockaddr_in serverAddress;

    serverAddress.sin_family= AF_INET;
    serverAddress.sin_port= htons(port);
    if(inet_aton(decadeIP, &serverAddress.sin_addr)== 0){
        printf("Wrong IP address!");
        exit(2);
    }
    memset( serverAddress.sin_zero, '\0', 8 );

    if( connect( mySocket,(struct sockaddr *) &serverAddress,sizeof( serverAddress)) == -1) perror( "connect");

    int done = 0;
    while(!done){
        printf( "\n\nChoose option...\n"
				"   1. Updating students location\n"
				"   2. Get students location\n"
				"   3. Number of students in a hall\n"
				"   4. Exit\n"
				"   \n: " );

        int option;
        scanf("%d", &option);

        switch( option )
		{
			case 1: DoUpdate( mySocket ); break;
			case 2: DoLocation( mySocket ); break;
			case 3: DoNumber( mySocket ); break;
			case 4: DoEnd( mySocket ); done = 1; break;
			default: printf( "Wrong option...\n" ); break;
		}
    }

    return 0;
}

void DoUpdate( int sock ){
    char JMBAG[20];
    char hallNumber[5];

    printf("Enter student's JMBAG:");
    scanf("%s", JMBAG);

    printf("Enter hall number:");
    scanf("%s", hallNumber);

    char message[100];
	sprintf( message, "%s %s", JMBAG, hallNumber);

    SendMessage(sock, UPDATE, message);

    int messageType; char *ans;
	if( ReceiveMessage( sock, &messageType, &ans ) != OK)
		error1( "Something went wrong while communication with server...\n");
		
	if( messageType != ANSWER)
		error1( "Something went wrong while communication with server (not sent an)...\n");
		
	printf("%s", ans);
	free(ans);
}

void DoLocation( int sock){
    char JMBAG[20];
    char hallNumber[5];

    printf("Enter student's JMBAG:");
    scanf("%s", JMBAG);

    char message[100];
	sprintf( message, "%s", JMBAG);

    SendMessage(sock, LOCATION, message);

    int messageType; char *ans;
	if( ReceiveMessage( sock, &messageType, &ans ) != OK)
		error1( "Something went wrong while communication with server...\n");
		
	if( messageType != ANSWER)
		error1( "Something went wrong while communication with server (nije poslao an)...\n");

    printf("%s", ans);
	    
	free(ans);
}

void DoNumber( int sock){
    char hallNumber[5];
    int numberOfStudents;

    printf("Enter hall number:");
    scanf("%s", hallNumber);

    SendMessage(sock, NUMBER, hallNumber);

    int messageType; char *ans;
	if( ReceiveMessage( sock, &messageType, &ans ) != OK)
		error1( "Something went wrong while communication with server...\n");
		
	if( messageType != ANSWER)
		error1( "Something went wrong while communication with server (nije poslao an)...\n");

    printf("There are %s students in hall %s",ans, hallNumber);
	    
	free(ans);
}

void DoEnd( int sock )
{
	SendMessage( sock, END, "" );

	int messageType;
	char *ans;
	if( ReceiveMessage( sock, &messageType, &ans ) != OK )
		error1( "Something went wrong while communication with server...\n" );

	if( messageType != ANSWER )
		error1( "Something went wrong while communication with server (ans not send)...\n" );

	if( strcmp( ans, "OK" ) != 0 )
		printf( "Error: %s\n", ans );
	else
	    printf( "OK\n" );
}
