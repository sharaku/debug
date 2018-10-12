/* --
 *
 * MIT License
 * 
 * Copyright (c) 2014-2018 Abe Takafumi
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <libsharaku/debug/log.h>

static inline void
___log_trace_internal(log_trace_t *trc, uint32_t us, const char *func,
		    uint16_t line, int8_t type, uint8_t u8)
{
	trc->func = func;
	trc->usec = 0;
	trc->line = line;
	trc->type = type;
}

static inline void
__log_infolog_internal_idxadd(log_header_t *hdr, int idx)
{
	// 一番大きなログが入らなければローテーションする。
	if (hdr->head_idx > (hdr->log_blks - LOG_MAXBLKS)) {
		hdr->head_idx = 0;
	} else {
		hdr->head_idx += idx;
	}
	if (hdr->head_idx > hdr->tail_idx) {
		hdr->tail_idx = hdr->head_idx;
	}
}

void
__log_trace_internal(log_header_t *hdr, uint32_t us, const char *func, uint16_t line)
{
	uint32_t idx = 0;
	log_trace_t *trc = NULL;

	idx = hdr->head_idx;
	__log_infolog_internal_idxadd(hdr, LOG_BLKS(log_trace_t));
	trc = (log_trace_t*)(hdr->log_top + idx * LOG_BASE_SZ);

	___log_trace_internal(trc, us, func, line, LOG_TYPE_TRACE, 0);
}

void
__log_infolog_internal_log64_32(log_header_t *hdr, uint32_t us, const char *fmt,
				const char *func, uint16_t line, int64_t arg0)
{
	uint32_t idx = 0;
	log_log64_32_t *info = NULL;

	idx = hdr->head_idx;
	__log_infolog_internal_idxadd(hdr, LOG_BLKS(log_log64_32_t));
	info = (log_log64_32_t*)(hdr->log_top + idx * LOG_BASE_SZ);
	___log_trace_internal(&(info->header), us, func, line, LOG_TYPE_INFO64_32, 0);
	info->format = fmt;
	info->arg[0] = arg0;
}

void
__log_infolog_internal_log64_64(log_header_t *hdr, uint32_t us, const char *fmt,
				const char *func, uint16_t line,
				int64_t arg0, int64_t arg1, int64_t arg2,
				int64_t arg3, int64_t arg4)
{
	uint32_t idx = 0;
	log_log64_64_t *info = NULL;

	idx = hdr->head_idx;
	__log_infolog_internal_idxadd(hdr, LOG_BLKS(log_log64_64_t));
	info = (log_log64_64_t*)(hdr->log_top + idx * LOG_BASE_SZ);
	___log_trace_internal(&(info->header), us, func, line, LOG_TYPE_INFO64_32, 0);
	info->format = fmt;
	info->arg[0] = arg0;
	info->arg[1] = arg1;
	info->arg[2] = arg2;
	info->arg[3] = arg3;
	info->arg[4] = arg4;
}

