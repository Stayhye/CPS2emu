/******************************************************************************
    cache.c
    Memory cache interface function - Optimized for Menu Selection
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "emumain.h"

#if USE_CACHE

#define MIN_CACHE_SIZE        0x40        // Minimum  4MB
#define MAX_CACHE_SIZE        0x200        // Maximum 32MB
#define BLOCK_SIZE            0x10000        // 1 Block size = 64KB
#define BLOCK_MASK            0xffff
#define BLOCK_SHIFT            16
#define BLOCK_NOT_CACHED    0xffff
#define BLOCK_EMPTY            0xffffffff

#define GFX_MEMORY            memory_region_gfx1
#define GFX_SIZE            memory_length_gfx1

/******************************************************************************
    Global variable
******************************************************************************/

u32 (*read_cache)(u32 offset);

u32 block_offset[MAX_CACHE_BLOCKS];
u8  *block_empty = (u8 *)block_offset;
u32 block_start;
u32 block_size;
u32 block_capacity;
u8  *block_data = NULL;

int cache_type;

/******************************************************************************
    Local struct/variable
******************************************************************************/

typedef struct cache_s
{
    int idx;
    int block;
    struct cache_s *prev;
    struct cache_s *next;
} cache_t;

static cache_t ALIGN_DATA cache_data[MAX_CACHE_SIZE];
static cache_t *head;
static cache_t *tail;

static int num_cache;
static u16 ALIGN_DATA blocks[MAX_CACHE_BLOCKS];
static char spr_cache_name[MAX_PATH];
static FILE* cache_fd;

/******************************************************************************
    Local function
******************************************************************************/

static inline void load_block(int index, int offset)
{
    int size, length;
    u8 src[BLOCK_SIZE];
    u8 dst[BLOCK_SIZE];

    if (!block_data) return;

    size = block_data[offset] | (block_data[offset + 1] << 8);
    if(size) {
        length = BLOCK_SIZE;
#ifdef MMUHACK
        uncompress(&GFX_MEMORY[index << BLOCK_SHIFT], &length, &block_data[offset + 2], size);
#else
        memcpy(src, &block_data[offset + 2], size);
        uncompress(dst, &length, src, size);
        memcpy(&GFX_MEMORY[index << BLOCK_SHIFT], dst, length);
#endif
    } else {
        memcpy(&GFX_MEMORY[index << BLOCK_SHIFT], &block_data[offset + 2], BLOCK_SIZE);
    }
}

static int fill_cache(void)
{
    int i, block;
    int block_free = 0;
    cache_t *p;

    if (cache_fd == NULL) return 0;

    if(block_data == NULL) {
        block_data = (u8 *)malloc(block_size);
        block_free = 1;
    }
    
    msg_printf("Loading cache data... \n");

    if (fseek(cache_fd, block_start, SEEK_SET) != 0) return 0;
    fread(block_data, sizeof(char), block_size, cache_fd);
     
    msg_printf("Fill cache data... 0%%\n");

    i = 0;
    block = 0;
    while (i < num_cache)
    {
        if (block_offset[block] != BLOCK_EMPTY)
        {
            p = head;
            p->block = block;
            blocks[block] = p->idx;
            
            load_block(p->idx, block_offset[block]);

            head = p->next;
            head->prev = NULL;

            p->prev = tail;
            p->next = NULL;

            tail->next = p;
            tail = p;
            i++;
            if((i % 20) == 0)
                msg_printf("Fill cache data... %d%%\n", i * 100 / num_cache);
        }

        if (++block >= MAX_CACHE_BLOCKS)
            break;
    }

    if((i % 20) != 0)
        msg_printf("Fill cache data... Complete\n");
    
    if(block_free) {
        free(block_data);
        block_data = NULL;
    }

    return 1;
}

static u32 read_cache_static(u32 offset)
{
    int idx = blocks[offset >> BLOCK_SHIFT];
    return ((idx << BLOCK_SHIFT) | (offset & BLOCK_MASK));
}

static u32 read_cache_compress(u32 offset)
{
    s16 new_block = offset >> BLOCK_SHIFT;
    u32 idx = blocks[new_block];
    cache_t *p;

    if (idx == BLOCK_NOT_CACHED)
    {
        p = head;
        blocks[p->block] = BLOCK_NOT_CACHED;
        p->block = new_block;
        blocks[new_block] = p->idx;
        load_block(p->idx, block_offset[new_block]);
    }
    else p = &cache_data[idx];

    if (p->next)
    {
        if (p->prev)
        {
            p->prev->next = p->next;
            p->next->prev = p->prev;
        }
        else
        {
            head = p->next;
            head->prev = NULL;
        }
        p->prev = tail;
        p->next = NULL;
        tail->next = p;
        tail = p;
    }

    return ((tail->idx << BLOCK_SHIFT) | (offset & BLOCK_MASK));
}

/******************************************************************************
    Cache Setup Functions
******************************************************************************/

void cache_init(void)
{
    int i;
    num_cache  = 0;
    cache_fd   = NULL;
    cache_type = CACHE_NOTFOUND;
    read_cache = read_cache_static;

    for (i = 0; i < MAX_CACHE_BLOCKS; i++)
        blocks[i] = BLOCK_NOT_CACHED;
}

int cache_start(void)
{
    int i;

    // --- STREAMING SETUP ---
    read_cache = read_cache_compress;

    // --- PATHING ---
    extern char game_dir[];
    extern char cache_dir[];
    strcpy(game_dir, "cdrom0:\\ROMS");
    strcpy(cache_dir, "mc1:");

    // --- ISO DISCOVERY ---
    // Since you are selecting Avp, ensure rominfo.cps2 is at root or adjust here
    cache_fd = fopen("cdrom0:\\ROMINFO.CPS2;1", "rb");
    if (cache_fd == NULL) cache_fd = fopen("cdrom0:\\ROMINFO.CPS2", "rb");
    if (cache_fd == NULL) cache_fd = fopen("cdrom0:\\rominfo.cps2", "rb");

    if (cache_fd == NULL) 
    {
        msg_printf("ERROR: Cache Info not found on cdrom0.\n");
        return 0; 
    }

    GFX_MEMORY = upper_memory;
    if(!GFX_MEMORY) {
        msg_printf("ERROR: GFX_MEMORY is NULL.\n");
        if (cache_fd) fclose(cache_fd);
        return 0;
    }

    // --- CONFIGURE WINDOW ---
    if(option_fullcache) {
        num_cache = CACHE_SIZE >> BLOCK_SHIFT;
        block_data = malloc(block_size);
    } else {
        num_cache = ((CACHE_SIZE - block_size) & ~0xffff) >> BLOCK_SHIFT;
        block_data = &GFX_MEMORY[num_cache << BLOCK_SHIFT];
    }

    msg_printf("Loading AVPU Cache...\n");

    for (i = 0; i < num_cache; i++)
        cache_data[i].idx = i;
    for (i = 1; i < num_cache; i++)
        cache_data[i].prev = &cache_data[i - 1];
    for (i = 0; i < num_cache - 1; i++)
        cache_data[i].next = &cache_data[i + 1];

    cache_data[0].prev = NULL;
    cache_data[num_cache - 1].next = NULL;

    head = &cache_data[0];
    tail = &cache_data[num_cache - 1];

    if (!fill_cache())
    {
        msg_printf("ROM Data load failed!\n");
        if (cache_fd) fclose(cache_fd);
        return 0;
    }

    msg_printf("AVPU Cache Initialized.\n");
    return 1;
}

void cache_shutdown(void)
{
    num_cache = 0;
    if(block_data != NULL) free(block_data);
    block_data = NULL;
    if (cache_fd != NULL) {
        fclose(cache_fd);
        cache_fd = NULL;
    }
}

void cache_sleep(int flag) {}

#endif /* USE_CACHE */
