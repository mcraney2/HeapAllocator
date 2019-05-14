////////////////////////////////////////////////////////////////////////////////
// Main File:        mem.c
// This File:        mem.c
// Other Files:      N/A
// Semester:         CS 354 Spring 2019
//
// Author:           Michael Connor Craney
// Email:            mcraney2@wisc.edu
// CS Login:         craney
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:         N/A
//
// Online sources:  N/A
//                  
////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "mem.h"

/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block but only containing size.
 */
typedef struct block_header {
        int size_status;
    /*
    * Size of the block is always a multiple of 8.
    * Size is stored in all block headers and free block footers.
    *
    * Status is stored only in headers using the two least significant bits.
    *   Bit0 => least significant bit, last bit
    *   Bit0 == 0 => free block
    *   Bit0 == 1 => allocated block
    *
    *   Bit1 => second last bit 
    *   Bit1 == 0 => previous block is free
    *   Bit1 == 1 => previous block is allocated
    * 
    * End Mark: 
    *  The end of the available memory is indicated using a size_status of 1.
    * 
    * Examples:
    * 
    * 1. Allocated block of size 24 bytes:
    *    Header:
    *      If the previous block is allocated, size_status should be 27
    *      If the previous block is free, size_status should be 25
    * 
    * 2. Free block of size 24 bytes:
    *    Header:
    *      If the previous block is allocated, size_status should be 26
    *      If the previous block is free, size_status should be 24
    *    Footer:
    *      size_status should be 24
    */
    


} block_header;         

/* Global variable - DO NOT CHANGE. It should always point to the first block,
 * i.e., the block at the lowest address.
 */

block_header *start_block = NULL;

/* 
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block on success.
 * Returns NULL on failure.
 * This function should:
 * - Check size - Return NULL if not positive or if larger than heap space.
 * - Determine block size rounding up to a multiple of 8 and possibly adding padding as a result.
 * - Use BEST-FIT PLACEMENT POLICY to find the block closest to the required block size
 * - Use SPLITTING to divide the chosen free block into two if it is too large.
 * - Update header(s) and footer as needed.
 * Tips: Be careful with pointer arithmetic.
 */
void* Alloc_Mem(int size) {
    int padding; // Keeps track of the padding that must be added onto the block
    int final_size; // The final size of the size to insert after padding and headers
    int is_block_free_mask = 0b0000000000000001; // Masks the is block free bit
    int is_previous_block_free_mask = 0b0000000000000010; // Masks the is previous block free bit
    int isBlockFree; // Contains status of if block is free
    int isPreviousBlockFree; // Contains status of if previous block is free
    int bestFitVal = 0; // Keeps track of the size closest to final_size
    block_header* best_fit = NULL; // Keeps track of the address of the closest space in size to final size
    block_header* holder_block = start_block; // Used as a holder block to step through the memory
    block_header* split_block; // Used as address holder for the split block for the allocation
    int size_stats; // Size status of the holder block w/ identifiers
    int sizeWithoutIdentifiers; // Size status of the holder block w/o identifiers
    int remaining; // Size of the space left over after the block has been split

    // Check size - Return NULL if not positive, if larger than heap space the loop will handle it
    if(size <= 0) {
	return NULL;
    }

    //printf("Size Passed In: %i\n", size);

    // Determine block size, rounding up to a multiple of 8 and possibly adding padding as a result
    padding = (size + sizeof(block_header))  % 8;
    if(padding != 0) {
    	final_size = size + sizeof(block_header) + (8 - padding);
    }
    else {
	final_size = size + sizeof(block_header);
    }

    // Use Best-Fit placement policy to find the block closest to the required block size
    while((holder_block->size_status) != 1) {
	size_stats = holder_block->size_status;
    	isBlockFree = (size_stats & is_block_free_mask);
	isPreviousBlockFree = (size_stats & is_previous_block_free_mask);
	sizeWithoutIdentifiers = size_stats - isPreviousBlockFree - isBlockFree;

	//printf("Size w/ Identifiers: %i\nIs Block Free: %i\nIs Prev Block Free %i\nSize w/o Identifiers: %i\nSize of Block to insert: %i\n", size_stats, isBlockFree, isPreviousBlockFree, sizeWithoutIdentifiers, final_size);

	// Case 1: Block isn't free
	if(isBlockFree == 1) {
		holder_block = holder_block + (sizeWithoutIdentifiers/sizeof(block_header));
	}

	// Case 2: Block is free + is the perfect size
	else if(final_size == sizeWithoutIdentifiers){
		best_fit = holder_block;
		best_fit->size_status = final_size + 3;
	       	return best_fit + 1;
	}	

	// Case 3: Block is free but isn't a perfect size, check if better then current best fit
	else {
		// Case 3A: Becomes new best fit since a block of right size hasn't been found yet
		if((best_fit == NULL) && (sizeWithoutIdentifiers > final_size)) {
			best_fit = holder_block;
			bestFitVal = sizeWithoutIdentifiers;	
		}

		// Case 3B: Becomes new best fit since a better size then current best fit
		if((bestFitVal > sizeWithoutIdentifiers) && (sizeWithoutIdentifiers > final_size)) {
			best_fit = holder_block;
			bestFitVal = sizeWithoutIdentifiers;
		}

		// Case 3C: Doesn't become new best fit since current best fit is a better size/Increment size regardless
		holder_block = holder_block + (sizeWithoutIdentifiers/sizeof(block_header));
	}
    }

    //printf("Start of Array: %8x\nStart of Newly Allocated Block: %8x\nReturned Pointer: %8x\n", (unsigned int)start_block, (unsigned int)best_fit, (unsigned int)(best_fit + 1));
        
    // If best fit is still NULL, no space for the block, return null
    if(best_fit == NULL) {
	return NULL;
    }
    
    // Use splitting to divide the choosen free block into two if it is too large
   
    // Split the block into the used and the remaining bits
    remaining = bestFitVal - final_size;

    //printf("Remaining Size for Free Block: %i\nFinal Size of Allocated Block: %i\n", remaining, final_size);

    // Step the size of the current block so the pointer is the beginning of free space
    split_block = best_fit + ((final_size)/sizeof(block_header));

    //printf("Pointer for Block to Split and Free: %8x\n", (unsigned int)(split_block));

    // Set the bits for the holder block which is the unallocated in the split
    split_block->size_status = remaining + 2;

    // Set the footer for the holder block 
    split_block = split_block + (remaining/sizeof(block_header)) - sizeof(block_header);
    split_block->size_status = remaining;

    // Update header of the newly allocated block
    best_fit->size_status = final_size + 3;	    

    return best_fit + 1;

}

/* 
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - USE IMMEDIATE COALESCING if one or both of the adjacent neighbors are free.
 * - Update header(s) and footer as needed.
 */                    
int Free_Mem(void *ptr) {
    int isPrevBlockFree; // Contains status of if the previous block is free
    int isNextBlockFree; // Contains status of if the next block is free
    int isBlockAllocated; // Contains status of block to be free'd if it is free
    int prevBlockSize; // Size of the previous block
    int combinedSize; // Used to set the size value of the coalesced block
    int nextBlockSize; // Size of the next block
    int isPreviousBlockFreeMask = 0b0000000000000010; // Acts as the mask for the is previous block free bit
    int isCurrentBlockFreeMask = 0b0000000000000001;  // Acts as the mask for the is current block free bit
    block_header* curr_block; // Pointer that holds address of the current block, or the block to free
    block_header* previous_block; // Pointer that holds address of the previous block
    block_header* next_block; // Pointer that holds the address of the next block
    block_header* footer;  // Pointer that holds the address of the footer
    int sizeOfBlockToFreeWithoutIdentifiers; // Size of the block to remove without identifiers
    int roughSize; // Size of the block to remove w/ identifiers

    // Check for a null pointer
    if(ptr == NULL) {
	return -1;
    }

    // Check for pointer that isn't a multiple of 8
    if((int)ptr % 8 != 0) {
	return -1;
    }
    
    // Points to start of payload, change it so it points to header
    curr_block = ptr - sizeof(block_header);
    roughSize = curr_block->size_status;

    // Check if pointer is already freed
    isBlockAllocated = (roughSize & isCurrentBlockFreeMask);
    if(isBlockAllocated == 0) {
	return -1;
    } 

    // Calculate need literals
    isPrevBlockFree = (roughSize & isPreviousBlockFreeMask);
    sizeOfBlockToFreeWithoutIdentifiers = roughSize - isPrevBlockFree - isCurrentBlockFreeMask;

    //printf("Size of Block to Remove w/o Identifiers: %i\n", sizeOfBlockToFreeWithoutIdentifiers);

    // Find the address of the next block/previous block and the footer for the current
    next_block = curr_block + (sizeOfBlockToFreeWithoutIdentifiers/sizeof(block_header));
    isNextBlockFree = (next_block->size_status & isCurrentBlockFreeMask);
    footer = next_block - 1;
    prevBlockSize = (curr_block - 1)->size_status;
    previous_block = curr_block - (prevBlockSize/sizeof(block_header));
    isPrevBlockFree = (previous_block->size_status & isCurrentBlockFreeMask);

    //printf("Pointer of Current Block: %8x\nPointer of Next Block: %8x\nPointer to Footer: %8x\nPointer to Previous Block: %8x\n", (unsigned int)(curr_block), (unsigned int)(next_block), (unsigned int)(footer), (unsigned int)(previous_block));


    // Free up block
    curr_block->size_status = roughSize & ~isCurrentBlockFreeMask;
    next_block->size_status = next_block->size_status & ~isPreviousBlockFreeMask;
    footer->size_status = sizeOfBlockToFreeWithoutIdentifiers;

    // Coalesscing

    nextBlockSize = next_block->size_status & ~isCurrentBlockFreeMask;
    combinedSize = sizeOfBlockToFreeWithoutIdentifiers; 

    // Attempt to Coalesce w/ Next Block
    if(!isNextBlockFree && next_block->size_status != 1) {
	combinedSize = nextBlockSize + combinedSize;   
	curr_block->size_status = combinedSize + 2;
	footer = footer + (nextBlockSize/sizeof(block_header));
	footer->size_status = combinedSize;

    }

    //Attempt to Coalesce w/ Previous Block
    if(!isPrevBlockFree && previous_block != curr_block) {
	combinedSize = combinedSize + prevBlockSize;
	previous_block->size_status = combinedSize + 2;
	footer->size_status = combinedSize;
    } 

    return 0;
}

/*
 * Function used to initialize the memory allocator.
 * Intended to be called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */                    
int Init_Mem(int sizeOfRegion) {         
    int pagesize;
    int padsize;
    int fd;
    int alloc_size;
    void* space_ptr;
    block_header* end_mark;
    static int allocated_once = 0;
  
    if (0 != allocated_once) {
        fprintf(stderr, 
        "Error:mem.c: Init_Mem has allocated space during a previous call\n");
        return -1;
    }
    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion 
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    alloc_size = sizeOfRegion + padsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    space_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, 
                    fd, 0);
    if (MAP_FAILED == space_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }
  
    allocated_once = 1;

    // for double word alignment and end mark
    alloc_size -= 8;

    // To begin with there is only one big free block
    // initialize heap so that start block meets 
    // double word alignement requirement
    start_block = (block_header*) space_ptr + 1;
    end_mark = (block_header*)((void*)start_block + alloc_size);
  
    // Setting up the header
    start_block->size_status = alloc_size;

    // Marking the previous block as used
    start_block->size_status += 2;

    // Setting up the end mark and marking it as used
    end_mark->size_status = 1;

    // Setting up the footer
    block_header *footer = (block_header*) ((char*)start_block + alloc_size - 4);
    footer->size_status = alloc_size;
  
    return 0;
}         
                 
/* 
 * Function to be used for DEBUGGING to help you visualize your heap structure.
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block 
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block as stored in the block header
 */                     
void Dump_Mem() {         
    int counter;
    char status[5];
    char p_status[5];
    char *t_begin = NULL;
    char *t_end = NULL;
    int t_size;

    block_header *current = start_block;
    counter = 1;

    int used_size = 0;
    int free_size = 0;
    int is_used = -1;

    fprintf(stdout, "************************************Block list***\
                    ********************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, "-------------------------------------------------\
                    --------------------------------\n");
  
    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;
    
        if (t_size & 1) {
            // LSB = 1 => used block
            strcpy(status, "used");
            is_used = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "Free");
            is_used = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "used");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "Free");
        }

        if (is_used) 
            used_size += t_size;
        else 
            free_size += t_size;

        t_end = t_begin + t_size - 1;
    
        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%d\n", counter, status, 
        p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);
    
        current = (block_header*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout, "---------------------------------------------------\
                    ------------------------------\n");
    fprintf(stdout, "***************************************************\
                    ******************************\n");
    fprintf(stdout, "Total used size = %d\n", used_size);
    fprintf(stdout, "Total free size = %d\n", free_size);
    fprintf(stdout, "Total size = %d\n", used_size + free_size);
    fprintf(stdout, "***************************************************\
                    ******************************\n");
    fflush(stdout);

    return;
}         


