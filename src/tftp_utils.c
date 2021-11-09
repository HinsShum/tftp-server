/**
 * @file src\tftp_utils.c
 *
 * Copyright (C) 2021
 *
 * tftp_utils.c is free software: you can redistribute it and/or modify
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
#include "include/tftp_utils.h"
#include <ctype.h>
#include <string.h>

/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
int32_t tftp_utils_get_first_string(const uint8_t *buf, uint16_t size, tftp_opt_context *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->buf = buf;
    ctx->size = size;
    for(uint16_t i = 0; i < size; ++i) {
        if(isalnum(buf[i])) {
            ctx->cur = &buf[i];
            ctx->cur_size = strlen((char *)ctx->cur);
            if((ctx->cur + ctx->cur_size) > (buf + size)) {
                ctx->cur_size = buf + size - ctx->cur;
                ctx->next = NULL;
            } else {
                ctx->next = ctx->cur + ctx->cur_size + 1;
                if(!isalnum(ctx->next[0])) {
                    ctx->next = NULL;
                }
            }
            break;
        }
    }

    return ctx->cur ? 0 : -1;
}

int32_t tftp_utils_get_next_string(tftp_opt_context *ctx)
{
    int32_t retval = -1;

    do {
        if(!ctx) {
            break;
        }
        ctx->cur = ctx->next;
        if(!ctx->next) {
            ctx->cur_size = 0;
            break;
        }
        ctx->cur_size = strlen((char *)ctx->cur);
        if((ctx->cur + ctx->cur_size) > (ctx->buf + ctx->size)) {
            ctx->cur_size = ctx->buf + ctx->size - ctx->cur;
            ctx->next = NULL;
        } else {
            ctx->next = ctx->cur + ctx->cur_size + 1;
            if(!isalnum(ctx->next[0])) {
                ctx->next = NULL;
            }
        }
        retval = 0;
    } while(0);

    return retval;
}
