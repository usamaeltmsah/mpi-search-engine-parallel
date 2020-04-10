#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mpi.h"
#include <time.h>

typedef struct{
    int index;
    char text[250];
}    existsWhere;

existsWhere stringsExists(char* str1, char* str2, int ind)
{
    char str1cpy[250] = "";
    strcpy(str1cpy, str1);
    if (islower(str2[0]))
        str1cpy[0] = tolower(str1cpy[0]);  // Convert the first letter to lower case

    char stringToArr2[30][15]; //can store 30 words each of 15 characters
    int i, j, cnt1 = 0, cnt2 = 0;
    existsWhere structexistsWhere;
    for(i = 0/*, j = 0*/; i <= (strlen(str1cpy)); i++)
    {
        // if space or NULL found, assign NULL into splitStrings[cnt]
        if(str1cpy[i] == ' '||str1cpy[i] == '\0'||str1cpy[i] == ','||str1cpy[i] == '.')
            cnt1++; //for next word
    }
    for(i = 0, j = 0; i <= (strlen(str2)); i++)
    {
        // if space or NULL found, assign NULL into splitStrings[cnt]
        if(str2[i] == ' '||str2[i] == '\0'||str2[i] == ','||str2[i] == '.'||str2[i] == '\n')
        {
            stringToArr2[cnt2][j] = '\0';
            cnt2++; //for next word
            j = 0; //for next word, init index to 0
        }
        else
        {
            stringToArr2[cnt2][j] = str2[i];
            j++;
        }
    }

    for (i = 0; i < cnt2; ++i) {
        for (j = 0; j < cnt1; ++j) {
            if (strstr(str1cpy, stringToArr2[i]) == NULL) // Check substrings
            {
                structexistsWhere.index = -1;
                return structexistsWhere;
            }
        }
    }
    structexistsWhere.index = ind;
    strcpy(structexistsWhere.text, str1);
    return structexistsWhere;
}

char* concatPath(char* path, int fileInd)
{
    char strNum[2];
    char file[50] = "Aristo-Mini-Corpus P-";
    strcat(path, file);
    sprintf(strNum, "%d", fileInd); // Change int to string
    strcat(path, strNum);
    strcat(path, ".txt");
    return path;
}

int main(int argc, char **argv) {
    // clock_t begin = clock();
    double start = MPI_Wtime(); /*start timer*/

    FILE *fp;
    existsWhere structexistsWhere[50][30];
    int c = 0;
    char* search_q = "sunlight energy nutrients"; // Search Query
    int rank, size, i, off;
    char* matched_queries[500];
    const int N_FILES = 50;
    MPI_Datatype type;
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    MPI_Type_contiguous(100, MPI_CHAR, &type);
    MPI_Type_commit(&type);

    MPI_Status status;
    int rc;
    MPI_File fh; // MPI file pointer
    int p = N_FILES / size; // Each process files number.
    int rem = N_FILES % size; // Remainder
    int isend[N_FILES], irecv[p];
    if(rank == 0){
        printf("\nSearch Query:\n %s\n\n", search_q);
        for (i = 1; i <= N_FILES; i++)
        {
            isend[i-1] = i;
        }
    }
    int x;
    MPI_Scatter(&isend, p, MPI_INT, &irecv, p, MPI_INT, 0, MPI_COMM_WORLD);
    for (i = 0; i < p; i++)
    {
        char path[] = "Aristo-Mini-Corpus/";
        
        concatPath(path, irecv[i]);
        fp = fopen(path, "r");
        //printf("%s\n", path);

        for (int m = 0; m < 30; ++m) {
            char buff[255];
            fgets(buff, 255, (FILE*)fp);
            existsWhere isEx = stringsExists(buff, search_q, m);
            if (isEx.index != -1)
            {
                //structexistsWhere[x-1][m].index = isEx.index;
                
                printf("I am proc #%d I found:  %s", rank, isEx.text);
                c++;
            }
            //else
                //structexistsWhere[x-1][m].index = -1;
        }
        fclose(fp);
    }

    // Handle the remainder
    if(rank == 0)
    {
        //MPI_Gather (&localMax , 1 , MPI_INT, localMaxima,1,MPI_INT,0,MPI_COMM_WORLD);
        while (rem > 0)
        {
            int x;
            char path[] = "Aristo-Mini-Corpus/";
            concatPath(path, N_FILES-rem+1);
            fp = fopen(path, "r");
            for (int m = 0; m < 30; ++m) {
                char buff[255];
                fgets(buff, 255, (FILE*)fp);
                existsWhere isEx = stringsExists(buff, search_q, m);
                if (isEx.index != -1)
                {
                    //structexistsWhere[x-1][m].index = isEx.index;

                    //MPI_File_open(MPI_COMM_WORLD, path, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
                    //MPI_File_read_at(fh, 0, buff, 255, MPI_CHAR, &status);

                    printf("I am proc #%d I found:  %s", rank, isEx.text);
                    c++;
                }
                //else
                    //structexistsWhere[x-1][m].index = -1;
            }
            fclose(fp);
            
            rem--;
        }
        double finish=MPI_Wtime(); /*stop timer*/
        printf("Parallel Elapsed time: %f seconds\n", finish-start); 
    }

    MPI_Finalize();        

    return 0;
}
