#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include "distribution.h"
#include "LinearProbingAOS.h"

__attribute__((optimize("no-tree-vectorize")))

int  main(int argc, char** argv){
	
	short DistributionType=0;
	int dataSize;
	int insertValue; 
	int initialSize = 5000000;
	int totalSize = 50000000;
	int iteration;
/*
Linear Linear Probing Evalution
*/

for(iteration=0;iteration<5;iteration++){
	printf("\nScalar Linear Probing\n");

	DistributionType=0;
	clearHash();
	clearClocks();

		while(DistributionType<4){
			switch(DistributionType){

				case 0 :	printf("\nDense Unique Random Values\n");
							break;
				case 1 :	printf("\nSequential Values\n");
							break;
				case 2 :	printf("\nUniform Random Values\n"); 
							break;
				case 3 :	printf("\nExponential Values\n"); 
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

					default:	
								for(i=0;i<dataSize;i++){
									insertValue = Exponential();
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
	SIMD Cuckoo hasing evaluation
	*/
	printf("\nSIMD cuckoo hashing\n");

	DistributionType=0;
	clearHash();
	clearClocks();

	while(DistributionType<4){
			switch(DistributionType){

				case 0 :	printf("\nDense Unique Random Values\n");
							break;
				case 1 :	printf("\nSequential Values\n");
							break;
				case 2 :	printf("\nUniform Random Values\n"); 
							break;
				case 3 :	printf("\nExponential Values\n"); 
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

					default:	
								for(i=0;i<dataSize;i++){
									insertValue = Exponential();
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