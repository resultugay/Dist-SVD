#include <stdio.h>
#include <stdlib.h>
#include <strings.h> //bzero function

#include <unistd.h>    //socket headers
#include <netdb.h>     //socket headers
#include <netinet/in.h>//socket headers

#include <pthread.h>   //for thread functions

#define BUFFER_SIZE 8

struct thread_data{
   int thread_id;
   int number_of_rows;
   int number_of_columns;
   double *data;
   int socket_number;
};

typedef struct thread_data thread_data;

void print_matrix(int row,int col,double **arry);
void *send_and_receive_tf(void *thread_arg);
int main(int argc, char *argv[]){

///////////////////////////////////////////////////////////////  FILE_READ  ///////////////////////////////////////////////////////////////

   //file pointer, gets data from the file given by first command line argument
   FILE *fp;
   fp = fopen(argv[1], "r");
   int total_row_number,total_column_number,i,j,nonzeros;
   //First line indicates number of rows and columns and nonzeros
   fscanf(fp,"%d",&total_row_number);
   printf("Number of rows : %d\n", total_row_number);
   fscanf(fp,"%d",&total_column_number);
   printf("Number of columns : %d\n", total_column_number);
   fscanf(fp,"%d\n",&nonzeros);
   printf("Number of nonzeros : %d\n", nonzeros);

   double ** full_matrix = (double **)malloc(total_row_number * sizeof(double *));
   for (i = 0; i < total_row_number; i++)
       full_matrix[i] = (double *)malloc(total_column_number * sizeof(double));

   for(i = 0; i < total_row_number ; i++)
      for(j = 0; j < total_column_number ; j++)
        full_matrix[i][j] = 0.0;

   //variables contain temporary values from the file when reading 
   int t1,t2;
   double t3;

   //Totally nonzeros is read from the file each one is seperated by a space and each enry by a line
   for(int i = 0 ; i < nonzeros * 3 ; i++){
      fscanf(fp,"%d",&t1);
      fscanf(fp,"%d",&t2);
      fscanf(fp,"%lf\n",&t3);  
      full_matrix[t1-1][t2-1] = t3;
   }
   //print_matrix(total_row_number,total_column_number,full_matrix); 

///////////////////////////////////////////////////////////////  FILE_READ  ///////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////  SOCKET  //////////////////////////////////////////////////////////////////
   int number_of_connections = atoi(argv[2]);
   socklen_t clilen;
   int sockfd, newsockfd, portno;
   struct sockaddr_in serv_addr, cli_addr;
   int  n;

	
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) //for reuse port again without waiting more
      perror("setsockopt(SO_REUSEADDR) failed");
	
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
    }

   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = 5000; // port number ,can be changed later on.
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno); 
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
     perror("ERROR on binding");
     exit(1);
   }
   listen(sockfd,number_of_connections);
   clilen = sizeof(cli_addr);

///////////////////////////////////////////////////////////////  SOCKET  //////////////////////////////////////////////////////////////////
   pthread_t threads[number_of_connections];
   int rc ;
   pthread_attr_t attr;
   pthread_attr_init(&attr);
   //Number of columns to be sent is computed here
   int columns_to_sent [number_of_connections] ;
   int size = total_column_number / number_of_connections;
   for(int i = 0 ; i < number_of_connections - 1 ;i++){
      columns_to_sent[i] = size;		
   }
   
   int tmp = total_column_number - (total_column_number / number_of_connections) * (number_of_connections - 1);
   columns_to_sent[number_of_connections - 1] = tmp ;

   int tmp_connections = 0;
   thread_data t[number_of_connections];

   for(i = 0 ; i < number_of_connections; i++)
       t[i].data = malloc(total_row_number*columns_to_sent[i] * sizeof *t[i].data);;


   //2D array is mapped to 1D array
   int c = 0;
   int km = 0;
   for(c = 0 ; c < number_of_connections ; c++){
    
     if(c == 0)
       km = 0;
     else
       km += columns_to_sent[c-1];
   
     for(j = 0 ; j < columns_to_sent[c] ; j++) 
        for(i = 0 ; i < total_row_number ; i++)
           t[c].data[i*columns_to_sent[c] + j] = full_matrix[i][km+j];  
  
     //printf("from %d to %d \n", km, km+columns_to_sent[c]);
   }
   
   //clients are being waited
   while(tmp_connections < number_of_connections){
      newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
      t[tmp_connections].thread_id = tmp_connections;
      t[tmp_connections].number_of_rows = total_row_number;	
      t[tmp_connections].number_of_columns = columns_to_sent[tmp_connections];
      t[tmp_connections].socket_number = newsockfd;
      rc = pthread_create(&threads[tmp_connections], &attr, send_and_receive_tf, (void *) &t[tmp_connections]);
      tmp_connections = tmp_connections + 1;
      if (newsockfd < 0) {
        perror("ERROR on accept");
        exit(1);
      }
    }

   //thread joining	
   void *status;
   for(i = 0; i < number_of_connections ; i++) {
       rc = pthread_join(threads[i], &status);
       if (rc) {
          printf("ERROR; return code from pthread_join() is %d\n", rc);
          exit(-1);
          }
	//printf("Main: completed join with thread %d having a status %ld\n",i,(long)status);
       }

   pthread_exit(NULL);
   //memory free
   free(full_matrix);


}

void *send_and_receive_tf(void *thread_arg){
   thread_data *arg;
   arg = (thread_data *) thread_arg;
   int thread_id = arg->thread_id;  
   int rows = arg->number_of_rows;
   int cols = arg->number_of_columns;

   int i,j = 0;

  /* for(i = 0 ; i < rows ; i++){
      for(j = 0 ; j < cols ; j++){
          printf("%lf ", arg->data[i*cols + j]);
      }
          printf("\n");
   }*/

   int socket_number = arg->socket_number;
   int n = write(socket_number,&arg->thread_id,sizeof(int)); // thread_id is sent
   if (n < 0) {
      perror("ERROR writing to socket(thread_id)");
      exit(1);
   }
   n = write(socket_number,&arg->number_of_rows,sizeof(int)); // number_of_rows is sent
   if (n < 0) {
      perror("ERROR writing to socket(number_of_rows)");
      exit(1);
   }
   n = write(socket_number,&arg->number_of_columns,sizeof(int)); // number_of_columns is sent
   if (n < 0) {
      perror("ERROR writing to socket(number_of_columns)");
      exit(1);
   }


   for(i = 0 ; i < rows*cols ; i = i + (BUFFER_SIZE/sizeof(double))){
      n = write(socket_number,&arg->data[i],BUFFER_SIZE); 
      if (n < 0) {
        perror("ERROR writing to socket(number_of_columns)");
        exit(1);
      }

   }
  
   int a;
   n = read(socket_number,&a,sizeof(int)); //thread_id is taken
   if (n < 0) {
      perror("ERROR writing to socket(number_of_columns)");
      exit(1);
   }


}



void print_matrix(int row,int col,double **arry){
   printf("Matrix is : \n");
   int i,j;
   for(i = 0; i < row ; i++){
      for(j = 0; j < col ; j++){
        printf("%6.2f ", arry[i][j]);
      }
	printf("\n");
   }
}
