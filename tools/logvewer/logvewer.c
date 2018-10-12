/* --
 *
 * MIT License
 * 
 * Copyright (c) 2018 Abe Takafumi
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <libsharaku/debug/log.h>

static int __log_mmap_fd;
static log_header_t *__log;
static int __bin_mmap_fd;
static char *__bin;
static int64_t __bin_offset_base = 0;
static int64_t __addr_base = 0;

static void
__init(char *log_filename, char *bin_filename)
{
	struct stat stat_buf;

	// ログをメモリにマッピングする
	__log_mmap_fd = open(log_filename, O_RDONLY);
	if (__log_mmap_fd < 0) {
		printf("open error. errno=%d", errno);
		exit(1);
	}
	fstat(__log_mmap_fd, &stat_buf);
	__log = (log_header_t*)mmap(NULL, stat_buf.st_size, PROT_READ, MAP_SHARED, __log_mmap_fd, 0);
	if (__log == NULL) {
		printf("mmap error. errno=%d", errno);
		exit(1);
	}

	// バイナリをマッピングする
	__bin_mmap_fd = open(bin_filename, O_RDONLY);
	if (__bin_mmap_fd < 0) {
		printf("open error. errno=%d", errno);
		exit(1);
	}
	fstat(__bin_mmap_fd, &stat_buf);
	__bin = (char*)mmap(NULL, stat_buf.st_size, PROT_READ, MAP_SHARED, __bin_mmap_fd, 0);
	if (__bin == NULL) {
		printf("mmap error. errno=%d", errno);
		exit(1);
	}

	// バイナリから.rodataセクションのアドレスを取り出す。
	// 変換にはobjdumpを使用する。
	char command[256];
	char	buf[256];
	FILE	*fp = 0;

	sprintf(command, "objdump -h %s | grep .rodata | sed -e 's/\\s\\+/ /g' | cut -d' ' -f 6", bin_filename);
	fp = popen(command, "r");
	fgets(buf, 256, fp);
	__addr_base = strtol(buf, NULL, 16);
	pclose(fp);
	sprintf(command, "objdump -h %s | grep .rodata | sed -e 's/\\s\\+/ /g' | cut -d' ' -f 7", bin_filename);
	fp = popen(command, "r");
	fgets(buf, 256, fp);
	__bin_offset_base = strtol(buf, NULL, 16);
	pclose(fp);
}

const char *
fmtaddr2addr(const char *fmt)
{
	int64_t offset = __bin_offset_base + (fmt - (char*)__addr_base);
	return (const char *)(__bin + offset);
}

void
__trace_print(log_trace_t *trace)
{
	time_t sec = (__log->base_usec + trace->usec) / 1000000;
	int32_t usec = (__log->base_usec + trace->usec) % 1000000;

	if (trace->usec) {
		struct tm *time_st = localtime(&sec);

		printf("%02d.%02d.%02d-%02d:%02d:%02d.%06d %24s:%-4d ",
			time_st->tm_year + 1900, time_st->tm_mon + 1, time_st->tm_mday,
			time_st->tm_hour, time_st->tm_min, time_st->tm_sec, usec,
			fmtaddr2addr(trace->func), trace->line);
	} else {
		printf("00.00.00-00:00:00.000000 %24s:%-4d ",
			fmtaddr2addr(trace->func), trace->line);
	}
}

void
__infolog_print(char *base_addr, uint32_t start_idx, uint32_t end_idx)
{
	uint32_t idx;
	uint32_t blks = 0;
	log_trace_t	*trace;
	log_log64_32_t	*_log64_32;
	log_log64_64_t	*_log64_64;

	for (idx = start_idx; idx < end_idx; idx += blks) {
		trace = (log_trace_t*)(base_addr + idx * LOG_BASE_SZ);
		switch (trace->type) {
		case LOG_TYPE_TRACE:
			__trace_print(trace);
			blks = LOG_BLKS(log_trace_t);
			break;
		case LOG_TYPE_INFO64_32:
			_log64_32 = (log_log64_32_t*)trace;
			__trace_print(trace);
			printf(fmtaddr2addr(_log64_32->format), _log64_32->arg[0]);
			blks = LOG_BLKS(log_log64_32_t);
			break;
		case LOG_TYPE_INFO64_64:
			_log64_64 = (log_log64_64_t*)trace;
			__trace_print(trace);
			printf(fmtaddr2addr(_log64_64->format), _log64_64->arg[0], _log64_64->arg[1], _log64_64->arg[2], _log64_64->arg[3], _log64_64->arg[4]);
			blks = LOG_BLKS(log_log64_64_t);
			break;
		default:
			blks = LOG_BLKS(log_trace_t);
		}
		printf("\n");
	}
}

void
_infolog_print(void)
{
	char *base_addr = ((char *)__log) + sizeof(log_header_t);

	// 1周回っていたら後半を出力。
	if (__log->head_idx !=  __log->tail_idx) {
		__infolog_print(base_addr, __log->head_idx, __log->tail_idx);
	}
	// 先頭から出力
	__infolog_print(base_addr, 0, __log->head_idx);
}

int
main(int argc, char *argv[])
{
	__init(argv[2], argv[1]);
	_infolog_print();
	return 0;
}

