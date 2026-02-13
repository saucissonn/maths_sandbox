#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main()
{
    int n;
    printf("Entrer n : ");
    scanf("%d", &n);

    int *premiers = malloc((n + 1) * sizeof(int));
    int count = 0;
    
    if (2 <= n) premiers[count++] = 2;

    for (int i = 3; i <= n; i+=2)
    {
        bool est_premier = true;

        for (int j = 0; j < count; j++)
        {
            int p = premiers[j];

            if (p * p > i)
                break;

            if (i % p == 0)
            {
                est_premier = false;
                break;
            }
        }

        if (est_premier)
        {
            premiers[count++] = i;
            //printf("%d ", i);
        }
    }
    count--;

    printf("len nb premiers %d\n", count);

    int c = 1;
 
    for (int i = 4; i <= n; i+=2) {
	int ok = 1;
	for (int j = 0; j < count; j++) {
	    for (int k = c; 0 < k; k--) {
	    	int res = premiers[j] + premiers[k];
		if (i > res) break;
		if (i == res) {
		    ok = 0;
		    printf("%d + %d = %d\n", premiers[j], premiers[k], i);
		    break;
		}
		if (ok == 0) break;
	    }
	    if (premiers[c+1] > i) c++;
	    if (ok == 0) break;
	}
	
    }
    printf("ok\n");
    free(premiers);
    return 0;
}


