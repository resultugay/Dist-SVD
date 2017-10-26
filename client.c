#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>     //socket headers
#include <netdb.h>      //socket headers
#include <netinet/in.h> //socket headers

#include "mkl.h"

#define BUFFER_SIZE 8

struct thread_data{
   int thread_id;
   int number_of_rows;
   int number_of_columns;
   double *data;
};

typedef struct thread_data thread_data;

void print_matrix( char* desc, int m, int n, double* a, int lda );

int main(int argc, char *argv[]) {

   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;

   if (argc < 1) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   }

   portno = 5000;//atoi(argv[2]);
   

   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
	
   server = gethostbyname("127.0.0.1"/*argv[1]*/);
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }

   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   

   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }

   thread_data data;
   n = read(sockfd,&data.thread_id,sizeof(int)); //Thread_id is taken

   if (n < 0) {
      perror("ERROR writing to socket(thread_id)");
      exit(1);
   }

   n = read(sockfd,&data.number_of_rows,sizeof(int)); //number_of_rows is taken

   if (n < 0) {
      perror("ERROR writing to socket(number_of_rows)");
      exit(1);
   }

   n = read(sockfd,&data.number_of_columns,sizeof(int)); //number_of_columns is taken

   if (n < 0) {
      perror("ERROR writing to socket(number_of_columns)");
      exit(1);
   }
   //getchar();

   printf("My thread id is %d \n",data.thread_id);
   printf("My row number is %d \n",data.number_of_rows);
   printf("My column number is %d \n",data.number_of_columns);

   int number_of_rows = data.number_of_rows;
   int number_of_columns = data.number_of_columns;

   data.data = malloc(number_of_rows * number_of_columns * sizeof *data.data);;
   int i = 0;
   for(i = 0 ; i < data.number_of_rows*data.number_of_columns ; i = i + (BUFFER_SIZE/sizeof(double))){
      n = read(sockfd,&data.data[i],BUFFER_SIZE); // number_of_columns is sent
      if (n < 0) {
        perror("ERROR writing to socket(number_of_columns)");
        exit(1);
      }

   }
  
  int  j = 0;

  for(i = 0 ; i <  data.number_of_rows ; i++){
      for(j = 0 ; j <  data.number_of_columns ; j++){
          printf("%lf ", data.data[i*data.number_of_columns + j]);
      }
          printf("\n");
   }


   n = write(sockfd,&data.thread_id,sizeof(int)); // thread_id is sent

   int M = data.number_of_columns;
   int N = data.number_of_rows;
   int LDA = M;
   int LDU = M;
   int LDVT = N;
   int m = M, lda = LDA, ldu = LDU, ldvt = LDVT, info, lwork;
   n = N;
   double wkopt;
   double* work;
        /* Local arrays */
  double * s;

  /* left singular vectors */
  double * u;

  /* right singular vectors */
  double * vt;


      s = (double *)malloc(sizeof(double)*m);
      for (int i = 0; i < m; i++) {
	s[i] = 0.0;
      }
      
      u = (double *)malloc(sizeof(double)*m*m);
      for (int i = 0; i < m*m; i++) {
	u[i] = 0.0;
      }

      
      vt = (double *)malloc(sizeof(double)*n*n);
      for (int i = 0; i < n*n; i++) {
	vt[i] = 0.0;
      }
      

        lwork = -1;
        dgesvd( "All", "All", &m, &n, data.data, &lda, s, u, &ldu, vt, &ldvt, &wkopt, &lwork,
         &info );
        lwork = (int)wkopt;
        printf("lwork %d\n",lwork);
        work = (double*)malloc( lwork*sizeof(double) );

	      for (int i =0; i<lwork; i++) {
		work[i] = 0.0;
	      }

        /* Compute SVD */
        dgesvd( "All", "All", &m, &n, data.data, &lda, s, u, &ldu, vt, &ldvt, work, &lwork,
         &info );
        /* Check for convergence */
        if( info > 0 ) {
                printf( "The algorithm computing SVD failed to converge.\n" );
                exit( 1 );
        }



        /* Print singular values */
        print_matrix( "Singular values", 1, n, s, 1 );
        /* Print left singular vectors */
        print_matrix( "Left singular vectors (stored columnwise)", m, n, u, ldu );
        /* Print right singular vectors */
        print_matrix( "Right singular vectors (stored rowwise)", n, n, vt, ldvt );
        /* Free workspace */
        free( (void*)work );
      free(s);
      free(u);
      free(vt);


}

/* Auxiliary routine: printing a matrix */
void print_matrix( char* desc, int m, int n, double* a, int lda ) {
        int i, j;
        printf( "\n %s\n", desc );
        for( i = 0; i < m; i++ ) {
                for( j = 0; j < n; j++ ) printf( " %6.2f", a[i+j*lda] );
                printf( "\n" );
        }
}



