#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

int threadnum=0;
typedef struct array
{
    int CA;
    int row;
    int col;
    int **A;
    int **B;
    int **res;
}array_t;
void parse_input(int *rowA,int *colA,int *rowB,int *colB,char *fileA,char *fileB)
{
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

void WriteperCell(int **res,int rows,int cols,unsigned long sec,unsigned long ms)
{
    char *file="c_per_cell.txt";
    FILE *fp=fopen(file,"w");
    fprintf(fp,"Method: thread per cell\nrows=%d    cols=%d\n",rows,cols);
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
void *ThreadperCell(void* ptr)
{
    array_t *arrays=(array_t *)ptr;

    int sum=0;
    for (int k = 0; k < arrays->CA; k++) {
            sum += arrays->A[arrays->row][k] * arrays->B[k][arrays->col]; // Use arrays->A[k] for the specific row  
    }
    arrays->res[arrays->row][arrays->col]=sum;
    //printf("%d\t",*result);
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
    arr_assign(rowA,colA,arrA,"test2/a.txt");
    arr_assign(rowB,colB,arrB,"test2/b.txt");
    printf("rowA= %d colA=%d\n",rowA,colA);
    printf("rowB= %d colB=%d\n",rowB,colB);

    printf("\nthread per cell\n\n");

    if(colA != rowB)
    {
        perror("Error in matrix multiplication");
        abort();
    }
    //pthread_t thread[rowA][colB];
    pthread_t **thread = (pthread_t **)malloc(rowA * sizeof(pthread_t *));
    for (int i = 0; i < rowA; i++)
    {
        thread[i] = (pthread_t *)malloc(colB * sizeof(pthread_t));
    }
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
        for(int j=0;j<colB;j++)
        {
            array_t *arr=(array_t*)malloc(sizeof(array_t));
            arr->row=i;
            arr->col=j;
            arr->res=result;
            arr->A=arrA;    
            arr->B=arrB;
            arr->CA=colA;
            pthread_create(&thread[i][j],NULL,ThreadperCell,(void *)arr);
        }   
    }
    
    for(int k=0;k<rowA;k++)
    {
        for(int l=0;l<colB;l++)
        {
            pthread_join(thread[k][l],NULL);
        }      
    }
    gettimeofday(&stop, NULL); //end checking time
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);

    
    WriteperCell(result,rowA,colB,stop.tv_sec - start.tv_sec,stop.tv_usec - start.tv_usec);
    for(int i=0;i<rowA;i++)
    {
        for(int k=0;k<colB;k++)
        {
            printf("%d\t",result[i][k]);
            //printf("%d\t",arr->res[i][k]);
        }
        printf("\n");
    }
    // Free dynamically allocated memory
    for(int i = 0; i < rowA; i++) {
        free(result[i]);
    }
    free(result);
}