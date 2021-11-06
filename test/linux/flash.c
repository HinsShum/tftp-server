/**
 * @file test/linux/flash.c
 *
 * Copyright (C) 2021
 *
 * flash.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author HinsShum hinsshum@qq.com
 *
 * @encoding utf-8
 */

/*---------- includes ----------*/
#include "flash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
static FILE *pfile = NULL;

/*---------- function ----------*/
bool flash_init(void)
{
    uint8_t *pbuf = malloc(16 * 1024);

    if(!pbuf) {
        perror("Malloc memory for flash failed\n");
        exit(EXIT_FAILURE);
    }
    memset(pbuf, 0xFF, 16 * 1024);
    pfile = fopen("flash.bin", "rb");
    if(pfile) {
        fread(pbuf, 16 * 1024, 1, pfile);
        fclose(pfile);
    }
    pfile = fopen("flash.bin", "wb+");
    if(!pfile) {
        perror("Open flash failed\n");
        free(pbuf);
        exit(EXIT_FAILURE);
    }
    fwrite(pbuf, 16 * 1024, 1, pfile);
    free(pbuf);

    return true;
}

void flash_deinit(void)
{
    if(pfile) {
        fclose(pfile);
        pfile = NULL;
    }
}

uint32_t flash_write(const uint8_t *pbuf, uint32_t offset, uint32_t len)
{
    uint32_t length = 0;
    uint32_t addr = offset;

    do {
        if(!pbuf || !pfile) {
            break;
        }
        if((addr + len) > (16 * 1024)) {
            length = (16 * 1024) - addr;
        } else {
            length = len;
        }
        fseek(pfile, addr, SEEK_SET);
        fwrite(pbuf, length, 1, pfile);
        fflush(pfile);
    } while(0);

    return length;
}

uint32_t flash_read(uint8_t *pbuf, uint32_t offset, uint32_t len)
{
    uint32_t length = 0;
    uint32_t addr = offset;

    do {
        if(!pbuf || !pfile) {
            break;
        }
        if((addr + len) > (16 * 1024)) {
            length = (16 * 1024) - addr;
        } else {
            length = len;
        }
        fseek(pfile, addr, SEEK_SET);
        fread(pbuf, length, 1, pfile);
    } while(0);

    return length;
}

bool flash_erase(void)
{
    bool retval = false;
    uint8_t erase_val = 0xFF;

    if(pfile) {
        fseek(pfile, 0, SEEK_SET);
        for(uint32_t i = 0; i < (16 * 1024); ++i) {
            fwrite(&erase_val, 1, 1, pfile);
        }
        fflush(pfile);
        retval = true;
    }

    return retval;
}
