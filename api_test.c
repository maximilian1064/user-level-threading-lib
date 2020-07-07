#include <stdlib.h>   // exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <stdio.h>    // printf(), fprintf(), stdout, stderr, perror(), _IOLBF
#include <stdbool.h>  // true, false
#include <limits.h>   // INT_MAX

#include "threads.h"

/* Prints the sequence 0, 1, 2, .... INT_MAX over and over again.
 */
void numbers(void* args) {
  int n = 0;
  while (true) {
    printf(" n = %d\n", n);
    n = (n + 1) % (INT_MAX);
    if (n > 3) return;
    af_thread_yield();
  }
}

/* Dummy test func
 */
void numbers_one_shot(void* args) {
  int n = 0;
  while (true) {
    printf(" n = %d %d\n", n, n);
    n = (n + 1) % (INT_MAX);
    if (n > (int)args) return;
  }
}

/* Prints the sequence a, b, c, ..., z over and over again.
 */
void letters(void* args) {
  char c = 'a';

  while (true) {
      printf(" c = %c\n", c);
      if (c == 'l') return;
      af_thread_yield();
      c = (c == 'z') ? 'a' : c + 1;
    }
}

/* Calculates the nth Fibonacci number using recursion.
 */
int fib(int n) {
  switch (n) {
  case 0:
    return 0;
  case 1:
    return 1;
  default:
    return fib(n-1) + fib(n-2);
  }
}

/* Print the Fibonacci number sequence over and over again.

   https://en.wikipedia.org/wiki/Fibonacci_number

   This is deliberately an unnecessary slow and CPU intensive
   implementation where each number in the sequence is calculated recursively
   from scratch.
*/

void fibonacci_slow(void* args) {
  int n = 0;
  int f;
  while (true) {
    f = fib(n);
    if (f < 0) {
      // Restart on overflow.
      n = 0;
    }
    printf(" fib(%02d) = %d\n", n, fib(n));
    n = (n + 1) % INT_MAX;
  }
}

/* Print the Fibonacci number sequence over and over again.

   https://en.wikipedia.org/wiki/Fibonacci_number

   This implementation is much faster than fibonacci().
*/
void fibonacci_fast(void* args) {
  int a = 0;
  int b = 1;
  int n = 0;
  int next = a + b;

  while(true) {
    printf(" fib(%02d) = %d\n", n, a);
    next = a + b;
    a = b;
    b = next;
    n++;
    if (a < 0) {
      // Restart on overflow.
      a = 0;
      b = 1;
      n = 0;
    }
  }
}

/* Prints the sequence of magic constants over and over again.

   https://en.wikipedia.org/wiki/Magic_square
*/
void magic_numbers(void* args) {
  int n = 3;
  int m;
  while (true) {
    m = (n*(n*n+1)/2);
    if (m > 0) {
      printf(" magic(%d) = %d\n", n, m);
      n = (n+1) % INT_MAX;
    } else {
      // Start over when m overflows.
      n = 3;
    }
    af_thread_yield();
  }
}

int main(){
    puts("\n==== Test program for the Simple Threads API ====\n");

    af_thread_init(rr_scheduler); // Initialization
    tid_t thr1 = af_thread_create(numbers, NULL);
    printf("thread 1 id: %d\n", thr1);
    tid_t thr2 = af_thread_create(letters, NULL);
    printf("thread 2 id: %d\n", thr2);

    tid_t thr3 = af_thread_create(numbers_one_shot, 4);
    printf("thread 3 id: %d\n", thr3);
    af_thread_join(thr3);
    printf("thread %d joined\n", thr3);

    af_thread_join(thr1);
    printf("thread %d joined\n", thr1);
    af_thread_join(thr2);
    printf("thread %d joined\n", thr2);
}
