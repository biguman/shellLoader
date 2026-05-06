#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "obf.h"

#ifdef _WIN32
    #include <windows.h>
    static HANDLE hProcessHeap = NULL;
    void* alloc(size_t size){
        if(!hProcessHeap){hProcessHeap = HeapCreate(0, 0, 0);}
        return HeapAlloc(hProcessHeap, 0, size);
    }
    void freeHeap(void* ptr){

        HeapFree(hProcessHeap, 0, ptr);
    }
#else
    #include <stdlib.h>

    void* alloc(size_t size){

        return malloc(size);
    }

    void freeHeap(void* ptr){
        free(ptr);
    }
#endif



    // This is supposed to print "real pc" by the end of it.
void trusted_machine(void){
    uint64_t XOR_key = 0;                           // our XOR decoding key for our "ascii constant", like the one we used earlier called "sum" and "sum2". Just another layer of obfuscation. 
                                                //Will calculate key using heap mem allocation difference like we did with "sum" pointers.
    uint64_t encoded_hex = 0x5de2fe86dfa1a8bd;
    char *output = alloc(sizeof(char) * 9);

    XOR_maker(&XOR_key);
    encoded_hex ^= XOR_key;


    for(int i = 0; i < 8; i++){

        output[i] = (encoded_hex >> (8 * i)) & (0xFF);        
    }

    output[8] = '\0';
    
    printf("%s\n", output);
 
    freeHeap(output);
}


    // Main logic for creating the XOR key
    
void XOR_maker(uint64_t* key){

      int **ptr_list = alloc(sizeof(int*) * 9);
      uint64_t sum_list[8];

      for(int i = 0; i < 9; i++){

          if((i%2) == 0){
              *(ptr_list + i) = alloc(sizeof(short) * i * 47);
          }
          else{
              *(ptr_list + i) = alloc(sizeof(char) * 193);
          }
      }

      for(int i = 0; i < 9; i++){
          printf("POINTER NUMBER %d ADDRESS: \t%#llx\n", i, (uint64_t)*(ptr_list + i));
      }

      for(int i = 0; i < 8; i++){

          sum_list[i] = (uint64_t)ptr_list[8] - ((uint64_t)ptr_list[i] >> 4);
      }

      for(int i = 0; i < 8; i++){

          *key |= (sum_list[i] & 0xFF) << (i * 8);
      }
      for(int i = 0; i < 9; i++){
          freeHeap(*(ptr_list + i));
      }

      freeHeap(ptr_list);
      printf("Key is: \t%#llx\n", *key);
  }
// With each visit of this code, I realize how mentally deficient i really am, every revisit is convincing me to make my repos private until they are fully complete
