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
double step;

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
void init_bar( node_t *tnodes, double bar_size, double ltem, double rtem )
{        
    // Number of nodes to create
    step = 1.0/(mesh_pts-1);
    for (int i = 0; i < mesh_pts; i++) {
        for (int j = 0; j < mesh_pts; j++) {
            tnodes[mesh_pts*i + j].T = T_default;
            tnodes[mesh_pts*i + j].T_sum = 0;
            tnodes[mesh_pts*i + j].x = (double) step*j;
            tnodes[mesh_pts*i + j].y = (double) step*i;
            tnodes[mesh_pts*i + j].fixed = false;
            tnodes[mesh_pts*i + j].edge = false;
            tnodes[mesh_pts*i + j].corner = false;
        }  
    }

    tnodes[0].corner = true;
    tnodes[mesh_pts*(mesh_pts-1)].corner = true;
    tnodes[mesh_pts-1].corner = true;
    tnodes[mesh_pts*(mesh_pts-1) + mesh_pts-1].corner = true;

    for (int i = 0; i < mesh_pts; i++) {
        tnodes[mesh_pts*i].edge = true;
        tnodes[mesh_pts*i + mesh_pts - 1].edge = true;
        tnodes[i].edge = true;
        tnodes[mesh_pts*(mesh_pts - 1) + i].edge = true;
    }

    for (int i = mesh_pts/10; i < mesh_pts - mesh_pts/10; i++) {
        int j;
        for (j = 0; j < mesh_pts - mesh_pts/10; j++) {
            if (i == mesh_pts/10) {
                tnodes[mesh_pts*(i-1) + j].edge = true;
            } else if (i == mesh_pts - mesh_pts/10 - 1) {
                tnodes[mesh_pts*(i+1) + j].edge = true;
            }
            tnodes[mesh_pts*i + j].x = -100;
        }
        tnodes[mesh_pts*i + j].edge = true;
    }

    for (int i = 0; i < mesh_pts/10; i++) {
        tnodes[mesh_pts*i].T = ltem;
        tnodes[mesh_pts*i].fixed = true;
        tnodes[mesh_pts*(mesh_pts-i-1)].T = rtem;
        tnodes[mesh_pts*(mesh_pts-i-1)].fixed = true;
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
    if ( dist < 10) {
        tnode.T_sum += neighbor.T;
    }
}

//
//  Solve for the temperature
//
void tupdate( node_t &tnode, double div )
{
    if (tnode.fixed) {
        return;
    }

    tnode.T = ((double) tnode.T_sum) / ((double) div);
    tnode.T_sum = 0;
}

//
//  I/O routines
//
void save( FILE *f, int step, int n, node_t *tnode )
{
    //static bool first = true;
    //if( first )
    //{
    //    fprintf( f, "%d\n", n );
    //    first = false;
    //}
    for( int i = 0; i < n*n; i++ ) {
        if (tnode[i].x > -1)
            fprintf( f, "%d,%g,%g,%g\n", step, tnode[i].x, tnode[i].y, tnode[i].T);
    }
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
