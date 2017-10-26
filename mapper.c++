#include <iostream>
#include <fstream>
#include <map>
#include <stdlib.h> //atoi

#include <stdio.h>
#include <stdlib.h>

using std::endl;
using std::cout;

using namespace std;

size_t number_of_candidates, number_of_jobs, nonzeros; //declaretions of numbers

int main(int argc, char *argv[]){

///////////////////////////////////////////////////           FILE         ////////////////////////////////////////////////////////////////

	int weights;  // weights // in future will be used

	std::ifstream file(argv[1]); // first command-line argument is taken as name of input //constructe calls the open file.
	file >> number_of_candidates >> number_of_jobs >> nonzeros; // first line contains number of cand,job and nonzeros.
	cout << "Number of candidates : " << number_of_candidates << "  Number of jobs : " <<  number_of_jobs << " Nonzeros :  " << nonzeros << endl;


	int *jobs = new int[nonzeros];
	int *candidates = new int[nonzeros];



	if(file.is_open()){ //read if file is openable

           for(int i = 0; i < nonzeros; i++) //read up to number nonzeros
              {
                file >> jobs[i] >> candidates[i] ; // first element is for candidates and second one is for jobs.
              }
  
        }

	cout << "data was read " << endl;

          

/////////////////////////////////////////////////////////////MAPPING///////////////////////////////////////////////////////////////
   int *jobs_mapped = new int[nonzeros];
   map<int , int> m;

   m[jobs[0]] = 1;
   int count = 2;

   for(int i = 1; i < nonzeros; i++){
	if(m[jobs[i]] == 0)
	   m[jobs[i]] = count++;
   }
	

   for(int i = 0; i < nonzeros ; i++){
	jobs_mapped[i] = m[jobs[i]];
   }
   int jobs_count = count - 1;
 ///////////////////////////////////////////////////
   int *candidates_mapped = new int[nonzeros];
   map<int , int> m2;

   m2[candidates[0]] = 1;
   count = 2;

   for(int i = 1; i < nonzeros ; i++){
	if(m2[candidates[i]] == 0)
	   m2[candidates[i]] = count++;
   }

   for(int i = 0; i < nonzeros ; i++){
	candidates_mapped[i] = m2[candidates[i]];
   }
     int candidates_count = count - 1;
/////////////////////////////////////////////////////////////MAPPING///////////////////////////////////////////////////////////////


	FILE *file2;

	if((file2=fopen("mapped.txt", "wb"))==NULL)
	{
	    printf("Something went wrong reading %s\n", "test.txt");
	    return 0;
	}
	else
	{        
		 fprintf(file2, "%d %d %zu\n",jobs_count,candidates_count,nonzeros);
 		 for(int i = 0; i < nonzeros; i++)
       	 	   fprintf(file2, "%d %d 1\n",jobs_mapped[i],candidates_mapped[i]);
		   //printf("%d %d \n",jobs[i],jobs_mapped[i]);
	}
	fclose(file2);

}
