/* unzip.c -- IO on .zip files using zlib
   Version 0.15 beta, Mar 19th, 1998,
   Modified for PS2 Case-Insensitivity
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "zlib.h"
#include "unzip.h"

// ... [Keep all your existing structs and helper functions] ...

/* FORCE Case Insensitivity for PS2 Compatibility 
   Modified unzLocateFile 
*/
int unzLocateFile(unzFile file, const char *szFileName, int iCaseSensitivity)
{
    unz_s* s;
    int err;
    uLong num_fileSaved;
    uLong pos_in_central_dirSaved;

    if (file == NULL)
        return UNZ_PARAMERROR;

    if (strlen(szFileName) >= UNZ_MAXFILENAMEINZIP)
        return UNZ_PARAMERROR;

    s = (unz_s *)file;
    if (!s->current_file_ok)
        return UNZ_END_OF_LIST_OF_FILE;

    /* FIX: Always force case-insensitive comparison (Mode 2) 
       This solves issues where 'ROM.zip' contains 'file.bin' but code looks for 'FILE.BIN'
    */
    iCaseSensitivity = 2; 

    num_fileSaved = s->num_file;
    pos_in_central_dirSaved = s->pos_in_central_dir;

    err = unzGoToFirstFile(file);

    while (err == UNZ_OK)
    {
        char szCurrentFileName[UNZ_MAXFILENAMEINZIP + 1];

        unzGetCurrentFileInfo(file, NULL,
                                szCurrentFileName, sizeof(szCurrentFileName) - 1,
                                NULL, 0, NULL, 0);

        if (unzStringFileNameCompare(szCurrentFileName, szFileName, iCaseSensitivity) == 0)
            return UNZ_OK;

        err = unzGoToNextFile(file);
    }

    s->num_file = num_fileSaved;
    s->pos_in_central_dir = pos_in_central_dirSaved;
    return err;
}

// ... [Rest of the file remains the same] ...
