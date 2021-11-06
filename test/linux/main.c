/**
 * @file test\linux\main.c
 *
 * Copyright (C) 2021
 *
 * main.c is free software: you can redistribute it and/or modify
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
#include "tftp_seriver.h"
#include "flash.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <malloc.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
static uint8_t recv_buf[1048];
static struct sockaddr_in cliaddr;
static socklen_t cliaddr_len = sizeof(cliaddr);
static tftp_context_t tftp_ctx;
static int32_t sock;
static fd_set servset = {0};

/*---------- function ----------*/
static bool _udp_recv(int32_t sock_num, fd_set recv_set, struct timeval *recv_timeout, tftp_context_t *ctx)
{
    int32_t recv_count = 0;
    bool retval = false;

    do {
        recv_count = select(sock_num + 1, &recv_set, NULL, NULL, recv_timeout);
        if(recv_count < 0) {
            break;
        }
        if(FD_ISSET(sock_num, &recv_set) == 0) {
            tftp_server_recv_timeout();
            break;
        }
        memset(ctx->buf, 0, ctx->size);
        if((recv_count = recvfrom(sock_num, ctx->buf, ctx->size, 0, (struct sockaddr *)&cliaddr, &cliaddr_len)) <= 0) {
            printf("Recv udp package error\n");
            break;
        }
        // printf("Recv from client(%s:%d)\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
        ctx->recv_size = recv_count;
        retval = true;
    } while(0);

    return retval;
}

static bool _tftp_recv_cb(tftp_context_t *ctx, uint32_t timeout)
{
    struct timeval seltime = {
        .tv_sec = timeout,
        .tv_usec = 0
    };

    return _udp_recv(sock, servset, &seltime, ctx);
}

static bool _tftp_send_cb(tftp_context_t *ctx)
{
    sendto(sock, ctx->buf, ctx->send_size, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));

    return true;
}

static bool _tftp_filename_verify(const char *filename)
{
    printf("Recv filename: %s\n", filename);

    return true;
}

static uint32_t _tftp_filesize_verify(const uint32_t tsize)
{
    printf("Recv filesize: %dBytes\n", tsize);

    return tsize;
}

static uint32_t _tftp_blksize_verify(const uint32_t blksize)
{
    printf("Blksize: %dBytes\n", blksize);

    return blksize;
}

static uint32_t _tftp_timeout_verify(const uint32_t timeout)
{
    printf("Timeout: %ds\n", timeout);

    return timeout;
}

static bool _tftp_start(void)
{
    printf("Start tftp recv...\n");

    return flash_erase();
}

static bool _tftp_save(uint32_t offset, const uint8_t *buf, uint32_t length)
{
    return (length == flash_write(buf, offset, length));
}

static void _tftp_over(void)
{
    printf("Recv tftp file ok\n");
}

int32_t main(void)
{
    struct sockaddr_in servaddr = {0};
    int32_t recv_count = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(6666);
    if(bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind socket failed\n");
        exit(EXIT_FAILURE);
    }
    FD_ZERO(&servset);
    FD_SET(sock, &servset);
    /* initialize tftp ctx */
    tftp_ctx.buf = recv_buf;
    tftp_ctx.size = sizeof(recv_buf);
    tftp_ctx.ops.recv = _tftp_recv_cb;
    tftp_ctx.ops.send = _tftp_send_cb;
    tftp_ctx.ops.filename_verify = _tftp_filename_verify;
    tftp_ctx.ops.filesize_verify = _tftp_filesize_verify;
    tftp_ctx.ops.blksize_verify = _tftp_blksize_verify;
    tftp_ctx.ops.timeout_verify = _tftp_timeout_verify;
    tftp_ctx.ops.start = _tftp_start;
    tftp_ctx.ops.save = _tftp_save;
    tftp_ctx.ops.over = _tftp_over;
    tftp_server_init(&tftp_ctx);
    flash_init();
    for(;;) {
        tftp_server_receive();
    }
}