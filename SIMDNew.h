#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>

typedef union {
  __m128i v;
  unsigned int ui;
  signed int si;
  unsigned short us;
  signed short ss;
} vec __attribute__ ((aligned (16)));


#define KEYSPERPAGE 1 
#define REPEATS 2 
#define HSIZE 20

int SIMDProbe(unsigned int key);
int SIMDInsert(unsigned int searchKey,unsigned int j);
void print128_num(__m128i var);

clock_t SIMDinsertBegin, SIMDinsertEnd, SIMDProbeBegin, SIMDProbeEnd, LinearinsertBegin, LinearinsertEnd, LinearProbeBegin, LinearProbeEnd;

static long double SIMDProbeTime, SIMDInsertTime, LinearProbeTime, LinearInsertTime;
int H = 2;
//32 bit keys and 32 bit payloads
typedef struct {
  __m128i keys;
  __m128i payloads;
} entry4;

typedef struct {
  __m128i keys;
  __m128i morekeys;
  __m128i payloads;
  __m128i indexes;
} entry8;

typedef struct {
  int bucket;
  int slot;
} idx;
typedef struct {
 int bucket; 
} idx1;

//#if (BFACTOR==4)
//typedef entry4 entry;
//#else
typedef entry8 entry;
//#endif

entry hashtable[HSIZE] __attribute__ ((aligned (128)));;
__m128i hashkeys[HSIZE] __attribute__ ((aligned (128)));;
__m128i hashpayloads[HSIZE] __attribute__ ((aligned (128)));;


unsigned int hashResult[2];
void hash(int key){
    hashResult[0] =(13*key)%HSIZE;
    hashResult[1] =4*((13*key)%HSIZE);
}
unsigned int hash2(int key){
  return 4*((13*key)%HSIZE);
}
unsigned int hash1(int key){
  return (13*key)%HSIZE;
}
int SIMDProbeInsert(unsigned int key){
  SIMDProbeBegin = clock();
  int SIMDProbeResult = SIMDProbe(key);
  SIMDProbeEnd = clock();
  SIMDProbeTime = SIMDProbeTime + ((long double)SIMDProbeEnd - (long double)SIMDProbeBegin);

  //if value not present, insert the value
  if(SIMDProbeResult>1){
    SIMDinsertBegin = clock();
    int insertReturn = SIMDInsert(key,SIMDProbeResult);
    SIMDinsertEnd = clock();
    if(insertReturn != -1){
      SIMDInsertTime = SIMDInsertTime +  ((long double)SIMDinsertEnd - (long double)SIMDinsertBegin);
      return 1;
    }
    return -1;
  }
  return 1;
}

int SIMDProbe(unsigned int key)
{
 
    SIMDProbeBegin = clock();

    //hash(key);
    unsigned int foffset = hash1(key);

    __m128i mask0 = _mm_setzero_si128();
    __m128i tmp;
    __m128i slot;
    __m128i k;
 
  for(int i=foffset; i<HSIZE; i++) {

	  slot = hashkeys[i];
    
      //Get the values of the offset into registers
    if(i<(foffset+H)) {


      //Key value replicated into SIMD registers
      k = _mm_set_epi32(key,key,key,key);

    //Compare the values in the registers and add to payload
    //Compare registers
      tmp = _mm_cmpeq_epi32(k,slot);

    //Check if any value is 1
      if(_mm_movemask_epi8(tmp)){
      
      //Add to Payload
        hashpayloads[i] = _mm_sub_epi32(hashpayloads[i],tmp);      
        return 1;
      }
    
      __m128i zeroPos = _mm_cmpeq_epi32(mask0,slot);
      int resMove = _mm_movemask_epi8(zeroPos);
      if(resMove){

        //Insert values
        //shift the values
        hashpayloads[i] = _mm_bslli_si128(hashpayloads[i],4);
        hashkeys[i] = _mm_bslli_si128(hashkeys[i],4);
      
        //Place the key in first position
        int *valkey = (int*) &hashkeys[i];
        int *val = (int*) &hashpayloads[i];
        val[0] = 1;
        valkey[0] = key;
        return 1;
      }
    }
    else  {
       __m128i zeroPos = _mm_cmpeq_epi32(mask0,slot);
       int *pt = (int*) &zeroPos;
       int resMove = _mm_movemask_epi8(zeroPos);    
       if (resMove)
       	if(pt[0]==-1) {
       		return i*4;
       	}
       	else if(pt[1]==-1) {
       		return (i*4)+1;
       	}
       	else if(pt[2]==-1) {
       		return (i*4)+2;
       	}
       	else {
       		return (i*4)+3;
       	}
    }
  }
  return 0;
}

int SIMDInsert(unsigned int searchKey,unsigned int jNew){

	clock_t begin = clock();
  
  unsigned int key = searchKey;
  //hash(key);

  unsigned int i = hash2(key);
	unsigned int j = jNew;
  int *indexes;

  printf("\nkey..\t%d\t%d\t%d\t%d\n",key,i,j,jNew );
  __m128i tmpA;
  __m128i tmpS;
  __m128i tmpJ;
  __m128i tmpI;
  __m128i tmpH;
 unsigned int k;
 int l=0;
 //idx1 *index = calloc(100,sizeof(idx1));
indexes = (int *)calloc(100,sizeof(int));
for(k = j; k>=i+(H*4);k=k-4)
{

   int *key6 = (int*) &hashkeys[0];
		//int m = hash2(key6[k-1]);
		//printf("\nwhats in hash...%d\t",m);
    tmpJ = _mm_set_epi32(hash2(key6[k-3]),hash2(key6[k-2]),hash2(key6[k-1]),hash2(key6[k]));
   //tmpJ = _mm_set_epi32(k-3,k-2,k-1,k);
   print128_num(tmpJ);
    tmpH = _mm_set_epi32(H*4,H*4,H*4,H*4);
    tmpI = _mm_set_epi32(i,i,i,i);

    tmpA = _mm_add_epi32(tmpI,tmpH);
    tmpS = _mm_sub_epi32(tmpJ,tmpA);

    __m128i zero = _mm_set1_epi32(-2);
    __m128i c = _mm_cmpgt_epi32(tmpS,zero);
    int *checkey = (int *) &c;


   if(checkey[0] == -1)
  {

   int *val = (int*) &tmpJ;

      indexes[l++] = val[0]; 

  }
  if(checkey[1] == -1)
  {
    int *val = (int*) &tmpJ;
      indexes[l++] = val[1]; 
  }
  if(checkey[2] == -1)
  { 
   int *val = (int*) &tmpJ; 
     indexes[l++] = val[2]; 

  }
  if(checkey[3] == -1)
  {
int *val = (int*) &tmpJ; 
// int *ins = (int *) &indexes;
//    ins[l++] = val[3];
   // int *ins = (int *) &index[0].bucket;
   //  ins[l] = val[3]; 
  indexes[l++] = val[3]; 
  //l++;
  } 


}
    for(i=0;i<20;i++) {
   // int *val = (int*) &index[i].bucket;
    printf("\nhello\t%d\n", indexes[i]);
    //printf("\nhellooo\n%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\n", 
     //      val[0], val[1], val[2], val[3],val[4],val[5],val[6],val[7],val[8],val[9],val[10],val[11],val[12],val[13],val[14],val[15]);
}
// if(l/4 != 0) {
//  l++;
// }

  for(int i=0;i<l;i=i+3) {
  	__m128i id = _mm_set_epi32(indexes[i+3],indexes[i+2],indexes[i+1],indexes[i]);
  	//__m128i id = _mm_loadl_epi64((__m128i *) &indexes[i]);
    int *valI = (int*) &id;

      printf("\nok\t");
     print128_num(id);

      int *key = (int*) &hashkeys[0];
      int *pay = (int*) &hashpayloads[0];

      if(valI[0] == 0) {
      	valI[0] = -1;
      }
       if(valI[1] == 0) {
      	valI[1] = -1;
      }
       if(valI[2] == 0) {
      	valI[2] = -1;
      }
       if(valI[3] == 0) {
      	valI[3] = -1;
      }

        __m128i values = _mm_i32gather_epi32 (key, id, 4);
         __m128i payloads = _mm_i32gather_epi32 (pay, id, 4);
     printf("\nyipeeeeee\t");
     print128_num(values);
     print128_num(payloads);

       __m128i scatteredkeys = _mm_bsrli_si128(values,4);
        __m128i scatteredpayloads = _mm_bsrli_si128(payloads,4);
     printf("\nyipee bhggff\t");
     print128_num(scatteredkeys);
     print128_num(scatteredpayloads);

	 int *val = (int*) &id;
     int *sck = (int*) &scatteredkeys;
     int *scp = (int*) &scatteredpayloads;
     key[val[0]] = sck[0];
     key[val[1]] = sck[1];
     key[val[2]] = sck[2];
     key[val[3]] = sck[3];

     pay[val[0]] = scp[0];
     pay[val[1]] = scp[1];
     pay[val[2]] = scp[2];
     pay[val[3]] = scp[3];
   
 }
 int *ptk = (int*) &hashkeys[0];
 if(ptk[indexes[l-1]]==0) {
 	printf("\nyess it is\t%d\n",l);
 	//int *ptk = (int*) &hashkeys[0];
 	int *ptp = (int*) &hashpayloads;
 	ptk[indexes[l-1]] = key;
 	ptp[indexes[l-1]] = 1;
 }
  //  if(H==HSIZE){
  //     return -1;
  //  }
  // else {        
  // 	H++;
  // SIMDInsert( searchKey, jNew); }
 int *key2 = (int*) &hashkeys[11];
 printf("heyyyyy\n%d\t%d\t%d\t%d\n",key2[0],key2[1],key2[2],key2[3] );

 //return 1;
}

   



void print128_num(__m128i var)
{
    int *val = (int*) &var;
    printf("%i\t%i\t%i\t%i\n", 
           val[0], val[1], val[2], val[3]);
}

void HashLookup(){
  int j;
 // printf("|-------------------------------|\n");
  for(j=0;j<HSIZE;j++){
   __m128i keys =  hashkeys[j];
   __m128i payloads =  hashpayloads[j];
   printf("|-------------------------------|\n|");
   print128_num(keys);
   printf("\t|\n|");
   print128_num(payloads);
   printf("\t|\n");
  }
  printf("|-------------------------------|\n");
}

int hashCheck(){
  int count = 0;
  int j;
  for(j=0;j<HSIZE;j++){
   int *keys =  (int*) &hashkeys[j];
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
   int *keys =  (int*) &hashpayloads[j];
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
    int *payload =  (int*) &hashpayloads[j];
    int *key =  (int*) &hashkeys[j];
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