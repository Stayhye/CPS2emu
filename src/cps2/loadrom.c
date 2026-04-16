/******************************************************************************
    loadrom.c
******************************************************************************/

#include "emumain.h"
#include <unistd.h>
#include <stddef.h>
#include <ctype.h> 

// Helper function to force uppercase for PS2 hardware compatibility
static void force_upper(char *s) {
    if (!s) return;
    while (*s) {
        *s = (char)toupper((unsigned char)*s);
        s++;
    }
}

void swab(const void *src, void *dest, ssize_t n) {
    const unsigned char *s = (const unsigned char *)src;
    unsigned char *d = (unsigned char *)dest;
    n &= ~1;
    for (ssize_t i = 0; i < n; i += 2) {
        d[i] = s[i + 1];
        d[i + 1] = s[i];
    }
}

static int rom_fd;

/*--------------------------------------------------------
    ZIP file opening logic
--------------------------------------------------------*/

int file_open(const char *fname1, const char *fname2, const u32 crc, char *fname)
{
    int found = 0;
    struct zip_find_t file;
    char path[MAX_PATH];

    // Force check in cdrom0:\ROMS/
    sprintf(path, "cdrom0:\\ROMS\\%s.ZIP", fname1);
    force_upper(path); 

    if (zip_open(path) != -1)
    {
        if (zip_findfirst(&file))
        {
            if (file.crc32 == crc) found = 1;
            else {
                while (zip_findnext(&file)) {
                    if (file.crc32 == crc) { found = 1; break; }
                }
            }
        }
        if (!found) zip_close();
    }

    if (!found && fname2 != NULL)
    {
        sprintf(path, "cdrom0:\\ROMS\\%s.ZIP", fname2);
        force_upper(path);

        if (zip_open(path) != -1)
        {
            if (zip_findfirst(&file))
            {
                if (file.crc32 == crc) found = 1;
                else {
                    while (zip_findnext(&file)) {
                        if (file.crc32 == crc) { found = 1; break; }
                    }
                }
            }
            if (!found) zip_close();
        }
    }

    if (found)
    {
        if (fname) strcpy(fname, file.name);
        rom_fd = zopen(file.name);
        return rom_fd;
    }

    return -1;
}

void file_close(void)
{
    if (rom_fd != -1)
    {
        zclose(rom_fd);
        zip_close();
        rom_fd = -1;
    }
}

int file_read(void *buf, size_t length)
{
    if (rom_fd != -1 && buf != NULL)
        return zread(rom_fd, buf, length);
    return -1;
}

int file_getc(void)
{
    if (rom_fd != -1)
        return zgetc(rom_fd);
    return -1;
}

/*--------------------------------------------------------
    Cache file opening logic - FORCED TO MC1
--------------------------------------------------------*/

#if USE_CACHE
FILE* cachefile_open(void)
{
    char path[MAX_PATH];
    FILE* cache_fd;

    sprintf(path, "mc1:/%s.CACHE", game_name);
    force_upper(path);
    
    printf("CPS2EMU: Attempting cache load from %s\n", path);

    cache_fd = fopen(path, "rb");
    if (cache_fd != NULL) return cache_fd;

    if (cache_parent_name[0])
    {
        sprintf(path, "mc1:/%s.CACHE", cache_parent_name);
        force_upper(path);
        cache_fd = fopen(path, "rb");
        if (cache_fd != NULL) return cache_fd;
    }

    return NULL;
}
#endif

/*--------------------------------------------------------
    ROM Loading Core - WITH TLB MISS PROTECTION
--------------------------------------------------------*/

int rom_load(struct rom_t *rom, u8 *mem, int idx, int max)
{
    u32 offset, length;

    // --- CRITICAL PROTECTION ---
    // If mem is NULL or pointing to kernel space (low memory), abort.
    if (mem == NULL || (u32)mem < 0x00100000) {
        printf("CPS2EMU FATAL: rom_load received invalid memory pointer: 0x%08X\n", (u32)mem);
        return max; // Return max to stop the loop
    }

_continue:
    offset = rom[idx].offset;

    if (rom[idx].skip == 0)
    {
        if (file_read(&mem[offset], rom[idx].length) < 0) {
             printf("CPS2EMU: Error reading rom at index %d\n", idx);
        }
        if (rom[idx].type == ROM_WORDSWAP)
            swab(&mem[offset], &mem[offset], rom[idx].length);
    }
    else
    {
        int c;
        int skip = rom[idx].skip + rom[idx].group;
        length = 0;

        if (rom[idx].group == 1)
        {
            if (rom[idx].type == ROM_WORDSWAP)
                offset ^= 1;

            while (length < rom[idx].length)
            {
                if ((c = file_getc()) == EOF) break;
                mem[offset] = (u8)c;
                offset += skip;
                length++;
            }
        }
        else
        {
            while (length < rom[idx].length)
            {
                if ((c = file_getc()) == EOF) break;
                mem[offset + 0] = (u8)c;
                if ((c = file_getc()) == EOF) break;
                mem[offset + 1] = (u8)c;
                offset += skip;
                length += 2;
            }
        }
    }

    if (++idx != max)
    {
        if (rom[idx].type == ROM_CONTINUE)
        {
            goto _continue;
        }
    }

    return idx;
}
