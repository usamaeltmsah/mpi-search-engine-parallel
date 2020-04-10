#include <stdio.h>
#include<stdlib.h>
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
    //if (islower(str2[0]))
      //  str1cpy[0] = tolower(str1cpy[0]);  // Convert the first letter to lower case

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

    FILE *fp, *write_file;
    existsWhere structexistsWhere[50][30];
    int counter, c = 0;
    char* search_q = "plants animals"; // Search Query
    //search_q = malloc(100*sizeof(char));
    //scanf("%[^\n]%*c", search_q);
    int rank, size, i, off;
    char my_queries[100][250]; // Each query have 250 char
    char matched_queries[500][250]; // Each query have 250 char
    const int N_FILES = 50;
    //MPI_Datatype type;
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    //MPI_Type_contiguous(100, MPI_CHAR, &type);
    //MPI_Type_commit(&type);

    MPI_Status status;
    int rc;
    MPI_File fh; // MPI file pointer
    int p = N_FILES / size; // Each process files number.
    int rem = N_FILES % size; // Remainder
    int isend[N_FILES], irecv[p];
    if(rank == 0){
        write_file = fopen("matched_queries.txt","a");
        printf("\nSearch Query:\n %s\n\n", search_q);
        fprintf(write_file, "Query: %s\n\n", search_q);
        fclose(write_file);
        for (i = 1; i <= N_FILES; i++)
        {
            isend[i-1] = i;
        }
    }
    MPI_Scatter(&isend, p, MPI_INT, &irecv, p, MPI_INT, 0, MPI_COMM_WORLD);
    int x = 0;
    // matched_queries[rank][0] = (char) rank;
    // printf("%s\n", matched_queries[rank]);
    for (i = 0; i < p; i++)
    {
        char path[] = "Aristo-Mini-Corpus/";
        
        concatPath(path, irecv[i]);
        fp = fopen(path, "r");
        write_file = fopen("matched_queries.txt","a");
        //printf("%s\n", path);

        for (int m = 0; m < 30; ++m) {
            char buff[255];
            fgets(buff, 255, (FILE*)fp);
            existsWhere isEx = stringsExists(buff, search_q, m);
            if (isEx.index != -1)
            {
                //structexistsWhere[x-1][m].index = isEx.index;
                strcpy(my_queries[x], isEx.text);
                fprintf(write_file, "%s", isEx.text);
                //printf("I am proc #%d I found:  %s", rank, my_queries[x]);                
                c++;
            }
            //else
                //structexistsWhere[x-1][m].index = -1;
        }
        fclose(fp);
        fclose(write_file);
    }
    MPI_Reduce(&c, &counter, 1,MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    //printf("%s\n", my_queries[10]);
    //printf("%ld\n", sizeof(my_queries)/sizeof(my_queries[0]));
    /*for(int i = 0; i < sizeof(my_queries)/100; i++)
    {
        printf("I am proc #%d I found:  %s", rank, my_queries[x]);
    }*/
    /*MPI_Gather (&my_queries, x+1 , type, matched_queries, x+1, MPI_INT,0,MPI_COMM_WORLD);
    for(int i = 0; i < 2; i++){
        printf("%s\n", matched_queries[0]);
    }*/
    c = 0;
    // Handle the remainder
    if(rank == 0)
    {
        if (rem == 0){
            printf("Number of matched queries: %d\n", counter);
            fprintf(write_file, "\n\nMatched queries: %d", counter);
            fclose(write_file);
            double finish=MPI_Wtime(); /*stop timer*/
            printf("Parallel Elapsed time: %f seconds\n", finish-start); 
            MPI_Finalize();
            return 0;
        }
        if(rem > 0){
            
            for(int x = p*size; x <= (p*size)+rem; x++)
            //while (rem > 0)
            {
                char path[] = "Aristo-Mini-Corpus/";
                concatPath(path, isend[x]);
                fp = fopen(path, "r");
                
                write_file = fopen("matched_queries.txt","a");
                for (int m = 0; m < 30; ++m) {
                    char buff[255];
                    fgets(buff, 255, (FILE*)fp);
                    existsWhere isEx = stringsExists(buff, search_q, m);
                    if (isEx.index != -1)
                    {
                        //structexistsWhere[x-1][m].index = isEx.index;

                        //MPI_File_open(MPI_COMM_WORLD, path, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
                        //MPI_File_read_at(fh, 0, buff, 255, MPI_CHAR, &status);
                        fprintf(write_file, "%s", isEx.text);
                        // printf("I am proc #%d I found:  %s", rank, isEx.text);
                        c++;
                    }
                    //else
                        //structexistsWhere[x-1][m].index = -1;
                }
                fclose(fp);
                rem--;
            }
            //MPI_Reduce(&c, &counter, 1,MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
            counter += c;
            printf("Number of matched queries: %d\n", counter);
            fprintf(write_file, "\n\nMatched queries: %d", counter);
            fclose(write_file);
            double finish=MPI_Wtime(); /*stop timer*/
            printf("Parallel Elapsed time: %f seconds\n", finish-start); 
        }
    }
 
    MPI_Finalize();        

    return 0;
}
