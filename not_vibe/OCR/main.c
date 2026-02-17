#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE *fp;
    char ch;
    fp = fopen("dog2.png", "rb+");

    if(fp == NULL)
    {
        printf("Error in opening the image");
        fclose(fp);
        exit(0);
    }

    printf("Successfully opened the image file");

    while((ch = fgetc(fp)) != EOF)
    {
        printf("%c", ch);
    }

    fclose(fp);
    printf("\nWriting to o/p completed");
}
