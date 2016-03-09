#include <stdio.h>
#include <stdlib.h>
void mdc(int num1, int num2);

int main()
{
    int num1, num2;

    printf("Numero 1: \n");
    scanf("%d", &num1);

    printf("Numero 2: \n");
    scanf("%d", &num2);

    mdc(num1,num2);

    return 0;
}

void mdc(int num1, int num2)
{
    int i, hcf;
    for(i=1; i<=num1 || i<=num2; ++i)
    {
        if(num1%i==0 && num2%i==0)
            hcf=i;
    }
    printf("H.C.F. of %d and %d is %d \n", num1, num2, hcf);
}
