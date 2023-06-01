/*
 * Copyright (C) 2022  Rockchip Electronics Co., Ltd.
 * Authors:
 *     YuQiaowei <cerf.yu@rock-chips.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "rga_mosaic_demo"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstddef>
#include <cmath>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "im2d.h"
#include "RgaUtils.h"

#include "utils.h"

int main(void) {
    int ret = 0;
    int64_t ts;
    int dst_width, dst_height, dst_format;
    int dst_buf_size;
    char *dst_buf;
    rga_buffer_t dst = {};
    im_rect dst_rect = {};
    rga_buffer_handle_t dst_handle;

    dst_width = 1280;
    dst_height = 720;
    dst_format = RK_FORMAT_RGBA_8888;

    dst_buf_size = dst_width * dst_height * get_bpp_from_format(dst_format);

    dst_buf = (char *)malloc(dst_buf_size);

    /* fill image data */
    if (0 != get_buf_from_file(dst_buf, dst_format, dst_width, dst_height, 0)) {
        printf("dst image write err\n");
        draw_rgba(dst_buf, dst_width, dst_height);
    }

    dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
    if (dst_handle == 0) {
        printf("importbuffer failed!\n");
        ret = -1;
        goto free_buf;
    }

    dst = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

    /*
     * mosaic a rectangular area on the dst image.
           dst_image
        --------------
        | --------   |
        | |mosaic|   |
        | --------   |
        --------------
     */

    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = 300;
    dst_rect.height = 200;

    ret = imcheck({}, dst, {}, dst_rect, IM_MOSAIC);
    if (IM_STATUS_NOERROR != ret) {
        printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
        goto release_buffer;
    }

    ret = immosaic(dst, dst_rect, IM_MOSAIC_32);
    if (ret == IM_STATUS_SUCCESS) {
        printf("%s running success! cost %ld us\n", LOG_TAG, get_cur_us() - ts);
    } else {
        printf("%s running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
        goto release_buffer;
    }

    printf("output [0x%x, 0x%x, 0x%x, 0x%x]\n", dst_buf[0], dst_buf[1], dst_buf[2], dst_buf[3]);
    output_buf_data_to_file(dst_buf, dst_format, dst_width, dst_height, 0);

release_buffer:
    if (dst_handle > 0)
        releasebuffer_handle(dst_handle);

free_buf:
    if (dst_buf)
        free(dst_buf);

    return 0;
}
