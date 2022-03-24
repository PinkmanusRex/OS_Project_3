#include "my_vm.h"
#include "rw_lock.h"

#define log2(X) __log_base2(X)

/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating
    void *physical_mem_ptr = malloc(MEMSIZE);
    offset_bits = (char) log2(PGSIZE);
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
}


/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */
int
add_TLB(void *va, void *pa)
{

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

    /* Part 2: TLB lookup code here */

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


    //If translation not successful, then return NULL
    return NULL; 
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int
page_map(pde_t *pgdir, void *va, void *pa)
{

    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */

    return -1;
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

unsigned long __get_fit(char *bitmap, unsigned long bitmap_length, unsigned long no_pages) {
    return __get_first_fit(bitmap, bitmap_length, no_pages);
}

unsigned long __get_first_fit(char *bitmap, unsigned long bitmap_length, unsigned long no_pages) {
    for (unsigned long idx = 0; idx < bitmap_length; idx += 1UL) {
        if (!__get_bit_at_index(bitmap, idx)) {
            unsigned long rem_pages = no_pages - 1UL;
            unsigned long j = 0;
            for (j = idx + 1UL; j < bitmap_length && rem_pages; j += 1UL) {
                if (!__get_bit_at_index(bitmap, j)) {
                    rem_pages -= 1UL;
                } else {
                    break;
                }
            }
            if (!rem_pages) 
                return idx;
            else 
                idx = j;
        }
    }
    return -1;
}

void __set_bit_at_index(char *bitmap, unsigned long index) {
    unsigned long bitmap_index = index / 8UL;
    unsigned long offset = index % 8UL;
    bitmap[bitmap_index] = bitmap[bitmap_index] | (1 << (7 - offset));
    return;
}

int __get_bit_at_index(char *bitmap, unsigned long index) {
    unsigned long bitmap_index = index / 8UL;
    unsigned long offset = index % 8UL;
    return (bitmap[bitmap_index] >> (7 - offset)) & 1;
}

int __check_empty_table(pte_t *table, unsigned long no_entries) {
    for (unsigned long idx = 0UL; idx < no_entries; idx += 1UL) {
        //0UL is an invalid physical address (since malloc itself will never return 0 and the userspace reserves 0 as the NULL address
        //as such, if that physical address is 0UL, we can be sure that it is empty
        if (table[idx] != 0UL)
            return 0;
    }
    return 1;
}

int __check_empty_directory(pde_t *directory, unsigned long no_entries) {
    for (unsigned long idx = 0UL; idx < no_entries; idx += 1UL) {
        if (directory[idx] != 0UL)
            return 0;
    }
    return 1;
}

void __init_table(pte_t *table, unsigned long no_entries) {
    for (unsigned long idx = 0UL; idx < no_entries; idx += 1UL) {
        table[idx] = 0UL;
    }
}

void __init_directory(pde_t *directory, unsigned long no_entries) {
    for (unsigned long idx = 0UL; idx < no_entries; idx += 1UL) {
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
