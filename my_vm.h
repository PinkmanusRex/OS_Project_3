#ifndef MY_VM_H_INCLUDED
#define MY_VM_H_INCLUDED
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

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

#define TLB_ENTRIES 512

struct tlb_entry {
    char valid;
    unsigned long vpn;
    unsigned long pfn;
};

//Structure to represents TLB
struct tlb {
    /*Assume your TLB is a direct mapped TLB with number of entries as TLB_ENTRIES
    * Think about the size of each TLB entry that performs virtual to physical
    * address translation.
    */
    struct tlb_entry entries[TLB_ENTRIES];
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

/****************************************************************************************************/

struct rw_lock {
    unsigned int no_readers;
    unsigned int no_writers;
    unsigned int no_writing;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

extern char initialized;
extern pthread_mutex_t init_mtx;
extern pthread_cond_t init_cond;

extern struct rw_lock __tlb_rw_lock;
extern struct rw_lock __table_rw_lock;
extern struct rw_lock __physical_lock;

extern unsigned long l1_base;
extern unsigned long p_base;
extern unsigned long p_offset;
extern unsigned long no_p_pages;
extern unsigned long no_v_pages;

extern char offset_bits;

extern char *physical_bitmap;
extern char *virtual_bitmap;

//for use in clock replacement algorithm
extern unsigned int no_free_tlb;
extern unsigned int clock_hand;
extern char *tlb_bitmap;

extern unsigned int tlb_hits;
extern unsigned int tlb_misses;

extern char l1_bits;
extern char l2_bits;

void __init_rw_lock(struct rw_lock *rw);

void __destroy_rw_lock(struct rw_lock *rw);

/* a shared read lock */
void __lock_r_rw_lock(struct rw_lock *rw);

/* unregisters this read lock */
void __unlock_r_rw_lock(struct rw_lock *rw);

/* a mutually exclusive write lock */
void __lock_w_rw_lock(struct rw_lock *rw);

void __unlock_w_rw_lock(struct rw_lock *rw);

//get back the true physical address, which involves right shifting and appending back in the physical offset
unsigned long __unsanitized_p_addr(unsigned long pfn, unsigned long offset);

//assumes va has already had offset bits taken out
unsigned long __get_l1_idx(unsigned long va);
//assumes va has already had offset bits taken out
unsigned long __get_l2_idx(unsigned long va);

unsigned long __get_fit(char *bitmap, unsigned long bitmap_length, unsigned int no_pages, char *found);

unsigned long __get_first_fit(char *bitmap, unsigned long bitmap_length, unsigned int no_pages, char *found);

/* 0..end left to right */
void __set_bit_at_index(char *bitmap, unsigned long index);

void __unset_bit_at_index(char *bitmap, unsigned long index);

unsigned int __get_bit_at_index(char *bitmap, unsigned long index);

void __init_table(pte_t *table, unsigned int no_entries);

void __init_directory(pde_t *directory, unsigned int no_entries);

/*
 * va - the virtual address yet to be "written" to
 * source - the address of the next byte to write down
 * destination - the address of the next byte to be written to
 * rem_size - the remaining amount of bytes to be written down
 * len - the maximum number of bytes that can be written
 */
void __write(unsigned long *va, unsigned long *source, unsigned long *destination, unsigned long *rem_size, unsigned long len);

char __log_base2(unsigned long long val);

unsigned int __calc_nec_pages(unsigned int size);

unsigned long __insert_page_table();

unsigned long __insert_page_dir();

unsigned int __clock_replacement();

/* inclusive start and end */
unsigned int __valid_address(char *bitmap, unsigned long start_idx, unsigned long end_idx);

unsigned long __get_offset(unsigned long va);

void __remove_TLB(unsigned long va);
/****************************************************************************************************/
#endif
