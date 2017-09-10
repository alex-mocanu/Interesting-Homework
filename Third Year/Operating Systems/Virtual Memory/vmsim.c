/**
 * Operating Systems 2017, Assignment 3
 *
 * Mocanu Alexandru
 * 331CB
 *
 */

#include "vmsim.h"

/**
 * Page-fault handler
 */
static void segv_handler(int signum, siginfo_t *info, void *context)
{
	char *addr;
	int page_offset;
	page_table_entry *pte;
	page_table parent;

	if (signum == SIGSEGV) {
		addr = info->si_addr;
		page_offset = (unsigned long)addr % w_get_page_size();
		addr -= page_offset;
		/* Find page table entry */
		pte = find_pte((void *)addr, &parent, nr_procs, pts);
		if (pte == NULL)
			return;

		treat_page_fault(pte, &parent);
	}
}

w_boolean_t vmsim_init(void)
{
	return w_set_exception_handler(segv_handler);
}

w_boolean_t vmsim_cleanup(void)
{
	return w_remove_exception_handler(NULL);
}

w_boolean_t vm_alloc(w_size_t num_pages, w_size_t num_frames, vm_map_t *map)
{
	int rc, page_size = w_get_page_size();
	int i;
	void *vmem;
	char ram_file[10] = "ramXXXXXX";
	char swap_file[10] = "swpXXXXXX";
	w_handle_t ram, swap;

	/* Check if RAM is smaller than virtual memory */
	if (num_pages < num_frames)
		return FALSE;

	/* Create, open and truncate RAM file */
	ram = mkstemp(ram_file);
	if (ram < 0)
		return FALSE;
	rc = ftruncate(ram, num_frames * page_size);
	if (rc < 0)
		return FALSE;
	map->ram_handle = ram;

	/* Create, open and truncate swap file */
	swap = mkstemp(swap_file);
	if (swap < 0)
		return FALSE;
	rc = ftruncate(swap, num_pages * page_size);
	if (rc < 0)
		return FALSE;
	map->swap_handle = swap;

	/* Allocate virtual memory */
	vmem = mmap(0, num_pages * page_size, PROT_NONE,
		MAP_ANONYMOUS | MAP_SHARED, -1, 0);
	if (vmem == MAP_FAILED)
		return FALSE;
	map->start = vmem;

	/* Create page table */
	++nr_procs;
	/* first process */
	if (nr_procs == 1) {
		pts = malloc(sizeof(page_table));
		if (pts == NULL)
			return FALSE;
	} else {
		pts = realloc(pts, nr_procs * sizeof(page_table));
		if (pts == NULL)
			return FALSE;
	}
	/* Initialize page table */
	pts[nr_procs - 1].nr_pages = num_pages;
	pts[nr_procs - 1].nr_frames = num_frames;
	pts[nr_procs - 1].handles.start = map->start;
	pts[nr_procs - 1].handles.ram_handle = map->ram_handle;
	pts[nr_procs - 1].handles.swap_handle = map->swap_handle;
	memcpy(pts[nr_procs - 1].ram_file, ram_file, sizeof(ram_file));
	memcpy(pts[nr_procs - 1].swap_file, swap_file, sizeof(swap_file));
	/* Initialize page table entries */
	pts[nr_procs - 1].ptes = malloc(num_pages * sizeof(page_table_entry));
	if (pts[nr_procs - 1].ptes == NULL)
		return FALSE;

	for (i = 0; i < num_pages; ++i)
		init_pte(&pts[nr_procs - 1].ptes[i],
			map->start + i * page_size);
	/* Initialize ram frames */
	pts[nr_procs - 1].ram_frames = malloc(num_frames * sizeof(frame));
	if (pts[nr_procs - 1].ram_frames == NULL)
		return FALSE;
	for (i = 0; i < num_frames; ++i)
		init_frame(0, &pts[nr_procs - 1], i);
	/* Initialize swap frames */
	pts[nr_procs - 1].swap_frames = malloc(num_pages * sizeof(frame));
	if (pts[nr_procs - 1].swap_frames == NULL)
		return FALSE;
	for (i = 0; i < num_pages; ++i)
		init_frame(1, &pts[nr_procs - 1], i);
	return TRUE;
}

w_boolean_t vm_free(w_ptr_t start)
{
	int i;

	for (i = 0; i < nr_procs; ++i)
		if (start == pts[i].handles.start) {
			/* Deallocate page table memory */
			free(pts[i].ptes);
			free(pts[i].ram_frames);
			free(pts[i].swap_frames);
			/* Close ram and swap files */
			if (w_close_file(pts[i].handles.ram_handle) == FALSE)
				return FALSE;
			if (w_close_file(pts[i].handles.swap_handle) == FALSE)
				return FALSE;
			/* Delete ram and swap files */
			if (w_delete_file(pts[i].ram_file) == FALSE)
				return FALSE;
			if (w_delete_file(pts[i].swap_file) == FALSE)
				return FALSE;
			/* Unmap pages */
			if (munmap(pts[i].handles.start,
				pts[i].nr_pages * w_get_page_size()) < 0)
				return FALSE;
			return TRUE;
		}

	return FALSE;
}
