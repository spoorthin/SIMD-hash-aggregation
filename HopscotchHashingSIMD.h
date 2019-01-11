#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <emmintrin.h>
#include <smmintrin.h>

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
#define H 3
int LinearInsert(unsigned int searchKey);
int findNeighborhood (unsigned int i, unsigned int j,unsigned int key);
 int findEmptySlot (unsigned int key);
//Clock Variables


clock_t SIMDinsertBegin, SIMDinsertEnd, SIMDProbeBegin, SIMDProbeEnd, LinearinsertBegin, LinearinsertEnd, LinearProbeBegin, LinearProbeEnd;

static long double SIMDProbeTime, SIMDInsertTime, LinearProbeTime, LinearInsertTime;

//32 bit keys and 32 bit payloads
typedef struct {
  __m128i keys;
  __m128i payloads;
} entry4;

typedef struct {
  __m128i keys;
 // __m128i morekeys;
  __m128i payloads;
 // __m128i indexes;
} entry8;

//#if (BFACTOR==4)
//typedef entry4 entry;
//#else
typedef entry8 entry;
//#endif

entry hashtable[HSIZE] __attribute__ ((aligned (128)));;


//32 bit keys and 32 bit payloads
 unsigned int hash(int key){
  
  //return ((unsigned long)((unsigned int)1300000077*key)* HSIZE)>>32; 
  return 4*(13*key)%HSIZE; 

}

int SIMDProbeInsert(unsigned int key){
  SIMDProbeBegin = clock();
  int SIMDProbeResult = SIMDProbe(key);
  SIMDProbeEnd = clock();
  SIMDProbeTime = SIMDProbeTime + ((long double)SIMDProbeEnd - (long double)SIMDProbeBegin);
  //printf("yes!%d\n",LinearProbeResult);
  //if value not present, insert the value
  if(SIMDProbeResult!=0){
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

int SIMDprobe(unsigned int key)
{
    SIMDProbeBegin = clock();
  //Hash the key
  unsigned int foffset = hash(key);
  __m128i mask0 = _mm_setzero_si128();
  __m128i tmp;
  register __m128i slot;
  register __m128i k;
  unsigned int i;
        __m128i zeroPos2 = _mm_cmpeq_epi32(mask0,slot);
    int resMove2 = _mm_movemask_epi8(zeroPos2);
    
  for(i=foffset; i<HSIZE; i++) {
    slot = hashtable[i].keys;
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
      hashtable[i].payloads = _mm_sub_epi32(hashtable[i].payloads,tmp);      
      return 1;
    }
    
    __m128i zeroPos = _mm_cmpeq_epi32(mask0,slot);
    int resMove = _mm_movemask_epi8(zeroPos);
    if(resMove){
      
      //Insert values
      //shift the values
      hashtable[i].payloads = _mm_bslli_si128(hashtable[i].payloads,4);
      hashtable[i].keys = _mm_bslli_si128(hashtable[i].keys,4);
      
      //Place the key in first position
      int *valkey = (int*) &hashtable[i].keys;
      int *val = (int*) &hashtable[i].payloads;
      val[0] = 1;
      valkey[0] = key;

      
    }

  }


  else if (resMove2) {
    return i;
  }
 
}
 return 0;
}

int SIMDInsert(unsigned int searchKey,unsigned int j){
  clock_t begin = clock();
  
  //find initial offset i (original position of value)
  //find offset j of next exmpty slot (linear probe)
  unsigned int key = searchKey;
  unsigned int i = hash(key);

  register __m128i tmpA;
  register __m128i tmpS;
  register __m128i J2;
  register __m128i res;
  register __m128i tmpJ;
  register __m128i tmpI;
  __m128i tmpH;
 unsigned int jj;
for(jj = j; jj>=i+H;jj=jj-4)
{
    tmpJ = _mm_set_epi32(jj,jj-1,jj-2,jj-3);
    tmpH = _mm_set_epi32(H,H,H,H);
    tmpI = _mm_set_epi32(i,i,i,i);

    tmpA = _mm_add_epi32(tmpI,tmpH);
    tmpS = _mm_sub_epi32(tmpJ,tmpA);
    __m128i zero = _mm_set1_epi32(0);
    int *checkey = (int *)_mm_cmpgt_epi32(tmpS,zero);
    checkey[0] =-1 // positive
    checkey[1] =-1 // positive
    checkey[2] =-1 // positive
    checkey[3] = 0 // negative
    

    register _m128i swapper = _mm_set_epi32(hashtable.keys[J2[0]],
    hashtable.keys[J2[1]],
    hashtable.keys[J2[2]],
    hashtable.keys[J2[3]]);
    res = _mm_bslli_si128(swapper, mask);
}

  hashtable.keys[J2[0]] = i;
  hashtable.payload[J2[0]] = 1;

//   while (j>=(i+H)) {
//      tmpJ = _mm_loadl_epi64(j);
//   }
 
//   tmpI = _mm_set_epi32(i,i,i,i);

//   tmpH = _mm_set_epi32(H,H,H,H);

//   tmpA = _mm_add_epi32(tmpI,tmpH);
//   tmpS = _mm_sub_epi32(tmpJ,tmpA)

//   J2 = _mm_cmpge_epi32(tmpS,0);

//   //Add i which is new key at end of J2.
  
//   for (int i = 0;i < length[J2];i+4){
// //int index[10];
//   //j3 =hashtable[index[i]].keys

// __m128i swapper = _mm_set_epi32(hashtable.keys[index[[i]],
//     hashtable.keys[index[[i+1]],
//       hashtable.keys[index[[i+2]],
//         hashtable.keys[index[[i+3]]);
//  res = _mm_bslli_si128(swapper, mask);

// }
//   hashtable.keys[res[0]] = i;
//   hashtable.payload[res[0]] = 1;

    return -1; 
}

//  int findEmptySlot (unsigned int key) {
//   unsigned int offset = hash(key);
//   __m128i mask0 = _mm_setzero_si128();
//   register __m128i slot;


//   //find empty slot, linear probe
//   while (offset<HSIZE){
//     slot = hashtable[offset].keys;
//     __m128i zeroPos = _mm_cmpeq_epi32(mask0,slot);
//     int resMove = _mm_movemask_epi8(zeroPos);

//     if(resMove){
//      return offset;
//     }    
//     offset++;
//   }
//   return -1;
// }

// int findNeighborhood (unsigned int i, unsigned int j,unsigned int key) {
  //  __m128i tmpA;
  // __m128i tmpS;
  // __m128i J2;
  // register __m128i tmpJ;
  // register __m128i tmpI;
  // register __m128i tmpH;

  // while (j>=(i+H)) {
  //    tmpJ = _mm_loadl_epi64(j); 
  // }

  // tmpI = _mm_set_epi32(i,i,i,i);
  // tmpH = _mm_set_epi32(H,H,H,H);

  // tmpA = _mm_add_epi32(tmpI,tmpH);
  // tmpS = _mm_sub_epi32(tmpJ,tmpA)

  // J2 = _mm_cmpge_epi32(tmpS,0);

//   // for(k=0;k<;k++) {
//   //   int off = J2[k];
//   //   int tmpk = 
//   //   int tmpp = 
//   // }

//   return -1; //if not rehash
// }
void print128_num(__m128i var)
{
    int *val = (int*) &var;
    printf("%i\t%i\t%i\t%i", 
           val[0], val[1], val[2], val[3]);
}

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
  // printf("here =%d\n", count);
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
  printf("|----TABLE-------|\n");
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


for (){
int index[10];

__m128i swapper = _mm_set_epi32(hashtable.keys[index[[0]],
    hashtable.keys[index[[1]],
      hashtable.keys[index[[2]],
        hashtable.keys[index[[3]]);
__m128i res = _mm_bslli_si128(swapper, mask);
hashtable.keys[index[[0]] = res[0];
}