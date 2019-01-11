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
#define H 2
int SIMDProbe(unsigned int key);
int SIMDInsert(unsigned int searchKey,unsigned int j);
void print128_num(__m128i var);
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

typedef struct {
  int bucket;
  int slot;
} idx;
typedef struct {
 __m128i bucket; 
} idx1;

//#if (BFACTOR==4)
//typedef entry4 entry;
//#else
typedef entry8 entry;
//#endif

entry hashtable[HSIZE] __attribute__ ((aligned (128)));;
__m128i hashkeys[HSIZE] __attribute__ ((aligned (128)));;
__m128i hashpayloads[HSIZE] __attribute__ ((aligned (128)));;
__m128i indexes[100] __attribute__ ((aligned (128)));;

//32 bit keys and 32 bit payloads
 unsigned int hash1(int key){
  
  //return ((unsigned long)((unsigned int)1300000077*key)* HSIZE)>>32; 
  return (13*key)%HSIZE;

}
 unsigned int hash2(int key){
  
  //return ((unsigned long)((unsigned int)1300000077*key)* HSIZE)>>32; 
  return 4*((13*key)%HSIZE); 

}

int SIMDProbeInsert(unsigned int key){
  SIMDProbeBegin = clock();
  int SIMDProbeResult = SIMDProbe(key);
  //printf("yess..%d\n",SIMDProbeResult);
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
  //Hash the key
  unsigned int foffset = hash1(key);
    __m128i mask0 = _mm_setzero_si128();
  __m128i tmp;
  register __m128i slot;
  register __m128i k;
 // printf("key..%d\t%d\n",key,foffset );
   int i;
 
  for(i=foffset; i<HSIZE; i++) {


    slot = hashkeys[i];
   // print128_num(slot);

    
      //Get the values of the offset into registers
   if(i<(foffset+H)) {


    //Key value replicated into SIMD registers
    k = _mm_set_epi32(key,key,key,key);
//print128_num(k);
    //Compare the values in the registers and add to payload
    //Compare registers
    tmp = _mm_cmpeq_epi32(k,slot);
  // print128_num(tmp);
    //Check if any value is 1
    
    if(_mm_movemask_epi8(tmp)){
      
      //Add to Payload
      hashpayloads[i] = _mm_sub_epi32(hashpayloads[i],tmp);      
      return 1;
    }
    
    __m128i zeroPos = _mm_cmpeq_epi32(mask0,slot);
    //print128_num(zeroPos);
    int resMove = _mm_movemask_epi8(zeroPos);
    //printf("%d\t\n",resMove );
    if(resMove){
      //print128_num(resMove);
      //Insert values
      //shift the values
      hashpayloads[i] = _mm_bslli_si128(hashpayloads[i],4);
      hashkeys[i] = _mm_bslli_si128(hashkeys[i],4);
      
      //Place the key in first position
      int *valkey = (int*) &hashkeys[i];
      int *val = (int*) &hashpayloads[i];
      val[0] = 1;
      valkey[0] = key;
      //printf("yes insr%d\t%d\n",valkey[i],i);
      return 1;
   }

}
  else  {
       __m128i zeroPos2 = _mm_cmpeq_epi32(mask0,slot);
    int resMove2 = _mm_movemask_epi8(zeroPos2);    
    if (resMove2)
    return i;
  }
 
}
 return 0;
}

// int SIMDInsert(unsigned int searchKey,unsigned int jNew){
//   clock_t begin = clock();
  
//   //find initial offset i (original position of value)
//   //find offset j of next exmpty slot (linear probe)
//   unsigned int key = searchKey;
//   unsigned int i = hash2(key);
    
// unsigned int j = jNew*4;
// printf("\nkey..%d\t%d\t%d\n",key,i,j );
//   __m128i tmpA;
//   __m128i tmpS;
//   __m128i J2;
//   __m128i res;
//   __m128i tmpJ;
//   __m128i tmpI;
//   __m128i tmpH;
//  unsigned int k;
//  int l=0;
//  idx *index = calloc(100,sizeof(idx));
// //int *index = (idx *)calloc(100,sizeof(idx));
//   //index = (idx *)calloc(100,sizeof(idx),-1);
// for(k = j; k>=i+(H*4);k=k-4)
// {
//     int j2Index=0;
//     tmpJ = _mm_set_epi32(k,k-1,k-2,k-3);
//     print128_num(tmpJ);
//     tmpH = _mm_set_epi32(H*4,H*4,H*4,H*4);
//     print128_num(tmpH);
//     tmpI = _mm_set_epi32(i,i,i,i);
//     print128_num(tmpI);

//     tmpA = _mm_add_epi32(tmpI,tmpH);
//     tmpS = _mm_sub_epi32(tmpJ,tmpA);
//     print128_num(tmpS);

//     __m128i zero = _mm_set1_epi32(-1);
//     __m128i c = _mm_cmpgt_epi32(tmpS,zero);
//     print128_num(c);
//     int *checkey = (int *) &c;


//   if(checkey[0] == -1)
//   {

//  //__m128i *tmpJJ = (__m128i *) &tmpJ;
//    int *val = (int*) &tmpJ;

//     index[l].bucket = val[0]/4;
//     index[l].slot = val[0]%4; 
//   }
//   if(checkey[1] == -1)
//   {
//   //__m128i *tmpJJ = (__m128i *) &tmpJ;
//     int *val = (int*) &tmpJ;
//     index[l+1].bucket = val[1]/4;
//     index[l+1].slot = val[1]%4;
//   }
//   if(checkey[2] == -1)
//   { 
//    // __m128i *tmpJJ = (__m128i *) &tmpJ;
//    int *val = (int*) &tmpJ;
//     index[l+2].bucket =val[2]/4;
//     index[l+2].slot = val[2]%4; 

//   }
//   if(checkey[3] == -1)
//   {
//     printf("yesss\n");
//      //__m128i *tmpJJ = (__m128i *) &tmpJ;
// int *val = (int*) &tmpJ;
//     printf("%i\t%i\t%i\t%i\n", 
//            val[0], val[1], val[2], val[3]);
//     index[l+3].bucket = val[3]/4;
//      int buck =  index[l+3].bucket;
//      printf("\n%i\n",buck);
//     //   int *val1 = (int*) &buck;
//     // printf("%i\t%i\t%i\t%i\n", 
//     //        val1[0], val1[1], val1[2], val1[3]);
//     index[l+3].slot = val[3]%4;
//   }
//   l+=4;
// }

//   // for(j=0;j<HSIZE;j++){
//   //  int buckets =  index[j].bucket;
//   //   printf("\n%i\n",buckets);
//   // }

// // 1,2,3,4,5,"x",7,"6",9,10,"8",12,"11"
// //index -> 5,7,10,12
// // 6,8,11,x -> x,6,8,11 
// //
// for (int i=0;i<100;i+3) {
//   idx = index[i];
//  __m128i offi = _mm_set_epi32(&index[i].bucket,128);
//  __m128i sloti = _mm_set_epi32(&index[i].slot,128);
//   __m128i id = _mm_set_epi32(&idx.buckets,96);


//  int *valkey = hashtable[index[i+3].bucket].keys;
//    slotN = index[i+3].slot;
//  __m128i values = _mm_mmask_i32gather_epi32 (valkey[slotN],0,i,&valkey[slotN],4);

//  __m128i val = _mm_set_epi32(hashtable[index[i]].keys,hashtable[index[i+1]].keys,
//                               hashtable[index[i+2]].keys,hashtable[index[i+3]].keys);
//  // __m128i values = _mm_i32gather_epi32(id,val,4);
  
//       __m128i scattered = _mm_bsrli_epi128(values,4);

//       __m128i sIndx;
//       for(int offIndx=0;offIndx<length.offi;offIndx++){
//         for(int slotIndx=0;slotIndx<length.sloti;slotIndx++){
//           int i=0;
//           __m128i buck = offi[offIndx];
//           int *valkey = hashtable[offi[offIndx]].keys;
//           valkey[sloti[slotIndx]] = scattered[i++];
//         }

//       }
//       hashtable[i].keys = scattered

// }

//     return -1; 
// }
int SIMDInsert(unsigned int searchKey,unsigned int jNew){
  clock_t begin = clock();
  
  //find initial offset i (original position of value)
  //find offset j of next exmpty slot (linear probe)
  unsigned int key = searchKey;
  unsigned int i = hash2(key);
    unsigned int j;
    int *valkey1 = (int*) &hashkeys[jNew];
   // printf("%i\t%i\t%i\t%i\n", 
   //         valkey1[0], valkey1[1], valkey1[2], valkey1[3]);
    if(valkey1[0]==0) {
      j=jNew*4;
    }
    else if(valkey1[1]==0) {
      j=(jNew*4)+1;
    }
    else if(valkey1[2]==0) {
      j=(jNew*4)+2;
    }
    else  {
      j=(jNew*4)+3;
    }
//unsigned int j = jNew*4;
printf("\nkey..%d\t%d\t%d\n",key,i,j );
  __m128i tmpA;
  __m128i tmpS;
  __m128i J2;
  __m128i res;
  __m128i tmpJ;
  __m128i tmpI;
  __m128i tmpH;
 unsigned int k;
 int l=0;
 idx1 *index = calloc(100,sizeof(idx1));
//int *index = (idx *)calloc(100,sizeof(idx));
  //index = (idx *)calloc(100,sizeof(idx),-1);
for(k = j; k>=i+(H*4);k=k-4)
{

    tmpJ = _mm_set_epi32(k-3,k-2,k-1,k);
    print128_num(tmpJ);
    tmpH = _mm_set_epi32(H*4,H*4,H*4,H*4);
    print128_num(tmpH);
    tmpI = _mm_set_epi32(i,i,i,i);
    print128_num(tmpI);

    tmpA = _mm_add_epi32(tmpI,tmpH);
    tmpS = _mm_sub_epi32(tmpJ,tmpA);
    print128_num(tmpS);

    __m128i zero = _mm_set1_epi32(-1);
    __m128i c = _mm_cmpgt_epi32(tmpS,zero);
    print128_num(c);
    int *checkey = (int *) &c;


   if(checkey[0] == -1)
  {

   int *val = (int*) &tmpJ;
   // int *ins = (int *) &indexes;
   // ins[l++] = val[0];
   int *ins = (int *) &idx1.bucket;
    index[l].bucket = val[0]; 
  l++;
  }
  if(checkey[1] == -1)
  {
    int *val = (int*) &tmpJ;
    int *ins = (int *) &indexes;
     ins[l++] = val[1];
    index[l].bucket = val[1];
  l++;
  }
  if(checkey[2] == -1)
  { 
   int *val = (int*) &tmpJ;
   int *ins = (int *) &indexes;
   ins[l++] = val[2];
    index[l].bucket = val[2];
  l++;

  }
  if(checkey[3] == -1)
  {
int *val = (int*) &tmpJ; 
int *ins = (int *) &indexes;
   ins[l++] = val[3];
    index[l].bucket = val[3]; 

  l++;
  } 


  // printf("\nl is\t%d",l);
  // int *val = (int*) &indexes;
  //   printf("hellooo%i\t%i\t%i\t%i\n", 
  //          val[0], val[1], val[2], val[3]);
}


  // for(i=0;i<30;i++) {
  //   int *ins = (int *) &indexes;
  //   print128_num(ins[0]);
  //   // __m128i ind =  index[i].bucket;
  //   // print128_num(ind[o]);
  // }

    int *val = (int*) &indexes;
    printf("\nhellooo\n%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\n", 
           val[0], val[1], val[2], val[3],val[4],val[5],val[6],val[7],val[8],val[9],val[10],val[11],val[12],val[13],val[14],val[15]);



// 1,2,3,4,5,"x",7,"6",9,10,"8",12,"11"
//index -> 5,7,10,12
// 6,8,11,x -> x,6,8,11 
//

  for(int i=0;i<15;i=i+3) {
    //__m128i id = _mm_set_epi32(index[i].bucket,index[i+1].bucket,index[i+2].bucket,index[i+3].bucket);
    //__m128i *val = (__m128i*) &index[i].bucket;
   __m128i vindex =   _mm_load_si128 (&indexes[i]);
    printf("\nok\t");
    print128_num(vindex);


     int *key = (int*) &hashkeys[0];
    // int buck1 = val[0]/4;
    // int slot1= val[0]%4;
    // int *pt1 = (int *) &hashkeys[val[0]/4];
    // int *pt2 = (int *) &pt1[val[0]%4];

    // __m128i vr1 = _mm_set_epi32((index[i+3].bucket)%4,(index[i+2].bucket)%4,(index[i+1].bucket)%4,(index[i].bucket)%4);
    // __m128i vr2 = _mm_set_epi32(0,1,2,3);

     __m128i values = _mm_i32gather_epi32 (key, vindex, 4);
     printf("\nyipeeeeee\t");
     print128_num(values);

     __m128i scattered = _mm_bsrli_si128(values,4);
     printf("\nyipee bhggff\t");
     print128_num(scattered);

    //  int *place = (int*) &scattered;
    //  pt2[0] = place[0];
    // pt2[1] =place[1];
    // pt2[2] = place[2];
    // pt2[3] =place[3];

        // printf("\n\thellooo%i\t%i\t%i\t%i\n", 
        //    pt2[0], pt2[1], pt2[2], pt2[3]);
  }
//for (int i=0;i<5;i+3) {

//   int *id = (int *) &hashkeys[i];
//  //int *valkey = hashtable[index[i+3].bucket].keys;
//  //__m128i values = _mm_mmask_i32gather_epi32 (valkey[slotN],0,i,&valkey[slotN],4);

//  //__m128i val = _mm_set_epi32(hashtable[].keys,hashtable[id[1].bucket].keys,
//  //                            hashtable[id[2].bucket].keys,hashtable[id[3].bucket].keys);


//   __m128i values = _mm_i32gather_epi32(id,indexes[i],4);
//   //print128_num(values);
//   __m128i scattered = _mm_bsrli_si128(values,4); 

// //__m128i *tmpJJ = (__m128i *) &tmpJ;
// int *val = (int*) &indexes[i]; 
//     int newV = val[0];
    
//   hashtable[newV].keys = scattered;
  // hashtable[index[i+1].bucket].keys = _mm_extract_epi32(scattered,1);
  // hashtable[index[i+2].bucket].keys = _mm_extract_epi32(scattered,2);
  // hashtable[index[i+3].bucket].keys = _mm_extract_epi32(scattered,3);

//}

    return -1; 
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


/*
typedef struct{
  short slot;
  short bucket;
}idx;


void *index =(idx *)calloc(100,sizeof(idx),-1);

... run and get all the swap indices

swap(idx index){
  for(i=0i<n;i++){
      __m128i i = _mm_set_epi32(base_addr,number of bytes ahead - 96 bits or 12 bytes);
      //shuffling
      1) __m128i values = _mm_gather..
      2) __m128i val = _mm_set_epi32(table[index[3]],table[index[1]],
                                      table[index[2]],table[index[3]])
      __m128i scattered = _mm_bsrli_epi128(i,4) 
      table[i] = scattered
  }
}

1,2,3,4 -> 2,3,4,0

*/