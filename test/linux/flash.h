/**
 * @file test/linux/flash.h
 *
 * Copyright (C) 2021
 *
 * flash.h is free software: you can redistribute it and/or modify
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
#ifndef __FLASH_H
#define __FLASH_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern bool flash_init(void);
extern void flash_deinit(void);
extern uint32_t flash_write(const uint8_t *pbuf, uint32_t offset, uint32_t len);
extern uint32_t flash_read(uint8_t *pbuf, uint32_t offset, uint32_t len);
extern bool flash_erase(void);

#ifdef __cplusplus
}
#endif
#endif /* __FLASH_H */
