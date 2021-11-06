/**
 * @file src\include\tftp_seriver.h
 *
 * Copyright (C) 2021
 *
 * tftp_seriver.h is free software: you can redistribute it and/or modify
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
#ifndef __TFTP_SERVER_H
#define __TFTP_SERVER_H

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
typedef struct tftp_context {
    uint8_t *buf;
    uint32_t size;
    uint32_t recv_size;
    uint32_t send_size;
    struct {
        bool (*recv)(struct tftp_context *ctx, uint32_t timeout);
        bool (*send)(struct tftp_context *ctx);
        bool (*filename_verify)(const char *filename);
        uint32_t (*filesize_verify)(const uint32_t tsize);
        uint32_t (*blksize_verify)(const uint32_t blksize);
        uint32_t (*timeout_verify)(const uint32_t timeout);
        bool (*start)(void);
        bool (*save)(uint32_t offset, const uint8_t *buf, uint32_t length);
        void (*over)(void);
    } ops;
} tftp_context_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern void tftp_server_init(tftp_context_t *ctx);
extern void tftp_server_receive(void);
extern void tftp_server_recv_timeout(void);

#ifdef __cplusplus
}
#endif
#endif /* __TFTP_SERVER_H */
