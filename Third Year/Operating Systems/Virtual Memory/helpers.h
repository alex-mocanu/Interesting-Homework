/**
 * Operating Systems 2017, Assignment 3
 *
 * Mocanu Alexandru
 * 331CB
 *
 */

#ifndef HELPERS_H_
#define HELPERS_H_

#include "common.h"

/* virtual memory mapping encapsulation; initialized by vm_alloc */
typedef struct vm_map {
	w_ptr_t start;
	w_handle_t ram_handle;
	w_handle_t swap_handle;
} vm_map_t;

enum page_state {
	STATE_IN_RAM,
	STATE_IN_SWAP,
	STATE_NOT_ALLOC
};

struct frame;

/* handle pages (virtual pages) */
typedef struct {
	enum page_state state;
	enum page_state prev_state;
	w_boolean_t dirty;
	w_prot_t protection;
	w_ptr_t start;
	struct frame *frame;	/* NULL in case page is not mapped */
} page_table_entry;

/* handle frames (physical pages) */
typedef struct frame {
	w_size_t index;
	/* "backlink" to page_table_entry; NULL in case of free frame */
	page_table_entry *pte;
} frame;

/* page table */
typedef struct {
	int nr_pages;
	int nr_frames;
	vm_map_t handles;
	char ram_file[10];
	char swap_file[10];
	page_table_entry *ptes;
	frame *ram_frames;
	frame *swap_frames;
} page_table;

#endif
