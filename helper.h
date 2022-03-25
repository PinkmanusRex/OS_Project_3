#include "rw_lock.h"
#include "my_vm.h"

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
