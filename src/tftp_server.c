/**
 * @file src\tftp_server.c
 *
 * Copyright (C) 2021
 *
 * tftp_server.c is free software: you can redistribute it and/or modify
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
#include "include/tftp_seriver.h"
#include "include/tftp_utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*---------- macro ----------*/
#define __tftp_log(x, y...)                             printf(x, ##y)
#define TFTP_TRANS_MODE                                 "octet"
#define TFTP_TRANS_DEFAULT_BLKSIZE                      (512)
#define TFTP_TRANS_DEFAULT_TIMEOUT                      (10)
#define TFTP_TRANS_MAX_REPEAT_COUNT                     (50)

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static void _tftp_rrq(void);
static void _tftp_wrq(void);
static void _tftp_data(void);
static void _tftp_error(void);

/*---------- type define ----------*/
typedef enum {
    RRQ = 1,
    WRQ = 2,
    DATA = 3,
    ACK = 4,
    ERROR = 5,
    OACK = 6
} tftp_opcode_en_t;

typedef enum {
    ERRCODE_NOT_DEFINED,
    ERRCODE_FILE_NOT_FOUND,
    ERRCODE_ACCESS_NOT_ALLOW,
    ERRCODE_MEM_OVERFLOW,
    ERRCODE_TFTP_OPER_NOT_ALLOW,
    ERRCODE_TRANS_ID_UNKONW,
    ERRCODE_FILE_EXIT,
    ERRCODE_USER_NOT_EXIT,
    ERRCODE_OPT_ERROR
} tftp_errcode_en_t;

typedef enum {
    STATE_IDLE,
    STATE_OACK,
    STATE_RECV
} tftp_state_en_t;

typedef struct {
    tftp_state_en_t state;
    uint32_t blksize;
    uint32_t timeout;
    uint32_t tsize;
    uint32_t recv_offset;
    uint32_t repeat_cnt;
    tftp_context_t *ctx;
} tftp_server_t;

typedef struct {
    uint16_t opc;
    uint8_t data[0];
} tftp_req_t;

typedef struct {
    uint16_t opc;
    uint16_t block;
    uint8_t data[0];
} tftp_data_t;

typedef struct {
    uint16_t opc;
    uint16_t block;
} tftp_ack_t;

typedef struct {
    uint16_t opc;
    void (*cb)(void);
} tftp_cb_t;

/*---------- variable ----------*/
static tftp_server_t tftp;
static tftp_cb_t tftp_cb_array[] = {
    {RRQ, _tftp_rrq},
    {WRQ, _tftp_wrq},
    {DATA, _tftp_data},
    {ACK, NULL},
    {ERROR, _tftp_error},
    {OACK, NULL}
};

/*---------- function ----------*/
static bool inline __is_little_endian(void)
{
    uint32_t temp = 0x01;

    return ((uint8_t *)&temp)[0] == 0x01 ? true : false;
}

static uint16_t __ntohs(uint16_t ns)
{
    if(__is_little_endian()) {
        ns = ((ns & 0xFF) << 8) | ((ns & 0xFF00) >> 8);
    }

    return ns;
}

static uint16_t __htons(uint16_t hs)
{
    if(__is_little_endian()) {
        hs = ((hs & 0xFF) << 8) | ((hs & 0xFF00) >> 8);
    }

    return hs;
}

static uint32_t __tftp_error_make(uint8_t *buf, uint16_t err_code, const char *err_msg)
{
    uint32_t size = 0;

    buf[0] = 0;
    buf[1] = ERROR & 0xFF;
    buf[2] = (err_code >> 8) & 0xFF;
    buf[3] = err_code & 0xFF;
    strcpy((char *)&buf[4], err_msg);
    size = 4 + strlen(err_msg);
    buf[size] = '\0';
    size++;

    return size;
}

static inline uint16_t _get_opc(const uint8_t *buf)
{
    uint16_t opc = 0;

    memcpy(&opc, buf, sizeof(opc));

    return __ntohs(opc);
}

static void _tftp_rrq(void)
{
    tftp.ctx->send_size = __tftp_error_make(tftp.ctx->buf, ERRCODE_FILE_NOT_FOUND, "file not found");
    tftp.ctx->ops.send(tftp.ctx);
    tftp.state = STATE_IDLE;
}

static void _tftp_wrq(void)
{
    tftp_req_t *req = (tftp_req_t *)tftp.ctx->buf;
    const char *filename = NULL, *mode = NULL;
    uint32_t blksize = 0, timeout = 0, tsize = 0;
    tftp_opt_context opt_ctx = {0};
    char *p = NULL;

    do {
        tftp.blksize = TFTP_TRANS_DEFAULT_BLKSIZE;
        tftp.timeout = TFTP_TRANS_DEFAULT_TIMEOUT;
        tftp.repeat_cnt = 0;
        tftp.recv_offset = 0;
        tftp.state = STATE_IDLE;
        if(tftp_utils_get_first_string(req->data, tftp.ctx->recv_size - offsetof(tftp_req_t, data), &opt_ctx) != 0) {
            /* wrq error */
            tftp.ctx->send_size = __tftp_error_make(tftp.ctx->buf, ERRCODE_OPT_ERROR, "opt negotiation error");
            tftp.ctx->ops.send(tftp.ctx);
            break;
        }
        filename = (char *)opt_ctx.cur;
        if(tftp_utils_get_next_string(&opt_ctx) != 0) {
            tftp.ctx->send_size = __tftp_error_make(tftp.ctx->buf, ERRCODE_OPT_ERROR, "opt negotiation error");
            tftp.ctx->ops.send(tftp.ctx);
            break;
        }
        mode = (char *)opt_ctx.cur;
        do {
            if(strcmp((char *)opt_ctx.cur, "blksize") == 0) {
                if(tftp_utils_get_next_string(&opt_ctx) != 0) {
                    break;
                }
                blksize = strtoul((char *)opt_ctx.cur, NULL, 10);
            } else if(strcmp((char *)opt_ctx.cur, "timeout") == 0) {
                if(tftp_utils_get_next_string(&opt_ctx) != 0) {
                    break;
                }
                timeout = strtoul((char *)opt_ctx.cur, NULL, 10);
            } else if(strcmp((char *)opt_ctx.cur, "tsize") == 0) {
                if(tftp_utils_get_next_string(&opt_ctx) != 0) {
                    break;
                }
                tsize = strtoul((char *)opt_ctx.cur, NULL, 10);
            }
            tftp_utils_get_next_string(&opt_ctx);
        } while(opt_ctx.next);
        if(strcmp(mode, TFTP_TRANS_MODE) != 0) {
            /* mode error */
            tftp.ctx->send_size = __tftp_error_make(tftp.ctx->buf, ERRCODE_NOT_DEFINED, "only support 'octet' mode");
            tftp.ctx->ops.send(tftp.ctx);
            break;
        }
        if(!tsize) {
            /* not trans file size opt */
            tftp.ctx->send_size = __tftp_error_make(tftp.ctx->buf, ERRCODE_NOT_DEFINED, "no file size specified");
            tftp.ctx->ops.send(tftp.ctx);
            break;
        }
        if(!tftp.ctx->ops.filename_verify(filename)) {
            tftp.ctx->send_size = __tftp_error_make(tftp.ctx->buf, ERRCODE_NOT_DEFINED, "file name error");
            tftp.ctx->ops.send(tftp.ctx);
            break;
        }
        if(!tftp.ctx->ops.start()) {
            tftp.ctx->send_size = __tftp_error_make(tftp.ctx->buf, ERRCODE_NOT_DEFINED, "erase flash error");
            tftp.ctx->ops.send(tftp.ctx);
            break;
        }
        tftp.tsize = tftp.ctx->ops.filesize_verify(tsize);
        /* wrq ok, make oack */
        req->opc = __htons(OACK);
        p =  (char *)req->data;
        if(blksize) {
            tftp.blksize = tftp.ctx->ops.blksize_verify(blksize);
            p[sprintf(p, "%s", "blksize")] = '\0';
            p += strlen(p) + 1;
            p[sprintf(p, "%d", blksize)] = '\0';
            p += strlen(p) + 1;
        }
        if(timeout) {
            tftp.timeout = tftp.ctx->ops.timeout_verify(timeout);
            p[sprintf(p, "%s", "timeout")] = '\0';
            p += strlen(p) + 1;
            p[sprintf(p, "%d", timeout)] = '\0';
            p += strlen(p) + 1;
        }
        p[sprintf(p, "%s", "tsize")] = '\0';
        p += strlen(p) + 1;
        p[sprintf(p, "%d", tsize)] = '\0';
        p += strlen(p) + 1;
        tftp.ctx->send_size = p - (char *)req;
        tftp.ctx->ops.send(tftp.ctx);
        tftp.state = STATE_OACK;
    } while(0);
}

static void _tftp_data(void)
{
    static uint16_t blocks = 0;
    tftp_data_t *data = (tftp_data_t *)tftp.ctx->buf;
    tftp_ack_t *ack = (tftp_ack_t *)tftp.ctx->buf;
    uint32_t blksize = tftp.ctx->recv_size - offsetof(tftp_data_t, data);

    do {
        if(tftp.state == STATE_OACK) {
            tftp.state = STATE_RECV;
            blocks = 1;
        }
        if(tftp.state != STATE_RECV) {
            tftp.ctx->send_size = __tftp_error_make(tftp.ctx->buf, ERRCODE_ACCESS_NOT_ALLOW, "illegal access");
            tftp.ctx->ops.send(tftp.ctx);
            tftp.state = STATE_IDLE;
            break;
        }
        if(blksize > tftp.blksize) {
            tftp.ctx->send_size = __tftp_error_make(tftp.ctx->buf, ERRCODE_NOT_DEFINED, "blksize error");
            tftp.ctx->ops.send(tftp.ctx);
            tftp.state = STATE_IDLE;
            break;
        } else if(blksize < tftp.blksize) {
            if((tftp.tsize - tftp.recv_offset) != blksize) {
                tftp.ctx->send_size = __tftp_error_make(tftp.ctx->buf, ERRCODE_NOT_DEFINED, "file size error");
                tftp.ctx->ops.send(tftp.ctx);
                tftp.state = STATE_IDLE;
                break;
            }
        }
        if(blocks == __ntohs(data->block)) {
            if(!tftp.ctx->ops.save(tftp.recv_offset, data->data, blksize)) {
                tftp.ctx->send_size = __tftp_error_make(tftp.ctx->buf, ERRCODE_NOT_DEFINED, "save to flash failed");
                tftp.ctx->ops.send(tftp.ctx);
                tftp.state = STATE_IDLE;
                break;
            }
            blocks++;
            tftp.recv_offset += blksize;
        }
        ack->opc = __htons(ACK);
        ack->block = __htons(blocks - 1);
        tftp.ctx->send_size = sizeof(tftp_ack_t);
        tftp.ctx->ops.send(tftp.ctx);
        if(tftp.tsize == tftp.recv_offset) {
            /* recv firmware complete */
            tftp.ctx->ops.over();
        }
    } while(0);
}

static void _tftp_error(void)
{
    tftp.state = STATE_IDLE;
}

static void *_find_cb(uint16_t opc)
{
    void *cb = NULL;

    for(uint16_t i = 0; i < (sizeof(tftp_cb_array) / sizeof(tftp_cb_array[0])); ++i) {
        if(tftp_cb_array[i].opc == opc) {
            cb = tftp_cb_array[i].cb;
            break;
        }
    }

    return cb;
}

void tftp_server_init(tftp_context_t *ctx)
{
    tftp.ctx = ctx;
    tftp.blksize = TFTP_TRANS_DEFAULT_BLKSIZE;
    tftp.timeout = TFTP_TRANS_DEFAULT_TIMEOUT;
    tftp.state = STATE_IDLE;
    tftp.repeat_cnt = 0;
}

void tftp_server_receive(void)
{
    void (*cb)(void) = NULL;
    uint16_t opc = 0;

    if(tftp.ctx->ops.recv(tftp.ctx, tftp.timeout)) {
        opc = _get_opc(tftp.ctx->buf);
        cb = (void (*)(void))_find_cb(opc);
        if(cb) {
            cb();
        } else {
            tftp.ctx->send_size = __tftp_error_make(tftp.ctx->buf, ERRCODE_TFTP_OPER_NOT_ALLOW, "invalid tftp opc");
            tftp.ctx->ops.send(tftp.ctx);
            tftp.state = STATE_IDLE;
        }
    }
}

void tftp_server_recv_timeout(void)
{
    if(tftp.state != STATE_IDLE) {
        if(tftp.repeat_cnt > TFTP_TRANS_MAX_REPEAT_COUNT) {
            tftp.repeat_cnt = 0;
            tftp.state = STATE_IDLE;
        } else {
            tftp.ctx->ops.send(tftp.ctx);
            tftp.repeat_cnt++;
        }
    }
}
