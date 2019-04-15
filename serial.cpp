#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "common.h"

//
//  benchmarking program
//
int main( int argc, char **argv )
{    

    if( find_option( argc, argv, "-h" ) >= 0 )
    {
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
    set_len( n );
    init_bar( tnodes, 400, 200 );

    if( find_option( argc, argv, "-no" ) == -1 )
        {
          //
          //  save if necessary
          //
          if( fsave )
              save( fsave, n, tnodes );
        }
    
    //
    //  simulate a number of time steps
    //
    double simulation_time = read_timer( );
	
    for( int step = 0; step < NSTEPS; step++ )
    {
        //
        //  sum temperatures for approximation
        //
        for( int i = 0; i < n; i++ )
        {
            for (int j = 0; j < n; j++ )
				apply_tsum( tnodes[i], tnodes[j]);
        }
 
        //
        //  move particles
        //
        for( int i = 0; i < n; i++ ) 
            tupdate( tnodes[i] );		

        if( find_option( argc, argv, "-no" ) == -1 )
        {
          //
          //  save if necessary
          //
          if( fsave && (step%SAVEFREQ) == 0 )
              save( fsave, n, tnodes );
        }
    }
    simulation_time = read_timer( ) - simulation_time;
    
    printf( "n = %d, simulation time = %g seconds\n", n, simulation_time);

    //
    // Printing summary data
    //
    if( fsum) 
        fprintf(fsum,"%d %g\n",n,simulation_time);
 
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
