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

#include <libsharaku/debug/log.h>
#include <gtest/gtest.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

TEST(log, log) {
	log_header_t	logh;
	char		buffer[16 * LOG_BASE_SZ];

	init_log_header(&logh, 16, buffer);
	EXPECT_EQ(logh.version, LOG_VIRSION);
	EXPECT_EQ(logh.base_usec, 0);
	EXPECT_EQ(logh.log_blks, 16);
	EXPECT_EQ(logh.head_idx, 0);
	EXPECT_EQ(logh.tail_idx, 0);
	EXPECT_EQ(logh.log_top, buffer);

	log_infolog64(&logh, 0, "test");
	EXPECT_EQ(logh.head_idx, 2);
	EXPECT_EQ(logh.tail_idx, 2);
}
