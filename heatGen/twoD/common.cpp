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
#define dx          0.005 // m
#define dt         0.0005 // s
#define T_default     200 // K

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
    for (int j = 0; j < mesh_pts; j++) {
        tnodes[j].T = ltem;
        tnodes[j].T_sum = 0;
        tnodes[j].x = step*j;
        tnodes[j].y = 0;
        tnodes[j].fixed = true;
        tnodes[j].edge = true;
        tnodes[j].qdot = 0;

        tnodes[(mesh_pts-1)*mesh_pts + j].T = rtem;
        tnodes[(mesh_pts-1)*mesh_pts + j].T_sum = 0;
        tnodes[(mesh_pts-1)*mesh_pts + j].x = step*j;
        tnodes[(mesh_pts-1)*mesh_pts + j].y = bar_size;
        tnodes[(mesh_pts-1)*mesh_pts + j].fixed = true;
        tnodes[(mesh_pts-1)*mesh_pts + j].edge = true;
        tnodes[(mesh_pts-1)*mesh_pts + j].qdot = 0;
    }
    
    for (int i = 1; i < mesh_pts-1; i++) {
        for (int j = 1; j < mesh_pts-1; j++) {
            tnodes[mesh_pts*i + j].T = T_default;
            tnodes[mesh_pts*i + j].T_sum = 0;
            tnodes[mesh_pts*i + j].x = (double) step*j;
            tnodes[mesh_pts*i + j].y = (double) step*i;
            tnodes[mesh_pts*i + j].fixed = false;
            tnodes[mesh_pts*i + j].edge = false;
            tnodes[mesh_pts*i + j].qdot = 0;
        }  

        tnodes[mesh_pts*i].T = ltem;
        tnodes[mesh_pts*i].T_sum = 0;
        tnodes[mesh_pts*i].x = (double) 0;
        tnodes[mesh_pts*i].y = (double) step*i;
        tnodes[mesh_pts*i].fixed = true;
        tnodes[mesh_pts*i].edge = true;

        tnodes[mesh_pts*i + (mesh_pts-1)].T = rtem;
        tnodes[mesh_pts*i + (mesh_pts-1)].T_sum = 0;
        tnodes[mesh_pts*i + (mesh_pts-1)].x = (double) 0;
        tnodes[mesh_pts*i + (mesh_pts-1)].y = (double) step*i;
        tnodes[mesh_pts*i + (mesh_pts-1)].fixed = true;
        tnodes[mesh_pts*i + (mesh_pts-1)].edge = true;
    }
    tnodes[(mesh_pts/2)*mesh_pts + mesh_pts/2].qdot = 1010*mesh_pts*mesh_pts;
    tnodes[(mesh_pts/2)*mesh_pts + mesh_pts/2 + 1].qdot = 1010*mesh_pts*mesh_pts;
    tnodes[(mesh_pts/2)*mesh_pts + mesh_pts/2 - 1].qdot = 1010*mesh_pts*mesh_pts;
    tnodes[(mesh_pts/2 + 1)*mesh_pts + mesh_pts/2].qdot = 1010*mesh_pts*mesh_pts;
    tnodes[(mesh_pts/2 - 1)*mesh_pts + mesh_pts/2].qdot = 1010*mesh_pts*mesh_pts;

    tnodes[(mesh_pts/2 + 2)*mesh_pts + mesh_pts/2].qdot = 1010*mesh_pts*mesh_pts;
    tnodes[(mesh_pts/2 - 2)*mesh_pts + mesh_pts/2].qdot = 1010*mesh_pts*mesh_pts;
    tnodes[(mesh_pts/2)*mesh_pts + mesh_pts/2 + 2].qdot = 1010*mesh_pts*mesh_pts;
    tnodes[(mesh_pts/2)*mesh_pts + mesh_pts/2 - 2].qdot = 1010*mesh_pts*mesh_pts;
    tnodes[(mesh_pts/2 + 1)*mesh_pts + mesh_pts/2 + 1].qdot = 1010*mesh_pts*mesh_pts;
    tnodes[(mesh_pts/2 - 1)*mesh_pts + mesh_pts/2 - 1].qdot = 1010*mesh_pts*mesh_pts;
    tnodes[(mesh_pts/2 + 1)*mesh_pts + mesh_pts/2 - 1].qdot = 1010*mesh_pts*mesh_pts;
    tnodes[(mesh_pts/2 - 1)*mesh_pts + mesh_pts/2 + 1].qdot = 1010*mesh_pts*mesh_pts;

}

//
//  interact two temperature nodes
//
void apply_tsum( node_t &tnode, node_t &neighbor)
{
    if (tnode.fixed) {
        return;
    }
    tnode.T_sum += neighbor.T;
}

//
//  Solve for the temperature
//
void tupdate( node_t &tnode, double div )
{
    if (tnode.fixed) {
        return;
    }
    tnode.T_sum += ( tnode.qdot * step * step)/k;
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
    for( int i = 0; i < n*n; i++ )
        fprintf( f, "%d,%g,%g,%g\n", step, tnode[i].x, tnode[i].y, tnode[i].T);
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
