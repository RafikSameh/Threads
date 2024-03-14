#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

int threadnum=0;
typedef struct array
{
    int RB;
    int CB;
    int row;    
    int **A;
    int **B;
    int **res;
}array_t;
void parse_input(int *rowA,int *colA,int *rowB,int *colB,char *fileA,char *fileB)
{
    //char *file="test1/a.txt";
    FILE *fp=fopen(fileA,"r");
    char *buffer;
    int n=100;
    getline(&buffer,&n,fp);
    char *temp;
    char *token=strtok(buffer,"row=");
    *rowA=(int)strtol(token,&temp,10);
    token=strtok(NULL,"col=");
    *colA=(int)strtol(token,&temp,10);
    fclose(fp);free(buffer);
    //file ="test1/b.txt";
    fp=fopen(fileB,"r");
    getline(&buffer,&n,fp);
    token=strtok(buffer,"row=");
    *rowB=(int)strtol(token,&temp,10);
    token=strtok(NULL,"col=");
    *colB=(int)strtol(token,&temp,10);
    free(buffer);
}
void arr_assign(int row,int col,int **arr,char *file)
{
    FILE *fp=fopen(file,"r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return ;}
    char *buffer=(char*)malloc(sizeof(char)*100);
    int n=100;
    fgets(buffer,100,fp);    
    printf("line is %s\n",buffer);
    char *temp;
    for(int i=0;i<row ;i++)
    {
        fgets(buffer,100,fp);
        char *token=strtok(buffer,"\t");
        for(int j=0;j<col;j++)
        { 
            arr[i][j]=(int)strtol(token,&temp,10);
            token=strtok(NULL,"\t");
        }
    }
    free(buffer);
    fclose(fp);
}
int** seq_mult(int rowA,int colA,int colB,int **arrA,int **arrB)
{
    int **result = (int **)malloc(rowA * sizeof(int *));
    for (int i = 0; i < rowA; i++) {
        result[i] = (int *)malloc(colB * sizeof(int));
    }
    for(int i=0;i<rowA;i++)
    {
        for(int j=0;j<colB;j++)
        {
            result[i][j]=0;
            for(int k=0;k<colA;k++)
            {
                result[i][j]=result[i][j]+(arrA[i][k]*arrB[k][j]);
            }
        }
    }
    return result;
}
void writepermatrix(int **res,int rows,int cols,unsigned long sec,unsigned long ms)
{
    char *file="c_per_matrix.txt";
    FILE *fp=fopen(file,"w");
    fprintf(fp,"Method: thread per matrix\nrows=%d    cols=%d\n",rows,cols);
    for(int i=0;i<rows;i++)
    {
        for(int j=0;j<cols;j++)
        {
            fprintf(fp,"%d\t",res[i][j]);
        }
        fprintf(fp,"\n");
    }
    fprintf(fp,"\n");
    fprintf(fp,"Seconds taken %lu\n", sec);
    fprintf(fp,"Microseconds taken: %lu\n", ms);
    fclose(fp);
}
void writeperrow(int **res,int rows,int cols,unsigned long sec,unsigned long ms)
{
    char *file="c_per_row.txt";
    FILE *fp=fopen(file,"w");
    fprintf(fp,"Method: thread per row\nrows=%d    cols=%d\n",rows,cols);
    for(int i=0;i<rows;i++)
    {
        for(int j=0;j<cols;j++)
        {
            fprintf(fp,"%d\t",res[i][j]);
        }
        fprintf(fp,"\n");
    }
    fprintf(fp,"\n");
    fprintf(fp,"Seconds taken %lu\n", sec);
    fprintf(fp,"Microseconds taken: %lu\n", ms);
    fclose(fp);
}
void *ThreadperRow(void* ptr)
{
    array_t *arrays=(array_t *)ptr;
    int *result = (int *)malloc(arrays->CB * sizeof(int)); // Allocate for one row's worth of results

    for (int k = 0; k < arrays->CB; k++) {
            for (int i = 0; i < arrays->RB; i++) {
                result[k] += arrays->A[arrays->row][i] * arrays->B[i][k]; // Use arrays->A[k] for the specific row
            }
    }
    arrays->res[arrays->row]=result;
    pthread_exit(NULL);
    return NULL;
}
int main()
{
    struct timeval stop, start;
    int rowA,colA,rowB,colB;
    
    parse_input(&rowA,&colA,&rowB,&colB,"test2/a.txt","test2/b.txt");
    int **arrA = (int **)malloc(rowA * sizeof(int *));
    for (int i = 0; i < rowA; i++) {
        arrA[i] = (int *)malloc(colA * sizeof(int));
    }

    int **arrB = (int **)malloc(rowB * sizeof(int *));
    for (int i = 0; i < rowB; i++) {
        arrB[i] = (int *)malloc(colB * sizeof(int));
    }
    printf("rowA= %d colA=%d\n",rowA,colA);
    printf("rowB= %d colB=%d\n",rowB,colB);
    arr_assign(rowA,colA,arrA,"test2/a.txt");
    arr_assign(rowB,colB,arrB,"test2/b.txt");


    gettimeofday(&start, NULL); //start checking time
    //your code goes here
    int** ResPerMatrix=seq_mult(rowA,colA,colB,arrA,arrB);

    gettimeofday(&stop, NULL); //end checking time
    printf("Time taken by thread per matrix\n");
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    if(!ResPerMatrix)
    {
        perror("error respermatr");
    }
    writepermatrix(ResPerMatrix,rowA,colB,stop.tv_sec - start.tv_sec,stop.tv_usec - start.tv_usec);
    printf("\nthread per row\n\n");
    pthread_t thread[rowA];
    int **result = (int **)malloc(rowA * sizeof(int *));
    for (int i = 0; i < rowA; i++)
    {
        result[i] = (int *)malloc(colB * sizeof(int));
    }
    //void *thread_result;
    gettimeofday(&start, NULL); //start checking time
    //your code goes here
    for(int i=0;i<rowA;i++)
    {
        array_t *arr=(array_t*)malloc(sizeof(array_t));
        arr->B=arrB;
        arr->A=arrA;
        arr->RB=rowB;
        arr->CB=colB;
        arr->row=i;
        arr->res=result;
        pthread_create(&thread[i],NULL,ThreadperRow,(void *)arr);
    }
    
    for(int j=0;j<rowA;j++)
    {
        pthread_join(thread[j],NULL);
    }
    gettimeofday(&stop, NULL); //end checking time
    printf("Time taken by thread per row\n");
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    
    writeperrow(result,rowA,colB,stop.tv_sec - start.tv_sec,stop.tv_usec - start.tv_usec);
    // Free dynamically allocated memory
    for(int i = 0; i < rowA; i++) {
        free(result[i]);
    }
    free(result);
}