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

#ifndef ROCKCHIP_SURFACE_H
#define ROCKCHIP_SURFACE_H

#include <rockchip_drv_video.h>

typedef struct object_surface {
    struct object_base  base;
    VAImage             image;
    VAContextID         context_id;
} object_surface_t, *object_surface_p;

VAStatus rockchip_CreateSurfaces(VADriverContextP ctx, int width, int height, int format, int num_surfaces, VASurfaceID *surfaces);

VAStatus rockchip_DestroySurfaces(VADriverContextP ctx, VASurfaceID *surface_list, int num_surfaces);

VAStatus rockchip_SyncSurface(VADriverContextP ctx, VASurfaceID render_target);

VAStatus rockchip_QuerySurfaceStatus(VADriverContextP ctx, VASurfaceID render_target, VASurfaceStatus *status);

VAStatus rockchip_PutSurface(VADriverContextP ctx, VASurfaceID surface, void *draw, short srcx, short srcy, unsigned short srcw, unsigned short srch, short destx, short desty, unsigned short destw, unsigned short desth, VARectangle *cliprects, unsigned int number_cliprects, unsigned int flags);

VAStatus rockchip_LockSurface(VADriverContextP ctx, VASurfaceID surface, unsigned int *fourcc, unsigned int *luma_stride, unsigned int *chroma_u_stride, unsigned int *chroma_v_stride, unsigned int *luma_offset, unsigned int *chroma_u_offset, unsigned int *chroma_v_offset, unsigned int *buffer_name, void **buffer);

VAStatus rockchip_UnlockSurface(VADriverContextP ctx, VASurfaceID surface);

#endif /* ROCKCHIP_SURFACE_H */
