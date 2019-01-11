#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include "distribution.h"
#include "2ChoiceHashing.h"

__attribute__((optimize("no-tree-vectorize")))

int  main(int argc, char** argv){
	
	short DistributionType=0;
	int dataSize;
	int insertValue; 
	int initialSize = 50000;
	int totalSize = 500000;
	int iteration;
/*
Linear Linear Probing Evalution
*/
setM();
for(iteration=0;iteration<20;iteration++){
	printf("\nScalar 2Choice hashing\n");

	DistributionType=0;
	clearHash();
	clearClocks();

		while(DistributionType<6){
			switch(DistributionType){

				case 0 :	printf("\nDense Unique Random Values\n");
							break;
				case 1 :	printf("\nSequential Values\n");
							break;
				case 2 :	printf("\nUniform Random Values\n"); 
							break;
				case 3 :	printf("\nExponential Values\n"); 
							break;
				case 4 : 	printf("\nZipf law Values\n");
							break;
				case 5 : 	printf("\nSelf Similar law Values\n");
							break;
			}
			
			printf("Datasize\tunique\ttotal\tscalar probing\n");

			for(dataSize=initialSize;dataSize<totalSize;dataSize+=initialSize){		
				
				int i;
				setGen(dataSize); //Initialize generation Size
				
				switch(DistributionType){ //Populate with values

					case 0:	
							setGen(dataSize*3);
							InitDenseUnique();
							for(i=0;i<dataSize;i++){
								insertValue = DenseUniqueRandom();
								ScalarProbe(insertValue);
							}
							break;

					case 1: 
							for(i=0;i<dataSize;i++){
								insertValue = SequentialNumbers(5);
								ScalarProbe(insertValue);
							}
							break;

					case 2: 
							for(i=0;i<dataSize;i++){
								insertValue = UniformRandom();
								ScalarProbe(insertValue);
							}
							break;

					case 3:	
								for(i=0;i<dataSize;i++){
									insertValue = Exponential();
									ScalarProbe(insertValue);
								}
								break;
					case 4:	
								for(i=0;i<dataSize;i++){
									insertValue = zipf(1000,0.6);
									ScalarProbe(insertValue);
								}
								break;
					default:	
								for(i=0;i<dataSize;i++){
									insertValue = selfsimilar(25,0.1);
									ScalarProbe(insertValue);
								}
								break;

				}
				printf("%d\t%d\t%d\t%Lf\n",dataSize,hashCheck(),addValues(), getScalarTime());
				clearHash();
				clearClocks();
			}

			DistributionType++;

		}


	/*
	SIMD 2Choice hasing evaluation
	*/
	printf("\nSIMD 2Choice hashing\n");

	DistributionType=0;
	clearHash();
	clearClocks();

	while(DistributionType<6){
			switch(DistributionType){

				case 0 :	printf("\nDense Unique Random Values\n");
							break;
				case 1 :	printf("\nSequential Values\n");
							break;
				case 2 :	printf("\nUniform Random Values\n"); 
							break;
				case 3 :	printf("\nExponential Values\n"); 
							break;
				case 4 : 	printf("\nZipf law Values\n");
							break;
				case 5 : 	printf("\nSelf Similar law Values\n");
							break;
			}
			
			printf("Datasize\tunique\ttotal\tSIMD probing\n");

			for(dataSize=initialSize;dataSize<totalSize;dataSize+=initialSize){	
				
				int i;
				setGen(dataSize); //Initialize generation Size
				
				switch(DistributionType){ //Populate with values

					case 0:	
							setGen(dataSize);
							InitDenseUnique();
							for(i=0;i<dataSize;i++){
								insertValue = DenseUniqueRandom();
								VectorProbe(insertValue);
							}
							break;

					case 1: 
							for(i=0;i<dataSize;i++){
								insertValue = SequentialNumbers(5);
								VectorProbe(insertValue);
							}
							break;

					case 2: 
							for(i=0;i<dataSize;i++){
								insertValue = UniformRandom();
								VectorProbe(insertValue);
							}
							break;

					case 3:	
								for(i=0;i<dataSize;i++){
									insertValue = Exponential();
									VectorProbe(insertValue);
								}
								break;
					case 4:	
								for(i=0;i<dataSize;i++){
									insertValue = zipf(1000,0.6);
									VectorProbe(insertValue);
								}
								break;
					default:	
								for(i=0;i<dataSize;i++){
									insertValue = selfsimilar(25,0.1);
									VectorProbe(insertValue);
								}
								break;

				}
				printf("%d\t%d\t%d\t%Lf\n",dataSize,hashCheck(),addValues(), getSIMDTime());
				clearHash();
				clearClocks();
			}

			DistributionType++;

		}

	}

}