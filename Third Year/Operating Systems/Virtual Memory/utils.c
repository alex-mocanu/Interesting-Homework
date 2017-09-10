/**
 * Operating Systems 2017, Assignment 3
 *
 * Mocanu Alexandru
 * 331CB
 *
 */

#include "utils.h"

void init_pte(page_table_entry *pte, w_ptr_t start)
{
	pte->state = STATE_NOT_ALLOC;
	pte->prev_state = STATE_NOT_ALLOC;
	pte->dirty = FALSE;
	pte->protection = PROTECTION_NONE;
	pte->start = start;
	pte->frame = NULL;
}

void init_frame(char frame_type, page_table *pt, w_size_t index)
{
	if (frame_type == 0) {
		pt->ram_frames[index].index = index;
		pt->ram_frames[index].pte = NULL;
	} else {
		pt->swap_frames[index].index = index;
		pt->swap_frames[index].pte = NULL;
	}
}

page_table_entry *find_pte(w_ptr_t address, page_table *parent,
	int nr_procs, page_table *pts)
{
	int i, offset;

	for (i = 0; i < nr_procs; ++i) {
		offset = address - pts[i].handles.start;
		if (offset >= 0 && offset < pts[i].nr_pages * w_get_page_size())
			break;
	}

	if (i < nr_procs) {
		*parent = pts[i];
		offset /= w_get_page_size();
		return &pts[i].ptes[offset];
	}

	return NULL;
}

/*
 * Map page in RAM
 */
void ram_map(page_table_entry *pte, page_table *process, int pos)
{
	w_boolean_t err;
	char *rc;
	int page_size = w_get_page_size();

	DIE(munmap(pte->start, page_size) < 0, "munmap");
	rc = mmap(pte->start, page_size, PROT_NONE, MAP_SHARED,
		process->handles.ram_handle, pos * page_size);
	DIE(rc == MAP_FAILED, "mmap");
	/* Configure page table entry */
	pte->frame = &process->ram_frames[pos];
	process->ram_frames[pos].pte = pte;
	pte->protection = PROTECTION_READ;
	err = w_protect_mapping(pte->start, 1, pte->protection);
	DIE(err == FALSE, "mprotect");
	pte->prev_state = STATE_NOT_ALLOC;
	pte->state = STATE_IN_RAM;
}

/*
 * Swap out a page from RAM to a given swap frame
 */
void swap_out(page_table_entry *pte, page_table *process, int pos)
{
	w_boolean_t err;
	char *rc;
	int page_size = w_get_page_size();
	int i;
	char buf[page_size + 1], zeros[page_size + 1];
	w_handle_t ram = process->handles.ram_handle;
	w_handle_t swap = process->handles.swap_handle;

	for (i = 0; i <= page_size; ++i)
		zeros[i] = 0;

	/* Swap out page if it is either dirty or hasn't been swapped out */
	if (pte->dirty == TRUE || pte->prev_state == STATE_NOT_ALLOC) {
		err = w_set_file_pointer(swap, pos * page_size);
		DIE(err == FALSE, "lseek");
		err = w_set_file_pointer(ram, 0);
		DIE(err == FALSE, "lseek");
		err = w_read_file(ram, buf, page_size);
		DIE(err == FALSE, "read");
		err = w_set_file_pointer(ram, 0);
		DIE(err == FALSE, "lseek");
		/* Clear RAM page */
		err = w_write_file(ram, zeros, page_size);
		DIE(err == FALSE, "write");
		err = w_write_file(swap, buf, page_size);
		DIE(err == FALSE, "write");
	}

	/* Map RAM page in place o virtual memory page */
	DIE(munmap(pte->start, page_size) < 0, "munmap");
	rc = mmap(pte->start, page_size, PROT_NONE, MAP_SHARED, swap,
		pos * page_size);
	DIE(rc == MAP_FAILED, "mmap");
	pte->frame = &process->swap_frames[pos];
	process->swap_frames[pos].pte = pte;
	pte->protection = PROTECTION_NONE;
	err = w_protect_mapping(pte->start, 1, pte->protection);
	DIE(err == FALSE, "mprotect");
	pte->prev_state = STATE_IN_RAM;
	pte->state = STATE_IN_SWAP;
	/* As page is swapped out, it becomes clean */
	pte->dirty = FALSE;
}

/*
 * Swap in a page into RAM
 */
void swap_in(page_table_entry *pte, page_table *process)
{
	w_boolean_t err;
	int page_size = w_get_page_size();
	char *rc, buf[page_size + 1];
	w_handle_t ram = process->handles.ram_handle;
	w_handle_t swap = process->handles.swap_handle;

	err = w_set_file_pointer(swap, pte->frame->index * page_size);
	DIE(err == FALSE, "lseek");
	err = w_read_file(swap, buf, page_size);
	DIE(err == FALSE, "read");
	err = w_set_file_pointer(ram, 0);
	DIE(err == FALSE, "lseek");
	err = w_write_file(ram, buf, page_size);
	DIE(err == FALSE, "write");
	DIE(munmap(pte->start, page_size) < 0, "munmap");
	rc = mmap(pte->start, page_size, PROT_NONE, MAP_SHARED, ram, 0);
	DIE(rc == MAP_FAILED, "mmap");
	pte->protection = PROTECTION_READ;
	err = w_protect_mapping(pte->start, 1, pte->protection);
	DIE(err == FALSE, "mprotect");
	pte->frame->pte = NULL;
	pte->frame = &process->ram_frames[0];
	process->ram_frames[0].pte = pte;
	pte->prev_state = STATE_IN_SWAP;
	pte->state = STATE_IN_RAM;
}

void treat_page_fault(page_table_entry *pte, page_table *process)
{
	int i;
	w_boolean_t err;
	w_ptr_t start = pte->start;
	page_table_entry *aux_pte = process->ram_frames[0].pte;

	/* Page is not yet mapped */
	if (pte->state == STATE_NOT_ALLOC) {
		/* Try mapping page in RAM */
		for (i = 0; i < process->nr_frames; ++i)
			if (process->ram_frames[i].pte == NULL) {
				ram_map(pte, process, i);
				return;
			}
		/* Swap out a page to map the new one in RAM */
		for (i = 0; i < process->nr_pages; ++i)
			if (process->swap_frames[i].pte == NULL) {
				/* Do the swap-out */
				swap_out(aux_pte, process, i);
				/* Map page into RAM */
				ram_map(pte, process, 0);
				break;
			}
	}
	/* Page is in RAM */
	else if (pte->state == STATE_IN_RAM) {
		pte->protection = PROTECTION_WRITE;
		err = w_protect_mapping(start, 1, pte->protection);
		DIE(err == FALSE, "mprotect");
		pte->dirty = TRUE;
	}
	/* Page is in swap */
	else
		for (i = 0; i < process->nr_pages; ++i)
			if (process->swap_frames[i].pte == NULL) {
				/* Do the swap-out */
				swap_out(aux_pte, process, i);
				/* Do the swap-in */
				swap_in(pte, process);
				break;
			}
}
