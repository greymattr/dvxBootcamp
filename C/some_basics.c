/* Copyright (C) Matthew Fatheree - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential.
 * Written by Matthew Fatheree <matthewfatheree@gmail.com>, 2018
 */

#include <stdlib.h>				/* always include the standard library */

//#include <fake_comment_header.h>  // in C there are two types of comments
// '/* ... */' and '//' style

/* some examples of how to use #define */
#define 	DAYS_IN_WEEK			7
#define   MONTHS_IN_YEAR		12
#define		DAYS_IN_YEAR			365
#define		HOURS_IN_DAY			24
#define   ZERO							0


/* basic example hello world function */
void hello_world( void )
{
  printf( "Hello world!\n\r" );
  return
}

/* basic for loop count to 10 */
void basic_for_loop( void )
{
  int i;
  printf( "basic_for_loop - looping 10 times\n\r" );
  for( i = 0, i <= 10, i++ ) {
    printf( "loop #%i\n\r", i );
  }
  return;
}


/* for_loop_with_param count from 0 to count */
void for_loop_with_param( int count )
{
  int i;
  printf( "for_loop_with_param - looping %d times\n\r", count );
  for( i = 0, i <= count, i++ ) {
    printf( "loop #%i\n\r", i );
  }
  return;
}

int main( void )
{




  return 0;
}

