// Reference: https://cs-fundamentals.com/tech-interview/c/c-program-to-check-little-and-big-endian-architecture

#include <stdio.h>

void main ()
{
  unsigned int x = 0x87654321;
  char *c = (char*) &x;
 
  printf ("*c is: 0x%x\n", *c);
  if (*c == 0x21)
  {
    printf ("Underlying architecture is little endian. \n");
  }
  else
  {
     printf ("Underlying architecture is big endian. \n");
  }
 
  return;
}
