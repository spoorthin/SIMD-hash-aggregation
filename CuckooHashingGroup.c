#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include "CuckooHashing.h"
#include "distribution.h"

int  main(int argc, char** argv){
int x;
for(x=0;x<3;x++){	
	printf("\n\n");
	int insertValue; 
	int totalSize =  10000;
/*
Linear Cuckoo hashing Evalution
*/

setM();

printf("\nScalar cuckoo hashing\n");

setGen(101);

printf("GroupSize\tRepeatedValues\tUniqueValues\tunique\ttotal\tprobing\tinsertion\n");
int groupPercent = 10;
int groupPercentStepSize = 1;
int i,j;
for(;groupPercent<=100;groupPercent = groupPercent + groupPercentStepSize){
	
	LinearProbeInsert(SequentialNumbers(5));
	clearHash();
	clearClocks();

	int RepeatedValues = totalSize*groupPercent/100;
	int uniqueValues = totalSize - RepeatedValues;
	int repeatValue = SequentialNumbers(5);
	for(j=0;j<RepeatedValues;){
		if(LinearProbeInsert(repeatValue))
				j++;

	}
	
	for(i=0;i<uniqueValues;){
		insertValue =SequentialNumbers(5);
		if(LinearProbeInsert(insertValue))
			i++;
	}

	printf("%d\t%d\t%d\t%d\t%d\t%lf\t%lf\n",groupPercent,RepeatedValues,uniqueValues,hashCheck(),addValues(), (double)getLinearProbeTime(), (double)getLinearTime());
	clearHash();
	clearClocks();
}

printf("GroupSize\tRepeatedValues\tUniqueValues\tunique\ttotal\tprobing\tinsertion\n");
groupPercent = 10;
groupPercentStepSize = 1;

for(;groupPercent<=100;groupPercent = groupPercent + groupPercentStepSize){
	
	int RepeatedValues = totalSize*groupPercent/100;
	int uniqueValues = totalSize - RepeatedValues;
	int repeatValue = SequentialNumbers(5);
	for(j=0;j<RepeatedValues;){
		if(SIMDProbeInsert(repeatValue))
				j++;

	}
	
	for(i=0;i<uniqueValues;){
		insertValue =SequentialNumbers(5);
		if(SIMDProbeInsert(insertValue))
			i++;
	}

	printf("%d\t%d\t%d\t%d\t%d\t%lf\t%lf\n",groupPercent,RepeatedValues,uniqueValues,hashCheck(),addValues(), (double)getSIMDProbeTime(), (double)getSIMDTime());
	clearHash();
	clearClocks();
}
	

}

}