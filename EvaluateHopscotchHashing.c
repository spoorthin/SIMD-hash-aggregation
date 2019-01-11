#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include "distribution.h"
#include "HopscotchHashing.h"

int  main(int argc, char** argv){
	
	short DistributionType=0;
	int dataSize;
	int insertValue; 
	int initialSize = 5;
	int totalSize = 100;
	int iteration;


//setM();
for(iteration=0;iteration<1;iteration++)
{
	printf("\nLinear hopscotch hashing\n");

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
			printf("Datasize\tunique\ttotal\tscalar probing\tscalar insertion\n");

			for(dataSize=initialSize;dataSize<totalSize;dataSize+=initialSize){		
				
				int i;
				setGen(dataSize); //Initialize generation Size
				//set_hsize(dataSize/4+1);				
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

					case 3:	
								for(i=0;i<dataSize;i++){
									insertValue = Exponential();
									LinearProbeInsert(insertValue);
								}
								break;
					case 4:	
								for(i=0;i<dataSize;i++){
									insertValue = zipf(1000,0.6);
									LinearProbeInsert(insertValue);
								}
								break;
					default:	
								for(i=0;i<dataSize;i++){
									insertValue = selfsimilar(25,0.1);
									LinearProbeInsert(insertValue);
								}
								break;								

				}
				printf("%d\t%d\t%d\t%Lf\t%Lf\t%d\n",dataSize,hashCheck(),addValues(), getLinearProbeTime(), getLinearTime(),DistributionType);
				clearHash();
				clearClocks();
			}

			DistributionType++;

		}


	/*
	SIMD Hopscotch hasing evaluation
	*/
	printf("\nSIMD Hopscotch hashing\n");

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

					case 3:	
								for(i=0;i<dataSize;i++){
									insertValue = Exponential();
									SIMDProbeInsert(insertValue);
								}
								break;
					case 4:	
								for(i=0;i<dataSize;i++){
									insertValue = zipf(1000,0.6);
									SIMDProbeInsert(insertValue);
								}
								break;
					default:	
								for(i=0;i<dataSize;i++){
									insertValue = selfsimilar(25,0.1);
									SIMDProbeInsert(insertValue);
								}
								break;									

				}
				printf("%d\t%d\t%d\t%Lf\t%Lf\t%d\n",dataSize,hashCheck(),addValues(), getSIMDProbeTime(), getSIMDTime(),DistributionType);
				clearHash();
				clearClocks();
			}

			DistributionType++;

		}
	}
}