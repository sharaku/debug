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

#ifndef ____LOG_H
#define ____LOG_H

#ifdef __cplusplus
	#ifndef CPP_SRC
		#define CPP_SRC(x) x
	#endif
#else
	#ifndef CPP_SRC
		#define CPP_SRC(x)
	#endif
#endif

#include <stdint.h>
#include <string.h>

CPP_SRC(extern "C" {)

enum {
	LOG_TYPE_TRACE,
	LOG_TYPE_INFO64_32,
	LOG_TYPE_INFO64_64,
};

#define LOG_VIRSION (0)
#define LOG_BASE_SZ	16
#define LOG_BLKS(type) (sizeof(type) / LOG_BASE_SZ)

typedef struct log_header
{
	uint32_t		version;	// ログバージョン
	uint32_t		padding4;	// padding
	int64_t			base_usec;	// ログのベース時間
	uint32_t		log_blks;	// 
	uint32_t		head_idx;	// 
	uint32_t		tail_idx;	// 
	char			*log_top;	// 
} log_header_t;

// 16byte
typedef struct log_trace {
	const char	*func;		// 0x00
	uint32_t	usec;		// 0x08
	uint16_t	line;		// 0x0c
	uint8_t		type;		// 0x0e
	uint8_t		u8;		// 0x0f
} log_trace_t;

// 32byte
typedef struct log_log64_32
{
	log_trace_t	header;		// 0x00
	const char	*format;	// 0x10
	int64_t		arg[1];		// 0x18 - 0x20
} log_log64_32_t;

// 64byte
typedef struct log_log64_64
{
	log_trace_t	header;		// 0x00
	const char	*format;	// 0x10
	int64_t		arg[5];		// 0x18 - 0x40
} log_log64_64_t;

union __union_log
{
	log_trace_t	trace;
	log_log64_32_t	log64_32;
	log_log64_64_t	log64_64;
};
#define LOG_MAXBLKS (sizeof(log_log64_64_t) / LOG_BASE_SZ)

// メモリの確保・管理はlog機能に含まれてはいない。
// よってバッファを登録するのみ。
// バッファの開放は、任意である。その場合はログ機能にアクセスしないこと。
static inline void
init_log_header(log_header_t *loghp, int64_t log_blks, char *buffer)
{
	memset(loghp, 0, sizeof *loghp);
	loghp->version		= LOG_VIRSION;
	loghp->base_usec	= 0;
	loghp->log_blks		= log_blks;
	loghp->head_idx		= 0;
	loghp->tail_idx		= 0;
	loghp->log_top		= buffer;
}


// ログ採取
void __log_trace_internal(log_header_t *hdr, uint32_t us, const char *func, uint16_t line);
void __log_infolog_internal_log64_32(log_header_t *hdr, uint32_t us, const char *fmt, const char *func, uint16_t line, int64_t arg0);
void __log_infolog_internal_log64_64(log_header_t *hdr, uint32_t us, const char *fmt, const char *func, uint16_t line, int64_t arg0, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4);

#define ____infolog64_0(hdr, us, func, line, fmt) \
		__log_infolog_internal_log64_32(hdr, us, fmt, func, line, 0)
#define ____infolog64_1(hdr, us, func, line, fmt, a1) \
		__log_infolog_internal_log64_32(hdr, us, fmt, func, line, (int64_t)a1)
#define ____infolog64_2(hdr, us, func, line, fmt, a1, a2) \
		__log_infolog_internal_log64_64(hdr, us, fmt, func, line, (int64_t)a1, (int64_t)a2, 0, 0, 0)
#define ____infolog64_3(hdr, us, func, line, fmt, a1, a2, a3) \
		__log_infolog_internal_log64_64(hdr, us, fmt, func, line, (int64_t)a1, (int64_t)a2, (int64_t)a3, 0, 0)
#define ____infolog64_4(hdr, us, func, line, fmt, a1, a2, a3, a4) \
		__log_infolog_internal_log64_64(hdr, us, fmt, func, line, (int64_t)a1, (int64_t)a2, (int64_t)a3, (int64_t)a4, 0)
#define ____infolog64_5(hdr, us, func, line, fmt, a1, a2, a3, a4, a5) \
		__log_infolog_internal_log64_64(hdr, us, fmt, func, line, (int64_t)a1, (int64_t)a2, (int64_t)a3, (int64_t)a4, (int64_t)a5)

#define __GET_FUNC6_NAME(_0, _1, _2, _3, _4, _5, NAME, ...) NAME
#define ____infolog64(hdr, us, ...)		\
	__GET_FUNC6_NAME(__VA_ARGS__, ____infolog64_5, ____infolog64_4, ____infolog64_3,	\
			  ____infolog64_2, ____infolog64_1, ____infolog64_0)			\
			  (hdr, us, __func__, __LINE__, ##__VA_ARGS__)

// ログ採取インタフェース。
// 実際に使用する場合は log_infolog64() をラッピングして使用する。
// 第2引数に指定する値は0もしくは現在時間のusecをlog_usec2logtime()で
// 変換したものを指定する。
#define log_usec2logtime(hdr, usec) ((usec) - (hdr)->base_usec)
#define log_infolog64(hdr, us, fmt, ...) ____infolog64(hdr, us, fmt, ##__VA_ARGS__)

CPP_SRC(})

#endif /* ____LOG_H */

