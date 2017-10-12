#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include "distribution.h"
#include "CuckooHashing.h"

int  main(int argc, char** argv){
	
	short DistributionType=0;
	int dataSize;
	int insertValue; 
	int initialSize = 5000000;
	int totalSize = 50000000;
	int iteration;
/*	
Linear Cuckoo hashing Evalution
*/

setM();

for(iteration=0;iteration<20;iteration++)
{
	printf("\nLinear cuckoo hashing\n");

	DistributionType=0;
	clearHash();
	clearClocks();

		while(DistributionType<4){
			printf("Datasize\tunique\ttotal\tscalar probing\tscalar insertion\n");

			for(dataSize=initialSize;dataSize<totalSize;dataSize+=initialSize){		
				
				int i;
				setGen(dataSize); //Initialize generation Size
				
				switch(DistributionType){ //Populate with values

					case 0:	
							setGen(dataSize*3);
							InitDenseUnique();
							for(i=0;i<dataSize;i++){
								insertValue = DenseUniqueRandom();
								LinearProbeInsert(insertValue);
							}
							break;

					case 1: 
							for(i=0;i<dataSize;i++){
								insertValue = SequentialNumbers(5);
								LinearProbeInsert(insertValue);
							}
							break;

					case 2: 
							for(i=0;i<dataSize;i++){
								insertValue = UniformRandom();
								LinearProbeInsert(insertValue);
							}
							break;

					default:	
								for(i=0;i<dataSize;i++){
									insertValue = Exponential();
									LinearProbeInsert(insertValue);
								}
								break;

				}
				printf("%d\t%d\t%d\t%Lf\t%Lf\n",dataSize,hashCheck(),addValues(), getLinearProbeTime(), getLinearTime());
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
			
			printf("Datasize\tunique\ttotal\tSIMD probing\tSIMD insertion\n");

			for(dataSize=initialSize;dataSize<totalSize;dataSize+=initialSize){	
				
				int i;
				setGen(dataSize); //Initialize generation Size
				
				switch(DistributionType){ //Populate with values

					case 0:	
							setGen(dataSize);
							InitDenseUnique();
							for(i=0;i<dataSize;i++){
								insertValue = DenseUniqueRandom();
								SIMDProbeInsert(insertValue);
							}
							break;

					case 1: 
							for(i=0;i<dataSize;i++){
								insertValue = SequentialNumbers(5);
								SIMDProbeInsert(insertValue);
							}
							break;

					case 2: 
							for(i=0;i<dataSize;i++){
								insertValue = UniformRandom();
								SIMDProbeInsert(insertValue);
							}
							break;

					default:	
								for(i=0;i<dataSize;i++){
									insertValue = Exponential();
									SIMDProbeInsert(insertValue);
								}
								break;

				}
				printf("%d\t%d\t%d\t%Lf\t%Lf\n",dataSize,hashCheck(),addValues(), getSIMDProbeTime(), getSIMDTime());
				clearHash();
				clearClocks();
			}

			DistributionType++;

		}
	}
}