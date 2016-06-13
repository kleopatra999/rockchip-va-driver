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

#include "rockchip_drv_video.h"

VAStatus rockchip_CreateSurfaces(
    VADriverContextP ctx,
    int width,
    int height,
    int format,
    int num_surfaces,
    VASurfaceID *surfaces       /* out */
)
{
    INIT_DRIVER_DATA
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    int i;

    /* We only support one format */
    if (VA_RT_FORMAT_YUV420 != format) {
        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    for (i = 0; i < num_surfaces; i++) {
        int surfaceID = object_heap_allocate( &driver_data->surface_heap );
        object_surface_p obj_surface = SURFACE(surfaceID);
        if (NULL == obj_surface) {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            break;
        }
        surfaces[i] = surfaceID;

        /**
         * TODO: Support multi formats
         */
        VAImageFormat image_format = { VA_FOURCC_NV12, VA_LSB_FIRST, 12 };
        vaStatus = rockchip_CreateImage(ctx, &image_format,
                width, height, &obj_surface->image);
        if (VA_STATUS_SUCCESS != vaStatus)
            break;

        obj_surface->context_id = VA_INVALID_ID;
    }

    /* Error recovery */
    if (VA_STATUS_SUCCESS != vaStatus) {
        /* surfaces[i-1] was the last successful allocation */
        for(; i--; ) {
            object_surface_p obj_surface = SURFACE(surfaces[i]);
            surfaces[i] = VA_INVALID_SURFACE;
            ASSERT(obj_surface);
            object_heap_free( &driver_data->surface_heap, (object_base_p) obj_surface);
        }
    }

    return vaStatus;
}

VAStatus rockchip_DestroySurfaces(
    VADriverContextP ctx,
    VASurfaceID *surface_list,
    int num_surfaces
)
{
    INIT_DRIVER_DATA
    int i;
    for(i = num_surfaces; i--; ) {
        object_surface_p obj_surface = SURFACE(surface_list[i]);
        ASSERT(obj_surface);

        rockchip_DestroyImage(ctx, obj_surface->image.image_id);

        object_heap_free( &driver_data->surface_heap,
                          (object_base_p) obj_surface);
    }
    return VA_STATUS_SUCCESS;
}
VAStatus rockchip_SyncSurface(
    VADriverContextP ctx,
    VASurfaceID render_target
)
{
    return rockchip_SyncEncoder(ctx, render_target);
}

VAStatus rockchip_QuerySurfaceStatus(
    VADriverContextP ctx,
    VASurfaceID render_target,
    VASurfaceStatus *status /* out */
)
{
    INIT_DRIVER_DATA
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    object_surface_p obj_surface;

    obj_surface = SURFACE(render_target);
    ASSERT(obj_surface);

    *status = VASurfaceReady;

    return vaStatus;
}

VAStatus rockchip_PutSurface(
    VADriverContextP ctx,
    VASurfaceID surface,
    void *draw, /* X Drawable */
    short srcx,
    short srcy,
    unsigned short srcw,
    unsigned short srch,
    short destx,
    short desty,
    unsigned short destw,
    unsigned short desth,
    VARectangle *cliprects, /* client supplied clip list */
    unsigned int number_cliprects, /* number of clip rects in the clip list */
    unsigned int flags /* de-interlacing flags */
)
{
    /* TODO */
    return VA_STATUS_ERROR_UNKNOWN;
}

VAStatus rockchip_LockSurface(
    VADriverContextP ctx,
    VASurfaceID surface,
    unsigned int *fourcc, /* following are output argument */
    unsigned int *luma_stride,
    unsigned int *chroma_u_stride,
    unsigned int *chroma_v_stride,
    unsigned int *luma_offset,
    unsigned int *chroma_u_offset,
    unsigned int *chroma_v_offset,
    unsigned int *buffer_name,
    void **buffer
)
{
    /* TODO */
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus rockchip_UnlockSurface(
    VADriverContextP ctx,
    VASurfaceID surface
)
{
    /* TODO */
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}
