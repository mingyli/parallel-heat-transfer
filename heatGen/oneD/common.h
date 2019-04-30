#ifndef __CS267_COMMON_H__
#define __CS267_COMMON_H__

inline int min( int a, int b ) { return a < b ? a : b; }
inline int max( int a, int b ) { return a > b ? a : b; }

//
//  saving parameters
//
const int NSTEPS = 5000;
const int SAVEFREQ = 50;
const int k = 50;

//
// mesh node data structure
//
typedef struct 
{
  double T;
  double T_sum;
  double x;
  double qdot;
  int fixed;
} node_t;

//
//  timing routines
//
double read_timer( );

//
//  simulation routines
//
void set_len( int n );
void init_bar( node_t *tnodes, double bar_size, double ltem, double rtem );
void apply_tsum( node_t &tnode, node_t &neighbor );
void tupdate( node_t &tnode, int dim );


//
//  I/O routines
//
FILE *open_save( char *filename, int n );
void save( FILE *f, int step, int n, node_t *tnodes );

//
//  argument processing routines
//
int find_option( int argc, char **argv, const char *option );
int read_int( int argc, char **argv, const char *option, int default_value );
char *read_string( int argc, char **argv, const char *option, char *default_value );

#endif
