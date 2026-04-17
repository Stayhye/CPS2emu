#include "romcnv.h"

#define SPR_NOT_EMPTY           0x80

enum
{
    REGION_GFX1 = 0,
    REGION_SKIP
};

enum
{
    ROM_LOAD = 0,
    ROM_CONTINUE,
    ROM_WORDSWAP,
    MAP_MAX
};

enum
{
    TILE08 = 0,
    TILE16,
    TILE32,
    TILE_SIZE_MAX
};

static u8  *memory_region_gfx1;
static u32 memory_length_gfx1;

static u32 gfx_total_elements[TILE_SIZE_MAX];
static u8  *gfx_pen_usage[TILE_SIZE_MAX];

static char game_dir[MAX_PATH];
static char zip_dir[MAX_PATH];
static char launchDir[MAX_PATH];

static char game_name[16];
static char parent_name[16];
static char cache_name[16];

struct cacheinfo_t
{
    const char *name;
    u32  object_start;
    u32  object_end;
    u32  scroll1_start;
    u32  scroll1_end;
    u32  scroll2_start;
    u32  scroll2_end;
    u32  scroll3_start;
    u32  scroll3_end;
    u32  object2_start;
    u32  object2_end;
};

// ... [CPS2_cacheinfo array and tile data remain as provided] ...

// Updated load_rom_info to prioritize PS2 CD-ROM pathing
int load_rom_info(const char *game_name)
{
    FILE *fp;
    char path[MAX_PATH];
    char buf[256];
    int rom_start = 0;
    int region = 0;

    num_gfx1rom = 0;

    // Force the emulator to look at the CD-ROM config folder
    sprintf(path, "cdrom0:\\CONFIG\\ROMINFO.CPS2;1");
    
    if ((fp = fopen(path, "r")) != NULL)
    {
        while (fgets(buf, 255, fp))
        {
            if (buf[0] == '/' && buf[1] == '/')
                continue;

            if (buf[0] != '\t')
            {
                if (buf[0] == '\r' || buf[0] == '\n')
                {
                    continue;
                }
                else if (str_cmp(buf, "FILENAME(") == 0)
                {
                    char *name, *parent;

                    strtok(buf, " ");
                    name    = strtok(NULL, " ,");
                    parent  = strtok(NULL, " ,");

                    if (strcasecmp(name, game_name) == 0)
                    {
                        if (str_cmp(parent, "cps2") == 0)
                            parent_name[0] = '\0';
                        else
                            strcpy(parent_name, parent);

                        rom_start = 1;
                    }
                }
                else if (rom_start && str_cmp(buf, "END") == 0)
                {
                    fclose(fp);
                    return 1; // Success
                }
            }
            else if (rom_start)
            {
                if (str_cmp(&buf[1], "REGION(") == 0)
                {
                    char *size, *type;

                    strtok(&buf[1], " ");
                    size = strtok(NULL, " ,");
                    type = strtok(NULL, " ,");

                    if (strcmp(type, "GFX1") == 0)
                    {
                        sscanf(size, "%x", &memory_length_gfx1);
                        region = REGION_GFX1;
                    }
                    else
                    {
                        region = REGION_SKIP;
                    }
                }
                else if (str_cmp(&buf[1], "ROM(") == 0)
                {
                    char *type, *offset, *length, *crc;

                    strtok(&buf[1], " ");
                    type   = strtok(NULL, " ,");
                    offset = strtok(NULL, " ,");
                    length = strtok(NULL, " ,");
                    crc    = strtok(NULL, " ");

                    if (region == REGION_GFX1 && num_gfx1rom < MAX_GFX1ROM)
                    {
                        sscanf(type, "%x", &gfx1rom[num_gfx1rom].type);
                        sscanf(offset, "%x", &gfx1rom[num_gfx1rom].offset);
                        sscanf(length, "%x", &gfx1rom[num_gfx1rom].length);
                        sscanf(crc, "%x", &gfx1rom[num_gfx1rom].crc);
                        gfx1rom[num_gfx1rom].group = 0;
                        gfx1rom[num_gfx1rom].skip = 0;
                        num_gfx1rom++;
                    }
                }
                else if (str_cmp(&buf[1], "ROMX(") == 0)
                {
                    char *type, *offset, *length, *crc, *group, *skip;

                    strtok(&buf[1], " ");
                    type   = strtok(NULL, " ,");
                    offset = strtok(NULL, " ,");
                    length = strtok(NULL, " ,");
                    crc    = strtok(NULL, " ,");
                    group  = strtok(NULL, " ,");
                    skip   = strtok(NULL, " ");

                    if (region == REGION_GFX1 && num_gfx1rom < MAX_GFX1ROM)
                    {
                        sscanf(type, "%x", &gfx1rom[num_gfx1rom].type);
                        sscanf(offset, "%x", &gfx1rom[num_gfx1rom].offset);
                        sscanf(length, "%x", &gfx1rom[num_gfx1rom].length);
                        sscanf(crc, "%x", &gfx1rom[num_gfx1rom].crc);
                        sscanf(group, "%d", &gfx1rom[num_gfx1rom].group);
                        sscanf(skip, "%d", &gfx1rom[num_gfx1rom].skip);
                        num_gfx1rom++;
                    }
                }
            }
        }
        fclose(fp);
    }
    return 0;
}
