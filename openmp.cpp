#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "common.h"
#include "omp.h"

//
//  benchmarking program
//
int main( int argc, char **argv )
{   
    int numthreads;

    if( find_option( argc, argv, "-h" ) >= 0 )
    {
        printf( "Options:\n" );
        printf( "-h to see this help\n" );
        printf( "-n <int> to set number of particles\n" );
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
    set_len( n );
    init_bar( tnodes, (double) 1.0, 400, 200 );

    //
    //  simulate a number of time steps
    //
    double simulation_time = read_timer( );

    #pragma omp parallel
    {
    numthreads = omp_get_num_threads();
    for( int step = 0; step < NSTEPS; step++ )
    {
        //
        //  sum temperatures for approximation
        //
        #pragma omp for
        for( int i = 1; i < n-1; i++ )
        {
          apply_tsum( tnodes[i], tnodes[i-1]);
          apply_tsum( tnodes[i], tnodes[i+1]);
        }
        
		
        //
        //  move particles
        //
        #pragma omp for
        for( int i = 0; i < n; i++ ) 
          tupdate( tnodes[i], 1);   
  
        if( find_option( argc, argv, "-no" ) == -1 ) 
        {
          //
          //  save if necessary
          //
          #pragma omp master
          if( fsave && (step%SAVEFREQ) == 0 )
              save( fsave, step, n, tnodes );
        }
    }
}
    simulation_time = read_timer( ) - simulation_time;
    
    printf( "n = %d,threads = %d, simulation time = %g seconds\n", n,numthreads, simulation_time);

    //
    // Printing summary data
    //
    // if( fsum)
    //     fprintf(fsum,"%d %d %g\n",n,numthreads,simulation_time);

    //
    // Clearing space
    //
    if( fsum )
        fclose( fsum );

    free( tnodes );
    if( fsave )
        fclose( fsave );
    
    return 0;
}
