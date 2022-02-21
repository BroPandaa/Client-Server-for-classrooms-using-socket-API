#ifndef _PROTOKOL_H_
#define _PROTOKOL_H_

#define UPDATE  1
#define LOCATION    2
#define LOCATION_R  3
#define NUMBER        4
#define NUMBER_R      5
#define END        6
#define ANSWER     7

#define OK      1
#define NOT_OK  2

#define error1( s ) { printf( s ); exit( 0 ); }
#define error2( s1, s2 ) { printf( s1, s2 ); exit( 0 ); }
#define myperror( s ) { perror( s ); exit( 0 ); }

int ReceiveMessage( int sock, int *vrstaPoruke, char **poruka );
int SendMessage( int sock, int vrstaPoruke, const char *poruka );

#endif 