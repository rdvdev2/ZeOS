/*
 * mm.c - Memory Management: Paging & segment memory management
 */

#include "mm_address.h"
#include <hardware.h>
#include <mm.h>
#include <sched.h>
#include <segment.h>
#include <types.h>

Byte phys_mem[TOTAL_PAGES];

/* SEGMENTATION */
/* Memory segements description table */
Descriptor *gdt = (Descriptor *)GDT_START;
/* Register pointing to the memory segments table */
Register gdtR;

/* PAGING */
/* Variables containing the page directory and the page table */

page_table_entry dir_pages[NR_TASKS][TOTAL_PAGES]
    __attribute__((__section__(".data.task")));

page_table_entry pagusr_table[NR_TASKS][TOTAL_PAGES]
    __attribute__((__section__(".data.task")));

/* TSS */
TSS tss;

/***********************************************/
/************** PAGING MANAGEMENT **************/
/***********************************************/

/* Init page table directory */

void init_dir_pages() {
  int i;

  for (i = 0; i < NR_TASKS; i++) {
    dir_pages[i][ENTRY_DIR_PAGES].entry = 0;
    dir_pages[i][ENTRY_DIR_PAGES].bits.pbase_addr =
        (((unsigned int)&pagusr_table[i]) >> 12);
    dir_pages[i][ENTRY_DIR_PAGES].bits.user = 1;
    dir_pages[i][ENTRY_DIR_PAGES].bits.rw = 1;
    dir_pages[i][ENTRY_DIR_PAGES].bits.present = 1;
  }
}

/* Initializes the page table (kernel pages only) */
void init_table_pages() {
  int i, j;
  /* reset all entries */
  for (j = 0; j < NR_TASKS; j++) {
    for (i = 0; i < TOTAL_PAGES; i++) {
      pagusr_table[j][i].entry = 0;
    }
    /* Init kernel pages */
    for (i = 1; i < NUM_PAG_KERNEL;
         i++) // Leave the page inaccessible to comply with NULL convention
    {
      // Logical page equal to physical page (frame)
      pagusr_table[j][i].bits.pbase_addr = i;
      pagusr_table[j][i].bits.rw = 1;
      pagusr_table[j][i].bits.present = 1;
    }
  }
}

/* Initialize pages for initial process (user pages) */
void set_user_pages(struct task_struct *task) {
  int pag;
  int new_ph_pag;
  page_table_entry *process_PT = get_PT(task);

  /* CODE */
  for (pag = 0; pag < NUM_PAG_CODE; pag++) {
    new_ph_pag = alloc_frame();
    process_PT[PAG_LOG_INIT_CODE + pag].entry = 0;
    process_PT[PAG_LOG_INIT_CODE + pag].bits.pbase_addr = new_ph_pag;
    process_PT[PAG_LOG_INIT_CODE + pag].bits.user = 1;
    process_PT[PAG_LOG_INIT_CODE + pag].bits.present = 1;
  }

  /* DATA */
  for (pag = 0; pag < NUM_PAG_DATA; pag++) {
    new_ph_pag = alloc_frame();
    process_PT[PAG_LOG_INIT_DATA + pag].entry = 0;
    process_PT[PAG_LOG_INIT_DATA + pag].bits.pbase_addr = new_ph_pag;
    process_PT[PAG_LOG_INIT_DATA + pag].bits.user = 1;
    process_PT[PAG_LOG_INIT_DATA + pag].bits.rw = 1;
    process_PT[PAG_LOG_INIT_DATA + pag].bits.present = 1;
  }
}

/* Writes on CR3 register producing a TLB flush */
void set_cr3(page_table_entry *dir) {
  asm volatile("movl %0,%%cr3" : : "r"(dir));
}

/* Macros for reading/writing the CR0 register, where is shown the paging status
 */
#define read_cr0()                                                             \
  ({                                                                           \
    unsigned int __dummy;                                                      \
    __asm__("movl %%cr0,%0\n\t" : "=r"(__dummy));                              \
    __dummy;                                                                   \
  })
#define write_cr0(x) __asm__("movl %0,%%cr0" : : "r"(x));

/* Enable paging, modifying the CR0 register */
void set_pe_flag() {
  unsigned int cr0 = read_cr0();
  cr0 |= 0x80000000;
  write_cr0(cr0);
}

/* Initializes paging for the system address space */
void init_mm() {
  init_table_pages();
  init_frames();
  init_dir_pages();
  allocate_DIR(&task[0].task);
  set_cr3(get_DIR(&task[0].task));
  set_pe_flag();
}
/***********************************************/
/************** SEGMENTATION MANAGEMENT ********/
/***********************************************/
void setGdt() {
  /* Configure TSS base address, that wasn't initialized */
  gdt[KERNEL_TSS >> 3].lowBase = lowWord((DWord) & (tss));
  gdt[KERNEL_TSS >> 3].midBase = midByte((DWord) & (tss));
  gdt[KERNEL_TSS >> 3].highBase = highByte((DWord) & (tss));

  gdtR.base = (DWord)gdt;
  gdtR.limit = 256 * sizeof(Descriptor);

  set_gdt_reg(&gdtR);
}

/***********************************************/
/************* TSS MANAGEMENT*******************/
/***********************************************/
void setTSS() {
  tss.PreviousTaskLink = NULL;
  tss.esp0 = INITIAL_ESP;
  tss.ss0 = __KERNEL_DS;
  tss.esp1 = NULL;
  tss.ss1 = NULL;
  tss.esp2 = NULL;
  tss.ss2 = NULL;
  tss.cr3 = NULL;
  tss.eip = 0;
  tss.eFlags = INITIAL_EFLAGS; /* Enable interrupts */
  tss.eax = NULL;
  tss.ecx = NULL;
  tss.edx = NULL;
  tss.ebx = NULL;
  tss.esp = USER_ESP;
  tss.ebp = tss.esp;
  tss.esi = NULL;
  tss.edi = NULL;
  tss.es = __USER_DS;
  tss.cs = __USER_CS;
  tss.ss = __USER_DS;
  tss.ds = __USER_DS;
  tss.fs = NULL;
  tss.gs = NULL;
  tss.LDTSegmentSelector = KERNEL_TSS;
  tss.debugTrap = 0;
  tss.IOMapBaseAddress = NULL;

  set_task_reg(KERNEL_TSS);
}

/* Initializes the ByteMap of free physical pages.
 * The kernel pages are marked as used */
int init_frames(void) {
  int i;
  /* Mark pages as Free */
  for (i = 0; i < TOTAL_PAGES; i++) {
    phys_mem[i] = FREE_FRAME;
  }
  /* Mark kernel pages as Used */
  for (i = 0; i < NUM_PAG_KERNEL; i++) {
    phys_mem[i] = USED_FRAME;
  }
  return 0;
}

/* alloc_frame - Search a free physical page (== frame) and mark it as
 * USED_FRAME. Returns the frame number or -1 if there isn't any frame
 * available. */
int alloc_frame(void) {
  int i;
  for (i = NUM_PAG_KERNEL; i < TOTAL_PAGES;) {
    if (phys_mem[i] == FREE_FRAME) {
      phys_mem[i] = USED_FRAME;
      return i;
    }
    i += 2; /* NOTE: There will be holes! This is intended.
                    DO NOT MODIFY! */
  }

  return -1;
}

/* alloc_frames - A plural wrapper for alloc_frame
 * frames should point to an int array with ammount entries.
 * Returns 0 on success. */
int alloc_frames(int ammount, int * frames) {
  for (int i = 0; i < ammount; ++i) {
    frames[i] = alloc_frame();

    if (frames[i] == -1) {
      free_frames(i, frames);      
      return -1;
    }
  }

  return 0;
}

/* Allocates multiple page blocks in the user pages. block_sizes should contain
 * the size of every requested block, in descending order. block_starts will
 * contain the first page of every block. All arrays should have size
 * block_count */
int allocate_user_pages(int * block_sizes, int * block_starts, int block_count, page_table_entry * PT, int * free_frames) {
  int consecutive_free_pages = 0;
  int allocated_count = 0;

  for (int i = 0; i < block_count; ++i) {
    block_starts[i] = NULL;
  }
  
  for (int i = NUM_PAG_KERNEL; i < TOTAL_PAGES && allocated_count < block_count; ++i) {
    if (PT[i].bits.present) {
      consecutive_free_pages = 0;
      continue;
    }

    ++consecutive_free_pages;

    if (i != TOTAL_PAGES - 1 && !PT[i+1].bits.present) continue;

    for (int j = 0; j < block_count; ++j) {
      if (block_starts[j] != NULL) continue;
      if (consecutive_free_pages < block_sizes[j]) continue;

      block_starts[j] = i - consecutive_free_pages + 1;
      consecutive_free_pages -= block_sizes[j];
      ++allocated_count;

      for (int k = 0; k < block_sizes[j]; ++k) {
        set_ss_pag(PT, block_starts[j] + k, *(free_frames++));
      }
    }
  }

  if (allocated_count != block_count) {
    deallocate_user_pages(block_sizes, block_starts, block_count, PT, NULL);
    return -1;
  }
  
  return 0;
}

/* Reverse of allocate_user_pages. If free_pages isn't null, the function
 * collects the frames used by the freed memory regions into the pointed
 * array. */
void deallocate_user_pages(int * block_sizes, int * block_starts, int block_count, page_table_entry * PT, int * free_frames) {
  for (int i = 0; i < block_count; ++i) {
    if (block_starts[i] == NULL) continue;

    for (int j = 0; j < block_sizes[i]; ++j) {
      if (free_frames != NULL) 
        *(free_frames++) = get_frame(PT, block_starts[i] + j);
      
      del_ss_pag(PT, block_starts[i] + j);
    }
  }
}

void free_user_pages(struct task_struct *task) {
  int pag;
  page_table_entry *process_PT = get_PT(task);
  /* DATA */
  for (pag = 0; pag < NUM_PAG_DATA; pag++) {
    free_frame(process_PT[PAG_LOG_INIT_DATA + pag].bits.pbase_addr);
    process_PT[PAG_LOG_INIT_DATA + pag].entry = 0;
  }
}

void clear_user_space(struct task_struct *task) {
  int pag;
  page_table_entry *process_PT = get_PT(task);

  for (pag = 0; pag < NUM_PAG_DATA + NUM_PAG_CODE; ++pag) {
    if (!process_PT[PAG_LOG_INIT_DATA + pag].bits.present)
      continue;
    free_frame(process_PT[PAG_LOG_INIT_DATA + pag].bits.pbase_addr);
    process_PT[PAG_LOG_INIT_DATA + pag].entry = 0;
  }
}

/* free_frame - Mark as FREE_FRAME the frame  'frame'.*/
void free_frame(unsigned int frame) {
  if ((frame > NUM_PAG_KERNEL) && (frame < TOTAL_PAGES))
    phys_mem[frame] = FREE_FRAME;
}

/* free_frames - Mark as FREE_FRAME the frames  'frames'.*/
void free_frames(int ammount, int * frames) {
  for (int i = 0; i < ammount; ++i) {
    free_frame(frames[i]);
  }
}

/* set_ss_pag - Associates logical page 'page' with physical page 'frame' */
void set_ss_pag(page_table_entry *PT, unsigned page, unsigned frame) {
  PT[page].entry = 0;
  PT[page].bits.pbase_addr = frame;
  PT[page].bits.user = 1;
  PT[page].bits.rw = 1;
  PT[page].bits.present = 1;
}

/* del_ss_pag - Removes mapping from logical page 'logical_page' */
void del_ss_pag(page_table_entry *PT, unsigned logical_page) {
  PT[logical_page].entry = 0;
}

/* get_frame - Returns the physical frame associated to page 'logical_page' */
unsigned int get_frame(page_table_entry *PT, unsigned int logical_page) {
  return PT[logical_page].bits.pbase_addr;
}
