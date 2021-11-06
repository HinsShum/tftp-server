/**
 * @file src\include\tftp_utils.h
 *
 * Copyright (C) 2021
 *
 * tftp_utils.h is free software: you can redistribute it and/or modify
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
#ifndef __TFTP_UTILS_H
#define __TFTP_UTILS_H

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
typedef struct {
    const uint8_t *buf;
    uint16_t size;
    uint16_t cur_size;
    const uint8_t *cur;
    const uint8_t *next;
} tftp_opt_context;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern int32_t tftp_utils_get_first_string(const uint8_t *buf, uint16_t size, tftp_opt_context *ctx);
extern int32_t tftp_utils_get_next_string(tftp_opt_context *ctx);

#ifdef __cplusplus
}
#endif
#endif /* __TFTP_UTILS_H */
