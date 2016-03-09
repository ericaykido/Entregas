#include <stdio.h>
#include <stdlib.h>
#define tamanho 20
// void swap(int *p1, int *p2);
void print_array(int v[]);
int *primos(void);

int main()
{
    /* Ex 1
    int count = 10, *temp, sum =0;
    temp = &count; // endereço de temp armazenado em count
    *temp = 2; // valor armazenado no endereço para onde temp aponta
    temp = &sum; // endereço de temp armazenado em sum
    *temp = count;
    printf("%d, %d, %d", count, *temp, sum);
    */

    /* Ex 2
    int x = 10, y = 20;
    swap(&x,&y);
    printf("x: %d, y: %d", x, y);
    */

    /* Ex 3
    int arr[20];
    arr[2]=23;
    arr[3]=45;
    *(arr+0) = 34;
    printf("valor da posição 2: %d\nvalor da posição 3: %d\nvalor da posição 0: %d", arr[2], arr[3], *(arr+0));
    */

    // Ex 4
    int teste_array[20];

    teste_array[0]=2;
    teste_array[1]=3;
    teste_array[2]=5;
    teste_array[3]=7;
    teste_array[4]=11;
    teste_array[5]=13;
    teste_array[6]=17;
    teste_array[7]=19;
    teste_array[8]=23;
    teste_array[9]=29;
    teste_array[10]=31;
    teste_array[11]=45;
    teste_array[12]=41;
    teste_array[13]=43;
    teste_array[14]=47;
    teste_array[15]=53;
    teste_array[16]=59;
    teste_array[17]=61;
    teste_array[18]=67;
    teste_array[19]=71;
    print_array(&teste_array);
    //*/
    int v[3];
    v = primos();
    print_array(v);

    return 0;
}

/* Ex 2
void swap(int *p1, int *p2)
{
    int aux, aux2;
    aux = *p1;
    aux2 = *p2;
    *p2 = aux;
    *p1 = aux2;
}
*/
// Ex 4
void print_array(int v[tamanho])
{
    int i;

    for(i=0 ; i < tamanho ; i++)
        printf("%d: %d \n", i, v[i]);


}
//*/

// Ex 5
int *primos (void)
{
    int *v[3];

    v = (double*) malloc(2 * sizeof(double));

    v[0] = 1009;
    v[1] = 1013;
    v[2] = 1019;
    return v;
}

