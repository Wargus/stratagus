#ifdef USE_BEOS
#include <Path.h>
#include <unistd.h>
extern "C" {

void beos_init( int argc, char **argv )
{ 
  BPath path( argv[0] );
  path.GetParent( &path );
  chdir( path.Path() );
}

}
#endif
