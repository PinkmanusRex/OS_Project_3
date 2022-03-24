#ifndef MY_VM_H_INCLUDED
#define MY_VM_H_INCLUDED
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

//Assume the address space is 32 bits, so the max memory size is 4GB
//Page size is 4KB

#define SYSBITS 32

//Add any important includes here which you may need

#define PGSIZE 4096

// Maximum size of virtual memory
#define MAX_MEMSIZE 4ULL*1024*1024*1024

// Size of "physcial memory"
#define MEMSIZE 1024*1024*1024

// Represents a page table entry
typedef unsigned long pte_t;

// Represents a page directory entry
typedef unsigned long pde_t;

unsigned long p_base;

unsigned long p_offset;

unsigned long no_p_pages;

unsigned long no_v_pages;

char *physical_bitmap;

char *virtual_bitmap;

char offset_bits; //how many bits are reserved for the offset

char l1_bits;

char l2_bits;

#define TLB_ENTRIES 512

//Structure to represents TLB
struct tlb {
    /*Assume your TLB is a direct mapped TLB with number of entries as TLB_ENTRIES
    * Think about the size of each TLB entry that performs virtual to physical
    * address translation.
    */

};
struct tlb tlb_store;


void set_physical_mem();
pte_t* translate(pde_t *pgdir, void *va);
int page_map(pde_t *pgdir, void *va, void* pa);
bool check_in_tlb(void *va);
void put_in_tlb(void *va, void *pa);
void *t_malloc(unsigned int num_bytes);
void t_free(void *va, int size);
void put_value(void *va, void *val, int size);
void get_value(void *va, void *val, int size);
void mat_mult(void *mat1, void *mat2, int size, void *answer);
void print_TLB_missrate();

/** Helpers **/

/* returns the starting candidate page
 */
unsigned long __get_fit(char *bitmap, unsigned long bitmap_length, unsigned long no_pages);

unsigned long __get_first_fit(char *bitmap, unsigned long bitmap_length, unsigned long no_pages);

/* sets the bit at the index, with index going 0...end from left to right
 */
void __set_bit_at_index(char *bitmap, unsigned long index);

/* returns the bit at the index, with index going 0...end from left to right
 */
int __get_bit_at_index(char *bitmap, unsigned long index);

/* checks if the page table is empty
 * returns 1 for empty, 0 for non-empty
 */
int __check_empty_table(pte_t *table, unsigned long no_entries);

/* checks if the page directory is empty
 * returns 1 for empty, 0 for non-empty
 */
int __check_empty_directory(pde_t *directory, unsigned long no_entries);

/* initializes a table
 */
void __init_table(pte_t *table, unsigned long no_entries);

/* initializes a directory
 */
void __init_directory(pde_t *directory, unsigned long no_entries);

/* will increment the value of va as you write
 * va - the virtual address yet to be "written" to
 * source - the address of the next byte to write down 
 * destination - the address of the next byte to be written to 
 * rem_size - the remaining amount of bytes to be written down
 * len - how much space is between the dest and the end of the page
 */
void __write(unsigned long *va, unsigned long *source , unsigned long *destination, unsigned long *rem_size, unsigned long len);

char __log_base2(unsigned long long val);

#endif
