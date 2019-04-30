#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "common.h"

int main(int argc, char **argv) {
  if (find_option(argc, argv, "-h") >= 0) {
    printf( "Options:\n" );
    printf( "-h to see this help\n" );
    printf( "-n <int> to set the number of particles\n" );
    printf( "-o <filename> to specify the output file name\n" );
    printf( "-s <filename> to specify a summary file name\n" );
    printf( "-no turns off all correctness checks and particle output\n");
    return 0;
  }
  int n = read_int( argc, argv, "-n", 1000 );

  char *savename = read_string( argc, argv, "-o", NULL );
  char *sumname = read_string( argc, argv, "-s", NULL );

  FILE *fsave = savename ? fopen( savename, "w" ) : NULL;
  FILE *fsum = sumname ? fopen ( sumname, "a" ) : NULL;

  node_t *tnodes = (node_t *) malloc( n * sizeof(node_t) );
  set_len(n);
  init_bar( tnodes, (double) 1.0, 400, 200 );

  // Set up MPI
  int n_proc, rank;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &n_proc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Datatype NODE;
  int blocklen[2] = {3, 1};
  MPI_Aint displacements[2] = {0, 24};
  MPI_Datatype types[2] = {MPI_DOUBLE, MPI_INT};
  MPI_Type_create_struct(2, blocklen, displacements, types, &NODE);
  MPI_Type_commit(&NODE);

  // Partition the nodes across n_proc processors by x value
  MPI_Bcast(tnodes, n, NODE, 0, MPI_COMM_WORLD);
  double width = 1.0 / n_proc;
  int lindex = rank * n / n_proc;
  int rindex = (rank + 1) * n / n_proc;

  printf("rank: %d\n", rank);
  printf("lindex: %d, rindex: %d\n", lindex, rindex);

  node_t *recv_buffer = (node_t *) malloc(n * sizeof(node_t));

  int tag;
  int dest_rank, source_rank;
  MPI_Status status;

  double simulation_time = read_timer( );
  for (int step = 0; step < NSTEPS; ++step) {
    // Compute temperature changes
    for (int i = lindex; i < rindex; ++i) {
      apply_tsum(tnodes[i], tnodes[i-1]);
      apply_tsum(tnodes[i], tnodes[i+1]);
    }

    for (int i = lindex; i < rindex; ++i) {
      tupdate(tnodes[i], 1);
    }

    // Send adjacent particles to adjacent processors
    // Send to left
    dest_rank = (rank == 0) ? MPI_PROC_NULL : (rank - 1);
    source_rank = (rank == n_proc - 1) ? MPI_PROC_NULL : (rank + 1);
    MPI_Sendrecv(&tnodes[lindex], 1, NODE, dest_rank, 0,
                 recv_buffer, 1, NODE, source_rank, 0,
		 MPI_COMM_WORLD, &status);
    if (rank != n_proc - 1) {
	    tnodes[rindex] = recv_buffer[0];
    }

    // Send to right
    dest_rank = (rank == n_proc - 1) ? MPI_PROC_NULL : (rank + 1);
    source_rank = (rank == 0) ? MPI_PROC_NULL : (rank - 1);
    MPI_Sendrecv(&tnodes[rindex-1], 1, NODE, dest_rank, 0,
		 recv_buffer, 1, NODE, source_rank, 0,
		 MPI_COMM_WORLD, &status);
    if (rank != 0)
	    tnodes[lindex-1] = recv_buffer[0];

    if( find_option( argc, argv, "-no" ) == -1 ) {
	    if( fsave && (step % SAVEFREQ == 0)) {
		    MPI_Allgather(&tnodes[lindex], rindex - lindex, NODE,
				    recv_buffer, rindex - lindex, NODE, MPI_COMM_WORLD);
		    if (rank == 0) {
			    save( fsave, step, n, recv_buffer );
		    }
	    }
    }
  }
  simulation_time = read_timer( ) - simulation_time;

  MPI_Allgather(&tnodes[lindex], rindex - lindex, NODE,
		  recv_buffer, rindex - lindex, NODE, MPI_COMM_WORLD);

  if (0 == rank) {
    printf( "n = %d, simulation time = %g seconds\n", n, simulation_time);
  }

  if( fsum )
    fclose( fsum );    
  free( tnodes );
  if( fsave )
    fclose( fsave );

  MPI_Finalize();

  return 0;
}
