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

#ifndef ROCKCHIP_BUFFER_H
#define ROCKCHIP_BUFFER_H

#include <rockchip_drv_video.h>

typedef struct coded_buffer_segment
{
    VACodedBufferSegment base;
    unsigned int mapped;
} coded_buffer_segment_t, *coded_buffer_segment_p;

typedef struct object_buffer {
    struct object_base  base;
    VAContextID         va_context;
    VABufferType        type;
    void               *buffer_base;
    void               *buffer_data;
    unsigned int        buffer_size;
    unsigned int        max_num_elements;
    unsigned int        num_elements;
    unsigned int        ref_cnt;
} object_buffer_t, *object_buffer_p;

#define ALIGN(i, n)    (((i) + (n) - 1) & ~((n) - 1))
#define CODED_BUFFER_HEADER_SIZE    ALIGN(sizeof(coded_buffer_segment_t), 64)

VAStatus rockchip_CreateBuffer(VADriverContextP ctx, VAContextID context, VABufferType type, unsigned int size, unsigned int num_elements, void *data, VABufferID *buf_id);

VAStatus rockchip_DestroyBuffer(VADriverContextP ctx, VABufferID buffer_id);

VAStatus rockchip_BufferSetNumElements(VADriverContextP ctx, VABufferID buf_id, unsigned int num_elements);

VAStatus rockchip_MapBuffer(VADriverContextP ctx, VABufferID buf_id, void **pbuf);

VAStatus rockchip_UnmapBuffer(VADriverContextP ctx, VABufferID buf_id);

VAStatus rockchip_BufferInfo(VADriverContextP ctx, VABufferID buf_id, VABufferType *type, unsigned int *size, unsigned int *num_elements);

#endif /* ROCKCHIP_BUFFER_H */
