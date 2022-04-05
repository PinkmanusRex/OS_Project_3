#include "my_vm.h"

#define log2(X) __log_base2(X)

/***initialization usage***/

char initialized = 0; //1 - true, 0 - false
pthread_mutex_t init_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t init_cond = PTHREAD_COND_INITIALIZER;

void __initialization_check() {
    pthread_mutex_lock(&init_mtx);
    while (!initialized)
        pthread_cond_wait(&init_cond, &init_mtx);
    pthread_mutex_unlock(&init_mtx);
}

/*************************/

struct rw_lock __tlb_rw_lock = { .no_readers = 0, .no_writers = 0, .no_writing = 0, .lock = PTHREAD_MUTEX_INITIALIZER, .cond = PTHREAD_COND_INITIALIZER};

struct rw_lock __table_rw_lock = { .no_readers = 0, .no_writers = 0, .no_writing = 0, .lock = PTHREAD_MUTEX_INITIALIZER, .cond = PTHREAD_COND_INITIALIZER};

struct rw_lock __physical_rw_lock = { .no_readers = 0, .no_writers = 0, .no_writing = 0, .lock = PTHREAD_MUTEX_INITIALIZER, .cond = PTHREAD_COND_INITIALIZER};

unsigned long l1_base = 0UL; //the base physical address of the l1 table (root table)
unsigned long p_base = 0UL;

unsigned long no_p_pages = 0;
unsigned long no_v_pages = 0;

char offset_bits = 0;

char *physical_bitmap = 0;
char *virtual_bitmap = 0;

unsigned int no_free_tlb = TLB_ENTRIES;
unsigned int clock_hand = 0;
char *tlb_bitmap = 0;

unsigned int tlb_hits;
unsigned int tlb_misses;

char l1_bits = 0;
char l2_bits = 0;

unsigned long insert_page_table_helper_idx = 0UL;

/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {
    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating
    p_base = (unsigned long) malloc(MEMSIZE);
    offset_bits = (char) log2(PGSIZE);
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them
    no_p_pages = 1UL << ((char) log2(MEMSIZE) - offset_bits);
    no_v_pages = 1UL << (SYSBITS - offset_bits);
    l2_bits = (char) log2((PGSIZE/sizeof(pte_t)));
    l1_bits = SYSBITS - offset_bits - l2_bits;
    /* how many bytes to allocate for the bitmaps*/
    unsigned long no_char_p = no_p_pages/8UL;
    if (no_p_pages % 8UL != 0UL) //but must be sure to keep in mind possible non-zero remainder. that needs a byte allocated as well
        no_char_p += 1UL;
    unsigned long no_char_v = no_v_pages/8UL;
    if (no_v_pages % 8UL != 0UL)
        no_char_v += 1UL;
    physical_bitmap = (char*) malloc(no_char_p);
    virtual_bitmap = (char *) malloc(no_char_v);
    //And also creating the root page directory and reserving the first necessary amount of physical bits
    l1_base = 0UL; //takes the 0th pfn by default
    unsigned int no_entries = 1U << l1_bits;
    __init_directory((pde_t *) __unsanitized_p_addr(l1_base, 0UL), no_entries);
    unsigned int no_pfn_to_reserve = __calc_nec_pages(no_entries*sizeof(pde_t));
    for (unsigned int idx = 0U; idx < no_pfn_to_reserve; idx += 1U)
        __set_bit_at_index(physical_bitmap, idx);
    unsigned int no_char_tlb = TLB_ENTRIES/8;
    if (TLB_ENTRIES % 8 != 0)
        no_char_tlb += 1U;
    tlb_bitmap = (char *) malloc(no_char_tlb);
    for (unsigned int i = 0; i < TLB_ENTRIES; i += 1U)
        tlb_store.entries[i].valid = 0;
}

/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */
int
add_TLB(void *vpn, void *pfn)
{
    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */
    unsigned int candidate = 0U;
    if (!no_free_tlb) {
        candidate = __clock_replacement();
    } else {
        for (unsigned int i = 0; i < TLB_ENTRIES; i += 1U) {
            if (!tlb_store.entries[i].valid) {
                candidate = i;
                no_free_tlb -= 1U;
                break;
            }
        }
    }
    tlb_store.entries[candidate].vpn = (unsigned long) vpn;
    tlb_store.entries[candidate].pfn = (unsigned long) pfn;
    tlb_store.entries[candidate].valid = 1;
    __set_bit_at_index(tlb_bitmap, candidate);
    return 0;
}

/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_TLB(void *vpn) {

    pte_t *pfn = 0UL;
    /* Part 2: TLB lookup code here */
    for (unsigned int i = 0; i < TLB_ENTRIES; i += 1U) {
        if(tlb_store.entries[i].valid) {
            if (tlb_store.entries[i].vpn == (unsigned long) vpn) {
                __set_bit_at_index(tlb_bitmap, i);
                pfn = (pte_t *) tlb_store.entries[i].pfn;
            }
        }
    }
    return pfn;
}

/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    double miss_rate = 0;	
    /*Part 2 Code here to calculate and print the TLB miss rate*/
    miss_rate = (double) tlb_misses / (tlb_hits + tlb_misses);
    fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
}

/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t *translate(pde_t *pgdir, void *va) {

    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    *
    * Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */

    if (!__valid_address(virtual_bitmap, ((unsigned long) va >> offset_bits), ((unsigned long) va) >> offset_bits)) {
        printf("can't translate memory never allocated\n");
        exit(EXIT_FAILURE);
    }
    pte_t pfn = 0UL;
    __lock_r_rw_lock(&__tlb_rw_lock); /* read lock */
    while (1) {
        pfn = (pte_t) check_TLB(va);
        if (pfn != 0UL) {
            tlb_hits += 1U;
            break;
        } else {
            tlb_misses += 1U;
            unsigned long l1_idx = __get_l1_idx((unsigned long) va);
            pte_t *l2_table = (pte_t *) __unsanitized_p_addr(pgdir[l1_idx], 0UL);
            unsigned long l2_idx = __get_l2_idx((unsigned long) va);
            unsigned long pfn = l2_table[l2_idx];
            __unlock_r_rw_lock(&__tlb_rw_lock); //need to unlock the read lock so that the writing into the TLB can work
            __lock_w_rw_lock(&__tlb_rw_lock); /* write lock */
            add_TLB(va, (void *) pfn);
            __unlock_w_rw_lock(&__tlb_rw_lock); /* write unlock */
            __lock_r_rw_lock(&__tlb_rw_lock); //read lock
        }
    }
    __unlock_r_rw_lock(&__tlb_rw_lock);
    return (pte_t *) pfn; 
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int
page_map(pde_t *pgdir, void *va, void *pfn)
{
    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */
    unsigned long l1_idx = __get_l1_idx((unsigned long) va);
    if (pgdir[l1_idx] == 0UL){ //can't be 0UL since the root page directory would have taken that as its base pfn already
        unsigned long table_pfn = __insert_page_table();
        pgdir[l1_idx] = table_pfn;
    }
    unsigned long l2_idx = __get_l2_idx((unsigned long) va);
    pte_t *pgtable = (pte_t *) __unsanitized_p_addr(pgdir[l1_idx], 0UL);
    if (pgtable[l2_idx] == 0UL)
        pgtable[l2_idx] = (unsigned long) pfn;
    return 0;
}


/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) {

    //Use virtual address bitmap to find the next free page
    char found = 0;
    unsigned long candidate = __get_fit(virtual_bitmap, no_v_pages, num_pages, &found);
    if (!found) {
        printf("Couldn't find enough virtual pages\n");
        candidate = 0UL;
    } else {
        for (unsigned int i = 0; i < num_pages; i += 1U)
            __set_bit_at_index(virtual_bitmap, candidate + i);
        candidate += 1UL;
    }
    return (void *) candidate;
}


/* Function responsible for allocating pages
and used by the benchmark
*/
void *t_malloc(unsigned int num_bytes) {

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */

    void *addr = 0;

    pthread_mutex_lock(&init_mtx);
    if (!initialized) {
        set_physical_mem();
        addr = __t_malloc_subroutine(num_bytes);
        initialized = 1;
        pthread_cond_broadcast(&init_cond);
    } else {
        addr = __t_malloc_subroutine(num_bytes);
    }
    pthread_mutex_unlock(&init_mtx);
    
    return addr;

}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void t_free(void *va, int size) {

    __initialization_check();

    /* Part 1: Free the page table entries starting from this virtual address
     * (va). Also mark the pages free in the bitmap. Perform free only if the 
     * memory from "va" to va+size is valid.
     *
     * Part 2: Also, remove the translation from the TLB
     */

    __lock_w_rw_lock(&__table_rw_lock); //write lock virtual address space
    unsigned long start_vpn = (((unsigned long) va) - 1UL) >> offset_bits;
    unsigned long end_vpn = ((((unsigned long) va) - 1UL) + (size - 1)) >> offset_bits;
    if (!__valid_address(virtual_bitmap, start_vpn, end_vpn)) {
        printf("invalid address or size given to free\n");
        __unlock_w_rw_lock(&__table_rw_lock); //write unlock virtual address space
        return;
    }
    pde_t *l1_dir = (pde_t *) __unsanitized_p_addr(l1_base, 0UL);
    for (unsigned long vpn_tracker = start_vpn; vpn_tracker <= end_vpn; vpn_tracker += 1UL) {
        __unset_bit_at_index(virtual_bitmap, vpn_tracker); //mark availability in virtual bitmap
        unsigned long l1_idx = __get_l1_idx(vpn_tracker << offset_bits);
        pte_t *l2_tab = (pte_t *) __unsanitized_p_addr(l1_dir[l1_idx], 0UL);
        unsigned long l2_idx = __get_l2_idx(vpn_tracker << offset_bits);
        unsigned long pfn = l2_tab[l2_idx];
        l2_tab[l2_idx] = 0UL;
        __lock_w_rw_lock(&__physical_rw_lock); //physical write lock
        __unset_bit_at_index(physical_bitmap, pfn); //mark availability in physical bitmap
        __unlock_w_rw_lock(&__physical_rw_lock); //physical write unlock
        __remove_TLB(vpn_tracker);
    }
    __unlock_w_rw_lock(&__table_rw_lock); //write unlock virtual address space

}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
 * The function returns 0 if the put is successfull and -1 otherwise.
 *
*/
void put_value(void *va, void *val, int size) {

    __initialization_check();

    /* HINT: Using the virtual address and translate(), find the physical page. Copy
     * the contents of "val" to a physical page. NOTE: The "size" value can be larger 
     * than one page. Therefore, you may have to find multiple pages using translate()
     * function.
     */

    __lock_w_rw_lock(&__table_rw_lock); //write lock virtual address space
    unsigned long virtual_addr = ((unsigned long) va) - 1UL;
    unsigned long source = (unsigned long) val;
    unsigned long rem_size = size;

    unsigned long start_vpn = virtual_addr >> offset_bits;
    unsigned long end_vpn = (virtual_addr + (size - 1)) >> offset_bits;

    if (!__valid_address(virtual_bitmap, start_vpn, end_vpn)) {
        printf("invalid destination address (va) or size given\n");
        __unlock_w_rw_lock(&__table_rw_lock); //write unlock virtual address space
        return;
    }

    pde_t *l1_dir = (pde_t *) __unsanitized_p_addr(l1_base, 0UL);
    while (rem_size) {
        pte_t pfn = (pte_t) translate(l1_dir, (void *)virtual_addr);
        unsigned long destination = __unsanitized_p_addr(pfn, __get_offset(virtual_addr));
        /***** read to physical address *****/
        __lock_w_rw_lock(&__physical_rw_lock);
        unsigned int sz = __write(virtual_addr, source, destination, rem_size, PGSIZE - __get_offset(virtual_addr));
        rem_size -= sz;
        virtual_addr += sz;
        source += sz;
        __unlock_w_rw_lock(&__physical_rw_lock);
        /************************************/
    }
    __unlock_w_rw_lock(&__table_rw_lock); //write unlock virtual address space
}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {

    __initialization_check();

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */

    __lock_r_rw_lock(&__table_rw_lock); //read lock the virtual address space
    unsigned long virtual_addr = ((unsigned long) va) - 1UL;
    unsigned long destination = (unsigned long) val;
    unsigned long rem_size = size;
    unsigned long start_vpn = virtual_addr >> offset_bits;
    unsigned long end_vpn = (virtual_addr + (size - 1)) >> offset_bits;
    if (!__valid_address(virtual_bitmap, start_vpn, end_vpn)) {
        printf("invalid source address (va) or size given\n");
        __unlock_r_rw_lock(&__table_rw_lock); //read unlock the virtual address space
        return;
    }
    pde_t *l1_dir = (pde_t *) __unsanitized_p_addr(l1_base, 0UL);
    while (rem_size) {
        pte_t pfn = (pte_t) translate(l1_dir, (void *) virtual_addr);
        unsigned long source = __unsanitized_p_addr(pfn, __get_offset(virtual_addr));
        /***** read from physical address *****/
        __lock_r_rw_lock(&__physical_rw_lock);
        unsigned int sz =__write(virtual_addr, source, destination, rem_size, PGSIZE - __get_offset(virtual_addr));
        rem_size -= sz;
        virtual_addr += sz;
        destination += sz;
        __unlock_r_rw_lock(&__physical_rw_lock);
        /**************************************/
    }
    __unlock_r_rw_lock(&__table_rw_lock); //read unlock the virtual address space
}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void mat_mult(void *mat1, void *mat2, int size, void *answer) {

    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
     * matrix accessed. Similar to the code in test.c, you will use get_value() to
     * load each element and perform multiplication. Take a look at test.c! In addition to 
     * getting the values from two matrices, you will perform multiplication and 
     * store the result to the "answer array"
     */
    int x, y, val_size = sizeof(int);
    int i, j, k;
    for (i = 0; i < size; i++) {
        for(j = 0; j < size; j++) {
            unsigned int a, b, c = 0;
            for (k = 0; k < size; k++) {
                int address_a = (unsigned int)mat1 + ((i * size * sizeof(int))) + (k * sizeof(int));
                int address_b = (unsigned int)mat2 + ((k * size * sizeof(int))) + (j * sizeof(int));
                get_value( (void *)address_a, &a, sizeof(int));
                get_value( (void *)address_b, &b, sizeof(int));
                //printf("Values at the index: %d, %d, %d, %d, %d\n", 
                //   a, b, size, (i * size + k), (k * size + j));
                c += (a * b);
            }
            int address_c = (unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int));
            //printf("This is the c: %d, address: %x!\n", c, address_c);
            put_value((void *)address_c, (void *)&c, sizeof(int));
        }
    }
}

unsigned long __get_fit(char *bitmap, unsigned long bitmap_length, unsigned int no_pages, char *found) {
    return __get_first_fit(bitmap, bitmap_length, no_pages, found);
}

unsigned long __get_first_fit(char *bitmap, unsigned long bitmap_length, unsigned int no_pages, char *found) {
    for (unsigned long idx = 0; idx < bitmap_length; idx += 1UL) {
        if (!__get_bit_at_index(bitmap, idx)) {
            unsigned int rem_pages = no_pages - 1U;
            unsigned long j = 0;
            for (j = idx + 1UL; j < bitmap_length && rem_pages; j += 1UL) {
                if (!__get_bit_at_index(bitmap, j)) {
                    rem_pages -= 1U;
                } else {
                    break;
                }
            }
            if (!rem_pages) {
                *found = 1;
                return idx;
            } else 
                idx = j;
        }
    }
    *found = 0;
    return 0UL;
}

void __set_bit_at_index(char *bitmap, unsigned long index) {
    unsigned long bitmap_index = index / 8UL;
    unsigned long offset = index % 8UL;
    bitmap[bitmap_index] = bitmap[bitmap_index] | (1 << (7 - offset));
    return;
}

void __unset_bit_at_index(char *bitmap, unsigned long index) {
    unsigned long bitmap_index = index / 8UL;
    unsigned long offset = index % 8UL;
    bitmap[bitmap_index] = bitmap[bitmap_index] & ~(1 << (7 - offset));
    return;
}

unsigned int __get_bit_at_index(char *bitmap, unsigned long index) {
    unsigned long bitmap_index = index / 8UL;
    unsigned long offset = index % 8UL;
    return (bitmap[bitmap_index] >> (7 - offset)) & 1;
}

void __init_table(pte_t *table, unsigned int no_entries) {
    for (unsigned long idx = 0U; idx < no_entries; idx += 1U) {
        table[idx] = 0UL;
    }
}

void __init_directory(pde_t *directory, unsigned int no_entries) {
    for (unsigned long idx = 0U; idx < no_entries; idx += 1U) {
        directory[idx] = 0UL;
    }
}

unsigned int __write(unsigned long va, unsigned long source, unsigned long destination, unsigned long rem_size, unsigned long len) {
    unsigned long sz_to_write = (rem_size <= len) ? rem_size : len;
    memcpy((void *) destination, (void *) source, sz_to_write);
    return sz_to_write;
}

char __log_base2(unsigned long long val) {
    char bits = 0;
    unsigned long long val2 = val;
    while (val2 > 0) {
        val2 >>= 1UL;
        bits += 1;
    }
    return bits - 1;
}

unsigned long __calc_nec_pages(unsigned long size) {
    unsigned long no_pages = size / PGSIZE;
    if (size % PGSIZE != 0UL)
        no_pages += 1UL;
    return no_pages;
}

unsigned long __insert_page_table() {
    char found = 0;
    unsigned long pfn = 0UL;
    for (unsigned long i = insert_page_table_helper_idx; i < no_v_pages; i += 1UL) {
        if (!__get_bit_at_index(physical_bitmap, i)) {
            found = 1;
            pfn = i;
            break;
        }
    }
    if (!found) {
        printf("can't fit table into physical memory\n");
        exit(EXIT_FAILURE);
    }
    __set_bit_at_index(physical_bitmap, pfn);
    unsigned long addr = __unsanitized_p_addr(pfn, 0UL);
    __init_table((pte_t *) addr, (1U << l2_bits));
    return pfn;
}

unsigned int __clock_replacement() {
    unsigned int candidate = 0U;
    while (1) {
        if (clock_hand == TLB_ENTRIES)
            clock_hand = 0;
        if (__get_bit_at_index(tlb_bitmap, clock_hand)) {
            __unset_bit_at_index(tlb_bitmap, clock_hand);
            clock_hand += 1U;
        } else {
            candidate = clock_hand;
            clock_hand += 1U;
            break;
        }
    }
    return candidate;
}

unsigned long __unsanitized_p_addr(unsigned long pfn, unsigned long offset) {
    return p_base + (pfn << offset_bits) + offset;
}

unsigned long __get_l1_idx(unsigned long va) {
    return (va >> (l2_bits + offset_bits));
}

unsigned long __get_l2_idx(unsigned long va) {
    return (va & ((1UL << (l2_bits + offset_bits)) - 1UL)) >> offset_bits;
}

unsigned int __valid_address(char *bitmap, unsigned long start_idx, unsigned long end_idx) {
    for (unsigned long i = start_idx; i <= end_idx; i += 1UL) {
        if (!__get_bit_at_index(bitmap, i))
            return 0;
    }
    return 1;
}

unsigned long __get_offset(unsigned long va) {
    return (va & ((1UL << offset_bits) - 1UL));
}

void __remove_TLB(unsigned long va) {
    __lock_w_rw_lock(&__tlb_rw_lock);
    struct tlb_entry *entries = tlb_store.entries;
    for (unsigned int i = 0; i < TLB_ENTRIES; i += 1U) {
        if (entries[i].valid) {
            if (entries[i].vpn == va) {
                entries[i].valid = 0;
                __unset_bit_at_index(tlb_bitmap, i);
                tlb_hits += 1U;
                no_free_tlb += 1U;
                __unlock_w_rw_lock(&__tlb_rw_lock);
                return;
            }
        }
    }
    tlb_misses += 1U;
    __unlock_w_rw_lock(&__tlb_rw_lock);
}

void __lock_r_rw_lock(struct rw_lock *rw) {
    pthread_mutex_lock(&rw->lock);
    while (rw->no_writers)
        pthread_cond_wait(&rw->cond, &rw->lock);
    rw->no_readers += 1U;
    pthread_mutex_unlock(&rw->lock);
}

void __unlock_r_rw_lock(struct rw_lock *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->no_readers -= 1U;
    pthread_cond_broadcast(&rw->cond);
    pthread_mutex_unlock(&rw->lock);
}

void __lock_w_rw_lock(struct rw_lock *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->no_writers += 1U;
    while (rw->no_writing || rw->no_readers)
        pthread_cond_wait(&rw->cond, &rw->lock);
    rw->no_writing = 1U;
    pthread_mutex_unlock(&rw->lock);
}

void __unlock_w_rw_lock(struct rw_lock *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->no_writing = 0;
    rw->no_writers -= 1U;
    pthread_cond_broadcast(&rw->cond);
    pthread_mutex_unlock(&rw->lock);
}

void *__t_malloc_subroutine(unsigned int num_bytes) {
    __lock_w_rw_lock(&__table_rw_lock); //write lock virtual address space

    unsigned int no_pages = num_bytes / PGSIZE;
    if (num_bytes % PGSIZE != 0U)
        no_pages += 1U;

    unsigned long start_vpn = (unsigned long) get_next_avail(no_pages);
    if (start_vpn == 0UL) {
        __unlock_w_rw_lock(&__table_rw_lock); //write unlock virtual address space
        return (void *) 0UL;
    }
    start_vpn -= 1UL;

    __lock_w_rw_lock(&__physical_rw_lock); //physical write lock
    if (!__find_if_enough_physical_pages(no_pages)) {
        __unlock_w_rw_lock(&__physical_rw_lock); //physical write unlock
        for (unsigned int i = 0; i < no_pages; i += 1U)
            __unset_bit_at_index(virtual_bitmap, start_vpn + i);
        __unlock_w_rw_lock(&__table_rw_lock); //write unlock virtual address
        return (void *) 0UL;
    }

    pde_t *l1_dir = (pde_t *) __unsanitized_p_addr(l1_base, 0UL);
    for (unsigned int i = 0; i < no_pages; i += 1U) {
        /** finding and marking the physical page **/
        char found = 0;
        unsigned long pfn = __get_fit(physical_bitmap, no_p_pages, 1U, &found);
        __set_bit_at_index(physical_bitmap, pfn);
        /******** mapping the vpn to the pfn *******/
        page_map(l1_dir, (void *)((start_vpn + i) << offset_bits), (void *)pfn);
        /*******************************************/
    }
    __unlock_w_rw_lock(&__physical_rw_lock); //physical write unlock
    __unlock_w_rw_lock(&__table_rw_lock); //write unlock virtual address space
    return (void *)((start_vpn << offset_bits) + 1UL);
}

int __find_if_enough_physical_pages(unsigned int no_pages) {
    unsigned int count = 0U;
    for (unsigned long i = 0UL; i < no_p_pages; i += 1UL) {
        if (!__get_bit_at_index(physical_bitmap, i)) {
            count += 1U;
            if (count == no_pages) {
                insert_page_table_helper_idx = i + 1UL;
                break;
            }
        }
    }
    if (count != no_pages) {
        printf("Physical memory cannot fit request\n");
        return 0;
    }
    return 1;
}
