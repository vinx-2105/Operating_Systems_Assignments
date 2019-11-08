#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define BUFFER_SIZE 256
#define FRAME_SIZE 256
#define NUM_FRAMES 256
#define PAGE_TABLE_SIZE 256
#define TLB_SIZE 16


//function to randomly access the file BACKING_STORE and returns a page frame from backing store

//retuns 1 if read successfully and -1 if not
int read_backing_store(char *bs_read_buffer, short frame_number){//gives the frame number to be read from the back store
    FILE *bs_file_pointer;
    bs_file_pointer = fopen("BACKING_STORE.bin", "rb");

    if(fseek(bs_file_pointer, frame_number*FRAME_SIZE, SEEK_SET)==0){
        //store the page frame from the back store and store it in array of 256 size
        fread(bs_read_buffer, 1, FRAME_SIZE, bs_file_pointer);

        //print the numbers read into the array to check
        // for(int i=0; i<BUFFER_SIZE; i++){
        //     printf("%d: %d\n",i, bs_read_buffer[i]);
        // }
        fclose(bs_file_pointer);
        return 0;
    }

    else{
        fclose(bs_file_pointer);
        return -1;
    }

}


int main(int argc, char* argv[]){

    if(argc!=2){
        printf("Unexpected arguments\n");
        return 0;
    }

    char *bs_read_buffer=NULL;

    // read_backing_store(1);
    // return 0;
    // bs_read_buffer = NULL;
    bs_read_buffer = (char *)malloc(BUFFER_SIZE*sizeof(char));
    // read_backing_store(255);

    short PHYS_MEM[NUM_FRAMES][FRAME_SIZE];
    short TLB[TLB_SIZE][3];//16 entries in the TLB and 3 columns in the TLB
    //0th column is page number, 1st column is frame number, 2nd column is time stamp of last use
    //init all time stamps with -1
    for(int i=0; i<TLB_SIZE; i++) TLB[i][2]=-1;

    short PAGE_TABLE[PAGE_TABLE_SIZE];

    //init page table with -1
    for(int i=0; i<PAGE_TABLE_SIZE; i++){
        PAGE_TABLE[i]=-1;
    }

    int curr_page = 0;

    //open the file where results are to be written
    FILE* result_file = fopen("result.txt","w");

    //open the addresses file
    FILE *add_file = fopen(argv[1], "r");
    // printf("\n");

    int page_faults = 0;
    int tlb_hits = 0;

    //iterate through the addresses and get the physical and virtual addresses
    //i also denotes the clock tick
    int address;
    int i=0;
    while(fscanf(add_file, "%d", &address)==1){
        short page_num = (address&65535)>>8;
        short page_offset = address&255;

        // printf("page num: %d; page_offset: %d\n", page_num, page_offset);

        //will be set to true in case of tlb hit
        bool tlb_found=false;

        //first check the TLB...scan linearly
        for(int j=0; j<TLB_SIZE; j++){
            if(TLB[j][0]==page_num){
                tlb_found=true;
                TLB[j][2] = i;
                tlb_hits++;
                fprintf(result_file, "Virtual address: %d Physical address: %d Signed Byte Value: %d\n", address, FRAME_SIZE*TLB[j][1]+page_offset, PHYS_MEM[TLB[j][1]][page_offset]);
                break;
            }
        }
        if(tlb_found){
            i++;
            continue;
        }

        //get the frame with frame_num into physical memory if not there already
        //go and check frame_num index of the page table
        //if page fault occurs
        if(PAGE_TABLE[page_num]==-1){
            page_faults++;
            read_backing_store(bs_read_buffer, page_num);
            //copy the contents to curr_page and increment curr_page
            //read from bs_read_buffer and copy to physical_memory[curr_page]
            for(int j=0; j<FRAME_SIZE; j++){
                PHYS_MEM[curr_page][j] = bs_read_buffer[j];
            }
            PAGE_TABLE[page_num] = curr_page;
            curr_page++;
        }


        //will reach here only when tlb miss but page hit or fault
        //check if there is an empty slot in the tlb
        bool tlb_full=true;
        for(int j=0; j<TLB_SIZE; j++){
            if(TLB[j][0]==-1){
                tlb_full=false;
                TLB[j][0] = page_num;
                TLB[j][1] = PAGE_TABLE[page_num];
                TLB[j][2] = i;//set the timestamp
                break;
            }
        }

        //if tlb is full, then we have to evict the oldest item from the tlb
        short lru_index=0;
        if(tlb_full){
            for(int j=1; j<TLB_SIZE; j++){
                if(TLB[j][2]<TLB[lru_index][2]){
                    lru_index = j;//if timestamp is less than the current mim timestamp
                }
            }
            TLB[lru_index][0] = page_num;
            TLB[lru_index][1] = PAGE_TABLE[page_num];
            TLB[lru_index][2] = i;//set the timestamp
        }
                // fprintf(result_file, "Virtual address: %d Physical address: %d\n Signed Byte Value: %d", address, FRAME_SIZE*TLB[j][1]+page_offset, PHYS_MEM[FRAME_SIZE*TLB[j][1]][page_offset]);

        fprintf(result_file, "Virtual address: %d Physical address: %d Signed Byte Value: %d\n", address, FRAME_SIZE*PAGE_TABLE[page_num]+page_offset,  PHYS_MEM[PAGE_TABLE[page_num]][page_offset]);
        i++;
    }

    fclose(result_file);

    fclose(add_file);


    float pf_rate = (float)page_faults/(float)(i);
    float tlb_hit_rate = (float)tlb_hits/(float)(i);

    printf("Page Faults: %d, TLB Hits: %d, Page Miss Rate: %.3f, TLB Hit Rate: %.3f\n", page_faults, tlb_hits, pf_rate, tlb_hit_rate);
}