#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "common.h"

int mesh_pts;

//
//  tuned constants
//  assume copper bar
//
#define cond          413 // W/m-K
#define dx          0.005 // m
#define dt         0.0005 // s
#define T_default     300 // K

//
//  timer
//
double read_timer( )
{
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

//
//  Set number of mesh points
//
void set_len( int n )
{
    mesh_pts = n;
}

//
//  Initialize the bar
//
void init_bar( node_t *tnodes, double ltem, double rtem )
{        
    // Number of nodes to create
    
    tnodes[0].T = ltem;
    tnodes[0].T_sum = 0;
    tnodes[0].x = 0;
    tnodes[0].fixed = true;
    tnodes[mesh_pts-1].T = rtem;
    tnodes[mesh_pts-1].T_sum = 0;
    tnodes[mesh_pts-1].x = (mesh_pts-1) * dx;
    tnodes[mesh_pts-1].fixed = true;

    for (int i = 1; i < mesh_pts-1; i++) {
        tnodes[i].T = T_default;
        tnodes[i].T_sum = 0;
        tnodes[i].x = (double) i * dx;
        tnodes[i].fixed = false;
    }
}

//
//  interact two temperature nodes
//
void apply_tsum( node_t &tnode, node_t &neighbor)
{
    if (tnode.fixed) {
        return;
    }
    double dist = fabs(tnode.x - neighbor.x);
    if ( dist <= dx + 0.001 && dist != 0 ) {
        tnode.T_sum += neighbor.T;
    }
}

//
//  Solve for the temperature
//
void tupdate( node_t &tnode )
{
    if (tnode.fixed) {
        return;
    }
    tnode.T = ((double) tnode.T_sum) / ((double) 2);
    tnode.T_sum = 0;
}

//
//  I/O routines
//
void save( FILE *f, int n, node_t *tnode )
{
    static bool first = true;
    if( first )
    {
        fprintf( f, "%d\n", n );
        first = false;
    }
    for( int i = 0; i < n; i++ )
        fprintf( f, "%g,%g\n", tnode[i].T, tnode[i].x );
}

//
//  command line option processing
//
int find_option( int argc, char **argv, const char *option )
{
    for( int i = 1; i < argc; i++ )
        if( strcmp( argv[i], option ) == 0 )
            return i;
    return -1;
}

int read_int( int argc, char **argv, const char *option, int default_value )
{
    int iplace = find_option( argc, argv, option );
    if( iplace >= 0 && iplace < argc-1 )
        return atoi( argv[iplace+1] );
    return default_value;
}

char *read_string( int argc, char **argv, const char *option, char *default_value )
{
    int iplace = find_option( argc, argv, option );
    if( iplace >= 0 && iplace < argc-1 )
        return argv[iplace+1];
    return default_value;
}
