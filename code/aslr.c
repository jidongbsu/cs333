#include <stdio.h>
#include <stdlib.h>

void main()
{
	char x[12];
	char *y = malloc(sizeof(char)*12);

	printf("Address of buffer x (on stack): 0x%x\n", x);
	printf("Address of buffer y (on heap): 0x%x\n", y);
}
