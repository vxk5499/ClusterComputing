// greetings.c
//
#include <stdio.h>
#include <string.h>
#include "mpi.h"
main( int argc, char *argv[] )
{
// General identity information
int my_rank; // Rank of process
int p; // Number of processes
char my_name[100]; // Local processor name
int my_name_len; // Size of local processor name
// Message packaging
int source;
int dest;
int tag=0;
char message[100];
MPI_Status status;
//
// Start MPI
//
MPI_Init( &argc, &argv );
// Get rank and size
MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );
MPI_Comm_size( MPI_COMM_WORLD, &p );
MPI_Get_processor_name( my_name, &my_name_len );
if( my_rank != 0 )
{
// Create the message
sprintf( message, "Greetings from process %d!", my_rank );
// Send the message
dest = 0;
MPI_Send( message, strlen(message)+1, MPI_CHAR,
dest, tag, MPI_COMM_WORLD );
}
else
{
for( source = 1; source < p; source++ )
{
MPI_Recv( message, 100, MPI_CHAR, source,
tag, MPI_COMM_WORLD, &status );
printf( "%s\n", message );
}
}
// Print the closing message
printf( "Process %d of %d on %s done\n", my_rank, p, my_name );
MPI_Finalize();
}