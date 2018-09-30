/* 
 * The MIT License (MIT)
 * 
 * Copyright (c) 2018 Belkin/Linksys 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>				/* always include the standard library */

//#include <fake_comment_header.h>  // in C there are two types of comments
// '/* ... */' and '//' style

/* some examples of how to use #define */
#define   DAYS_IN_WEEK      7
#define   MONTHS_IN_YEAR    12
#define   DAYS_IN_YEAR      365
#define   HOURS_IN_DAY      24
#define   ZERO              0


/* basic example hello world function */
void hello_world( void )
{
  printf( "Hello world!\n\r" );
  return;
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

