/**
 * Operating Systems 2017, Assignment 3
 *
 * Mocanu Alexandru
 * 331CB
 *
 */

#ifndef UTILS_H_
#define UTILS_H_
#include "helpers.h"
#include "common.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

/*
 * Initialize a page table entry with a given start address
 * @param pte - page table entry to be initialized
 * @param start - start address of the page described by the entry
 */
void init_pte(page_table_entry *pte, w_ptr_t start);

/*
 * Initialize a swap or RAM frame
 * @param frame_type - 0 for RAM, 1 for swap
 * @param pt - page table that contains the frame
 * @param index - number of the frame within the swap/RAM file
 */
void init_frame(char frame_type, page_table *pt, w_size_t index);

/*
 * Find the page table entry correspding to an address
 * @param address - address of the page table entry we are looking for
 * @param parent - page table that contains the page table entry (to be found)
 * @param nr_procs - number of active processes
 * @param pts - page tables
 */
page_table_entry *find_pte(w_ptr_t address, page_table *parent,
	int nr_procs, page_table *pts);

/*
 * Signal handling routine
 * @param pte - page table entry for the page that registered a page fault
 * @param process - page table of the process that contains the entry
 */
void treat_page_fault(page_table_entry *pte, page_table *process);

#endif
