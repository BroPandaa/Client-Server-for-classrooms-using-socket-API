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

int SendMessage( int sock, int messageType, const char *message )
{
	int messageLength = strlen( message );

	int sent, lastSent;
	// radi jednostavnosti pretpostavljamo da ce messageLength i kodPoruke biti
	// uspjesno poslani odjednom (a ne u vise pokusaja)
	int messageLength_n = htonl( messageLength );
	lastSent = send( sock, &messageLength_n, sizeof( messageLength_n ), 0 );
	if( lastSent != sizeof( messageLength_n ) ) return NOT_OK;

	int messageType_n = htonl( messageType );
	lastSent = send( sock, &messageType_n, sizeof( messageType_n ), 0 );
	if( lastSent != sizeof( messageType_n ) ) return NOT_OK;

	sent = 0;
	while( sent != messageLength )
	{
		lastSent = send( sock, message + sent, messageLength - sent, 0 );

		if( lastSent == -1 || lastSent == 0 ) return NOT_OK;
		sent += lastSent;
	}

	return OK;
}


int ReceiveMessage( int sock, int *messageType, char **message )
{
	int received, receiveLast;
	// radi jednostavnosti pretpostavljamo da ce messageLength i kodPoruke biti
	// uspjesno primljeni odjednom (a ne u vise pokusaja)
	int messageLength_n, messageLength;
	receiveLast = recv( sock, &messageLength_n, sizeof( messageLength_n ), 0 );
	if( receiveLast != sizeof( messageLength_n ) ) return NOT_OK;
	messageLength = ntohl( messageLength_n );

	int messageType_n;
	receiveLast = recv( sock, &messageType_n, sizeof( messageType_n ), 0 );
	if( receiveLast != sizeof( receiveLast ) ) return NOT_OK;
	*messageType = ntohl( messageType_n );

	*message = (char *) malloc( ( messageLength + 1 ) * sizeof( char ) );

	received = 0;
	while( received != messageLength )
	{
		receiveLast = recv( sock, *message + received, messageLength - received, 0 );

		if( receiveLast == -1 || receiveLast == 0 ) return NOT_OK;
		received += receiveLast;
	}

	(*message)[ messageLength ] = '\0';

	return OK;
}

