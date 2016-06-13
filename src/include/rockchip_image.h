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

#ifndef ROCKCHIP_IMAGE_H
#define ROCKCHIP_IMAGE_H

#include <rockchip_drv_video.h>

typedef struct object_image {
    struct object_base  base;
    VAImage             image;
    unsigned int        ref_cnt;
} object_image_t, *object_image_p;

VAStatus rockchip_QueryImageFormats(VADriverContextP ctx, VAImageFormat *format_list, int *num_formats);

VAStatus rockchip_CreateImage(VADriverContextP ctx, VAImageFormat *format, int width, int height, VAImage *image);

VAStatus rockchip_DestroyImage(VADriverContextP ctx, VAImageID image_id);

VAStatus rockchip_DeriveImage(VADriverContextP ctx, VASurfaceID surface, VAImage *image);

VAStatus rockchip_SetImagePalette(VADriverContextP ctx, VAImageID image, unsigned char *palette);

VAStatus rockchip_GetImage(VADriverContextP ctx, VASurfaceID surface, int x, int y, unsigned int width, unsigned int height, VAImageID image_id);

VAStatus rockchip_PutImage(VADriverContextP ctx, VASurfaceID surface, VAImageID image, int src_x, int src_y, unsigned int src_width, unsigned int src_height, int dest_x, int dest_y, unsigned int dest_width, unsigned int dest_height);

#endif /* ROCKCHIP_IMAGE_H */
