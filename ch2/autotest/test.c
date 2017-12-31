#include <stdio.h>
#include "include/func1.h"

int main(int argc, char* argv[])
{
	int result, num1, num2;
	result = 0;
	num1 = 1;
	num2 = 2;
	result = func1(num1, num2);
	printf("%d + %d = %d\n", num1, num2, result);
	
	return 0;
}