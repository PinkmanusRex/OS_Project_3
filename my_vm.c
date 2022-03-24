#include "helper.h"

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
unsigned long p_offset = 0UL;

unsigned long no_p_pages = 0;
unsigned long no_v_pages = 0;

char offset_bits = 0;

char *physical_bitmap = 0;
char *virtual_bitmap = 0;

char l1_bits = 0;
char l2_bits = 0;

/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating
    void *physical_mem_ptr = malloc(MEMSIZE);
    offset_bits = (char) log2(PGSIZE);
    //p_offset is since the physical memory might not be '0' aligned
    p_offset = ((unsigned long) physical_mem_ptr) & ((1UL << offset_bits) - 1UL);
    p_base = ((unsigned long) physical_mem_ptr) >> offset_bits;
    
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them

    no_p_pages = 1UL << ((char) log2(MEMSIZE) - offset_bits);
    no_v_pages = 1UL << (SYSBITS - offset_bits);

    l2_bits = (char) log2((PGSIZE/sizeof(pte_t)));
    l1_bits = SYSBITS - offset_bits - l2_bits;

    physical_bitmap = (char*) malloc((no_p_pages/8));
    virtual_bitmap = (char *) malloc((no_v_pages/8));

    //And also creating the root page directory and reserving the first necessary amount of physical bits

    l1_base = (p_base << offset_bits) + p_offset;
    unsigned int no_entries = 1U << l1_bits;
    __init_directory((pde_t *) l1_base, no_entries);

    unsigned int no_p_bits_to_reserve = __calc_nec_pages(no_entries*sizeof(pde_t));
    for (unsigned int idx = 0U; idx < no_p_bits_to_reserve; idx += 1U)
        __set_bit_at_index(physical_bitmap, idx);
}


/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */
int
add_TLB(void *va, void *pa)
{
    __initialization_check();

    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */

    return -1;
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_TLB(void *va) {

    __initialization_check();

    /* Part 2: TLB lookup code here */

}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    __initialization_check();

    double miss_rate = 0;	

    /*Part 2 Code here to calculate and print the TLB miss rate*/




    fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
}



/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t *translate(pde_t *pgdir, void *va) {

    __initialization_check();

    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    *
    * Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */


    //If translation not successful, then return NULL
    return NULL; 
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
//assuming that va and pa have the offset removed already
int
page_map(pde_t *pgdir, void *va, void *pa)
{
    __lock_w_rw_lock(&__table_rw_lock);
    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */
    unsigned long l1_idx = ((unsigned long) va >> l2_bits);
    if (pgdir[l1_idx] == 0UL){
        __lock_w_rw_lock(&__physical_lock);
        unsigned long table_addr = __insert_page_table();
        __unlock_w_rw_lock(&__physical_lock);
        pgdir[l1_idx] = table_addr;
    }
    unsigned long l2_idx = ((unsigned long) va & ((1UL << l2_bits) - 1UL));
    pte_t *pgtable = (pte_t *) pgdir[l1_idx];
    pgtable[l2_idx] = (unsigned long) pa;
    __unlock_w_rw_lock(&__table_rw_lock);
    return 0;
}


/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) {
 
    //Use virtual address bitmap to find the next free page
}


/* Function responsible for allocating pages
and used by the benchmark
*/
void *t_malloc(unsigned int num_bytes) {

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */

    pthread_mutex_lock(&init_mtx);
    if (!initialized) {
        set_physical_mem();
        initialized = 1;
        pthread_cond_broadcast(&init_cond);
    }
    pthread_mutex_unlock(&init_mtx);

   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */

    return NULL;
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
    
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
 * The function returns 0 if the put is successfull and -1 otherwise.
*/
void put_value(void *va, void *val, int size) {

    __initialization_check();

    /* HINT: Using the virtual address and translate(), find the physical page. Copy
     * the contents of "val" to a physical page. NOTE: The "size" value can be larger 
     * than one page. Therefore, you may have to find multiple pages using translate()
     * function.
     */

    unsigned long *virtual_addr = 0;
    *virtual_addr = (unsigned long) va;
    unsigned long *source = 0;
    *source = (unsigned long) val;
    unsigned long *rem_size = 0;
    *rem_size = size;
}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {

    __initialization_check();

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */

    unsigned long *virtual_addr = 0;
    *virtual_addr = (unsigned long) va;
    unsigned long *destination = 0;
    *destination = (unsigned long) val;
    unsigned long *rem_size = 0;
    *rem_size = size;
}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void mat_mult(void *mat1, void *mat2, int size, void *answer) {

    __initialization_check();

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
                // printf("Values at the index: %d, %d, %d, %d, %d\n", 
                //     a, b, size, (i * size + k), (k * size + j));
                c += (a * b);
            }
            int address_c = (unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int));
            // printf("This is the c: %d, address: %x!\n", c, address_c);
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

unsigned int __get_bit_at_index(char *bitmap, unsigned long index) {
    unsigned long bitmap_index = index / 8UL;
    unsigned long offset = index % 8UL;
    return (bitmap[bitmap_index] >> (7 - offset)) & 1;
}

unsigned int __check_empty_table(pte_t *table, unsigned int no_entries) {
    for (unsigned int idx = 0U; idx < no_entries; idx += 1U) {
        //0UL is an invalid physical address (since malloc itself will never return 0 and the userspace reserves 0 as the NULL address
        //as such, if that physical address is 0UL, we can be sure that it is empty
        if (table[idx] != 0UL)
            return 0;
    }
    return 1;
}

unsigned int __check_empty_directory(pde_t *directory, unsigned int no_entries) {
    for (unsigned int idx = 0U; idx < no_entries; idx += 1U) {
        if (directory[idx] != 0UL)
            return 0;
    }
    return 1;
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

void __write(unsigned long *va, unsigned long *source, unsigned long *destination, unsigned long *rem_size, unsigned long len) {
    char *src = 0;
    char *dest = 0;
    for (unsigned long i = 0UL; i < len && rem_size; i += 1UL) {
        src = (char *) *source;
        dest = (char *) *destination;
        *dest = *src;
        *va += 1UL; //the virtual address advances one byte
        *source += 1UL; //advance the *source pointer
        *destination += 1UL; //advance the destination pointer
        *rem_size -= 1UL; //one less byte to write
    }
}

char __log_base2(unsigned long long val) {
    char bits = 0;
    unsigned long long val2 = val;
    while (val2 > 0) {
        val2 >>= 1UL;
        bits += 1;
    }
    if (val % 2 != 0)
        return bits;
    return bits - 1;
}

unsigned int __calc_nec_pages(unsigned int size) {
    unsigned int no_pages = size / PGSIZE;
    if (size % PGSIZE != 0U)
        no_pages += 1U;
    return no_pages;
}

unsigned long __insert_page_table() {
    char found = 0;
    unsigned long p_page_idx = __get_fit(physical_bitmap, no_p_pages, 1U, &found);
    if (!found) {
        printf("can't fit table into physical memory\n");
        exit(EXIT_FAILURE);
    }
    __set_bit_at_index(physical_bitmap, p_page_idx);
    unsigned long addr = ((p_base + p_page_idx) << offset_bits) | p_offset;
    __init_table((pte_t *) addr, (1U << l2_bits));
    return (p_base + p_page_idx);
}
