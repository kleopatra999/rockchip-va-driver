/*
 * Copyright (c) 2016 Rockchip Electronics Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef V4l2_UTILS_H
#define V4l2_UTILS_H

#include "linux/videodev2.h"
#include "rk_vepu_plugin.h"

typedef struct enc_context {
    void *enc;
    int fd;
    int width;
    int height;

    void *coded_buffer;
    int coded_size;

    void *input_buffer[3];
    int input_size[3];

} enc_context_t, *enc_context_p;

enc_context_p v4l2_init(const char *device_path);
enc_context_p v4l2_init_by_name(const char *name);
int v4l2_deinit(enc_context_p ctx);
int v4l2_reqbufs(enc_context_p ctx);
int v4l2_querybuf(enc_context_p ctx);
int v4l2_s_fmt(enc_context_p ctx);
int v4l2_streamon(enc_context_p ctx);
int v4l2_streamoff(enc_context_p ctx);
int v4l2_s_ext_ctrls(enc_context_p ctx, struct v4l2_ext_controls* ext_ctrls);
int v4l2_s_parm(enc_context_p ctx, struct v4l2_streamparm *parm);
int v4l2_qbuf_input(enc_context_p ctx, void *data, int size);
int v4l2_qbuf_output(enc_context_p ctx);
int v4l2_dqbuf_input(enc_context_p ctx);
int v4l2_dqbuf_output(enc_context_p ctx);

#endif /* V4l2_UTILS_H */
