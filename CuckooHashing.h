/*
 * Pentium probe using splash tables

   Ken Ross, rossak@us.ibm.com
   December 2005 - Feb 2006
   Version 0.2

   IBM Confidential

 */

//Scalar and vector done

#include <stdio.h>
#include <stdlib.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdint.h>
#include <inttypes.h>

 typedef union {
  __m128i v;
  unsigned int ui[4];
  signed int si[4];
  unsigned short us[8];
  signed short ss[8];
} vec __attribute__ ((aligned (16)));


#define KEYSPERPAGE 1 //1000
#define REPEATS 2 //500000
#define HSIZE 33000000
#define HASHFUNCTIONS 2
#define BFACTOR 4

//  The following constants must be defined on compilation
//HSIZE max number of entries in the splash table
//HASHFUNCTIONS number of hash functions used, between 2 and 4
//BFACTOR number of keys per bucket, 4 or 8 (if 8, then only implemented
//                                       for HASHFUNCTIONS=2)
 
  __m128i m0; /* original multipliers - we'll shift them to get m for efficiency */

__m128i tbsize;


  vec m;
  vec tb; 

/*
  Clock variables
*/

clock_t SIMDinsertBegin, SIMDinsertEnd, SIMDProbeBegin, SIMDProbeEnd, LinearinsertBegin, LinearinsertEnd, LinearProbeBegin, LinearProbeEnd;

static long double SIMDProbeTime, SIMDInsertTime, LinearProbeTime, LinearInsertTime;
  /*set multipliers: numbers below are prime but otherwise arbitrary
     should actually be chosen from among known "good" multipliers
     (see Knuth)
  */

  //Setting multipliers id there are only 2 functions
  void setM(){
    m.ui[0] = 1300000077;
    m.ui[2] = 1145678917;

    m0 = m.v;

    //Table size inserted to perform multiplication
    tbsize = _mm_set_epi32(HSIZE,HSIZE,HSIZE,HSIZE);  
  }
  


//SIMD Hashing function
__inline __m128i hash(__m128i k)
{
  __m128i h;

      /* 
	 Use 32-bit multiplies to compute 2 hash values at a time
      */
  h = _mm_mul_epu32(m0,k);  // hash values are in slots 0 and 2 
  // now multiply by table size; don't need to zero out parts of h.v because mult only looks at lower 32 bits

  h = _mm_mul_epu32(h,tbsize);

	//print128_num(h);	
  // result is in slots 1 and 3 (32-bit values), i.e., slots 2 and 6 (16-bit values)
  return h;
}

//32 bit keys and 32 bit payloads
typedef struct {
  __m128i keys;
  __m128i payloads;
} entry4;

typedef struct {
  __m128i keys;
  __m128i morekeys;
  __m128i payloads;
  __m128i morepayloads;
} entry8;


#if (BFACTOR==4)
typedef entry4 entry;
#else
typedef entry8 entry;
#endif

entry hashtable[HSIZE] __attribute__ ((aligned (128)));;

#if (HSIZE <= 65536)
#define kextract(hvec,size) _mm_extract_epi16(hvec,size)
#else
#define kextract(hvec,size) (_mm_extract_epi16(hvec,size) | (_mm_extract_epi16(hvec,size+1)<<16))
// The following is not as good as the above
//#define kextract(hvec,size) (((vec)hvec).ui[size/2])
#endif


int SIMDinsert(unsigned int searchKey){
  
  

 	register __m128i k;
	register __m128i h;
	register __m128i keys;
	register __m128i payloads;
	
	int count;
	//int tmpk,tmpp;
	int foffset;
  int payload = 1;
  int key = searchKey;
    
    for(count=0;((count<1000)&&(key!=0));count++){
   
    k=_mm_cvtsi32_si128(key);
	  k=_mm_shuffle_epi32(k,0);
	
	h = hash(k);	
	
	short countPercent = count%2;

	foffset = (!(countPercent)*kextract(h,2)) + ((countPercent)*kextract(h,6));
  	
		int *valkey = (int*) &hashtable[foffset].keys;
		int *val = (int*) &hashtable[foffset].payloads;
		
		int tmpk =  valkey[(2*countPercent)+1];
		int tmpp =  val[(2*countPercent)+1];

		//shift payload and key
		if(!countPercent){
			hashtable[foffset].payloads =	_mm_shuffle_epi32(hashtable[foffset].payloads,225);
			hashtable[foffset].keys =		_mm_shuffle_epi32(hashtable[foffset].keys,225);
		}
		else{
			hashtable[foffset].payloads =	_mm_shuffle_epi32(hashtable[foffset].payloads,180);
			hashtable[foffset].keys =		_mm_shuffle_epi32(hashtable[foffset].keys,180)	;
		}

		valkey[(2*countPercent)] = key;
		val[(2*countPercent)] = payload;
		key = tmpk;
		payload = tmpp;
  
	}
    if(count >= 1000){
      //printf("count : %d\n", count);
      return -1;
    }
    return count;


}

__inline int SIMDprobe(unsigned int key)
{
  int i;
  int foffset0,foffset1;
   __m128i slot0,slot1;
   __m128i compareSlots;
   __m128i tmp0,tmp1;
   __m128i payload0,payload1;

   __m128i k;
   __m128i h;

  k=_mm_cvtsi32_si128(key); /* replicate key into vector */
  k=_mm_shuffle_epi32(k,0);

  h = hash(k);

      /* h now contains four 32-bit values; the leading 16 bits of each are the
	 best hash bits */

      /* Use slot 1 of the hash to get slot array offset.
	 Assumes HSIZE <= 2^16; if not also need to use other slots
	 (Note limited local memory of 256KB on each SPU: 2^16x128 bits = 1MB.)
       */

	   //Got the position for first hash
  	foffset0 = kextract(h,2);
    //Get the key in slot
    slot0 = hashtable[foffset0].keys;
    //Compare key isprint128_num(tmp0); equal to the given key
    tmp0 = _mm_cmpeq_epi32(k,slot0);
    //get payload in the solt and multiply with tmp0 which has 0 or 1. Based on resultof tmp we get payload values
    hashtable[foffset0].payloads = _mm_sub_epi32(hashtable[foffset0].payloads,tmp0);

    int maskval = ((_mm_movemask_epi8((__m128i)tmp0)));
      if(maskval){
        return 0;
      }

    foffset1 = kextract(h,6);
    slot1 = hashtable[foffset1].keys;
    tmp1 = _mm_cmpeq_epi32(k,slot1);  
    tmp1 = _mm_andnot_si128(tmp0,tmp1);
    //Second hash position
    hashtable[foffset1].payloads = _mm_sub_epi32(hashtable[foffset1].payloads,tmp1);
    
    //Same Procedure
    //Combine the payloads
    payload0 = _mm_or_si128(payload0,payload1);

    //Check if payload is all zero
    __m128i zero = _mm_setzero_si128();
	int cmpzero1ero0 = _mm_movemask_epi8(_mm_cmpeq_epi32(tmp0,zero));
  int cmpzero1 = _mm_movemask_epi8(_mm_cmpeq_epi32(tmp1,zero));
      
  maskval = !( (_mm_movemask_epi8((__m128i)tmp1)));
  //printf("%d\n",maskval );
    return maskval;
}

int SIMDProbeInsert(unsigned int key){
  
  SIMDProbeBegin = clock();
  short SIMDinsertResult = SIMDprobe(key);
  SIMDProbeEnd = clock();
  SIMDProbeTime = SIMDProbeTime + ((long double)SIMDProbeEnd - (long double)SIMDProbeBegin);

  if(SIMDinsertResult){
      SIMDinsertBegin = clock();
      int insertReturn = SIMDinsert(key);
      SIMDinsertEnd = clock();
      if(insertReturn != -1){
        SIMDInsertTime = SIMDInsertTime +  ((long double)SIMDinsertEnd - (long double)SIMDinsertBegin);
        return 1;
      }
      return -1;
    }
 
  return 1;
}


//Linear Hahing Functions
//uint64_t something
unsigned int hashValues[4] = {1300000077,1145678917,2000000089,1709000081};
unsigned int hashResult[2];

void LinearHash(unsigned int key){	

//	return ((unsigned int)(hashValues[0]*key)* HSIZE)>>32;
  //printf("inside Kinear Hash\n");
	//hashResult[0] = hashValues[0]*key;
	hashResult[0] =(hashValues[0]*key)%HSIZE;
	//hashResult[1] = hashValues[1]*key;
	hashResult[1] =(hashValues[1]*key)%HSIZE;
	
}

int LinearProbe(unsigned int key){

  //Hash the key
  //Get the slots
  //Compare key with slot values
  //if found add the payload value
  LinearHash(key);
  //printf("Hash Result : %d\n",hashResult[0]);
  //printf("inside Kinear Probe\n");
  int *valkey = (int*) &hashtable[hashResult[0]].keys;
  int *val = (int*) &hashtable[hashResult[0]].payloads;
  //Compare key and add payload
  int i=0;

for(i=0;i<4;i++){
	val[i] += (valkey[i] == key);	
}
  if(((valkey[0] == key) + (valkey[1] == key) + (valkey[2] == key) + (valkey[3] == key))!=0){
     return 2;
  }

  //Pointer to hash slot1
  valkey = (int*) &hashtable[hashResult[1]].keys;
  val = (int*) &hashtable[hashResult[1]].payloads;
 for(i=0;i<4;i++){
	val[i] += (valkey[i] == key);	
}

  return  (valkey[0] == key) + (valkey[1] == key) + (valkey[2] == key) + (valkey[3] == key);
}

int LinearProbeInsert(unsigned int key){
  LinearProbeBegin = clock();
  int LinearProbeResult = LinearProbe(key);
  LinearProbeEnd = clock();
  LinearProbeTime = LinearProbeTime + ((long double)LinearProbeEnd - (long double)LinearProbeBegin);

  if(LinearProbeResult==0){
      LinearinsertBegin = clock();
      int insertReturn = LinearInsert(key);
      LinearinsertEnd = clock();
      if(insertReturn != -1){
        LinearInsertTime = LinearInsertTime +  ((long double)LinearinsertEnd - (long double)LinearinsertBegin);
        return 1;
      }
      return -1;
  }
  
  return 1;
}


int LinearInsert(unsigned int searchKey){
	clock_t begin = clock();
	int count;
	int tmpk,tmpp;
	int foffset;
    int payload = 1;
    int key = searchKey;
    //printf("inside Kinear Insert\n");
    for(count=0;((count<1000)&&(key!=0));count++){// for(count=0;((count<100)||(searchKey!=key))&&(key!=0);count++){
    		
  		LinearHash(key);
  		short percentCount = count%2;
  		//short slot = hashResult[percentCount];
      
      //Add cutting off for hashing functions
      //unsigned int hashResult = hashValues[percentCount]*key;
      //short slot = ((unsigned long)hashResult * HSIZE)>>32; 
      //Addition finished
     		int slot = hashResult[percentCount];
			int *valkey = (int*) &hashtable[slot].keys;
			int *val = (int*) &hashtable[slot].payloads;    		

			//Take the last value out
			int tmpp = val[(2*percentCount)+1];
			int tmpk = valkey[(2*percentCount)+1];

			//Move the front value to back
			val[(2*percentCount)+1] = val[2*percentCount];
			valkey[(2*percentCount)+1] = valkey[2*percentCount];

			//Insert the value in the position
			valkey[2*percentCount] = key;
			val[2*percentCount] = payload;

			//Storing the key and payload back to search values
			key = tmpk;
			payload = tmpp;

    	}

    	
// 		printf("Linsert : %g\n",(double)(end - begin));
	    
	    if(count >= 1000){
	      //printf("Rehash the table. Not able to insert %d\n",key);
        return -1;
	    }
	    return count;	

		}
//Linear Functions End here

int hashCheck(){
  int count = 0;
  int j;
  for(j=0;j<HSIZE;j++){
   int *keys =  (int*) &hashtable[j].keys;
   if(keys[0]!=0)
      count++;
    if(keys[1]!=0)
      count++;
    if(keys[2]!=0)
      count++;
    if(keys[3]!=0)
      count++;
  } 
 return count;
}

int addValues(){
  int count = 0;
  int j;
  for(j=0;j<HSIZE;j++){
   int *keys =  (int*) &hashtable[j].payloads;
   if(keys[0]!=0)
      count+=keys[0];
    if(keys[1]!=0)
      count+=keys[1];
    if(keys[2]!=0)
      count+=keys[2];
    if(keys[3]!=0)
      count+=keys[3];
  } 
 return count;

}

void clearHash(){
  int j;
  for(j=0;j<HSIZE;j++){
    int *payload =  (int*) &hashtable[j].payloads;
    int *key =  (int*) &hashtable[j].keys;
    payload[0] = 0;
    payload[1] = 0;
    payload[2] = 0;
    payload[3] = 0;
    key[0] = 0;
    key[1] = 0;
    key[2] = 0;
    key[3] = 0;

  }
}

void print128_num(__m128i var)
{
    int *val = (int*) &var;
    printf("%i\t%i\t%i\t%i", 
           val[0], val[1], val[2], val[3]);
}

void HashLookup(){
  int j;
  printf("|-------------------------------|\n");
  printf("|----TABLE1           TABLE2----|\n");
  for(j=0;j<HSIZE;j++){
   __m128i keys =  hashtable[j].keys;
   __m128i payloads =  hashtable[j].payloads;
   printf("|-------------------------------|\n|");
   print128_num(keys);
   printf("\t|\n|");
   print128_num(payloads);
   printf("\t|\n");
  }
  printf("|-------------------------------|\n");
}


long double getSIMDTime(){

  return SIMDInsertTime;

}

long double getSIMDProbeTime(){
  return SIMDProbeTime;
}

long double getLinearTime(){
  return LinearInsertTime;
}

long double getLinearProbeTime(){
  return LinearProbeTime;
}

void clearClocks(){
   SIMDInsertTime = 0;
   SIMDProbeTime = 0;
   LinearInsertTime = 0;
   LinearProbeTime = 0;
}