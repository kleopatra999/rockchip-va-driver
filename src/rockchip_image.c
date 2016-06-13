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

static VAImageFormat rockchip_SupportedImageFormat[] = {
    { VA_FOURCC_IYUV, VA_LSB_FIRST, 12, },
    { VA_FOURCC_YV12, VA_LSB_FIRST, 12, },
    { VA_FOURCC_NV12, VA_LSB_FIRST, 12, },
};

VAStatus rockchip_QueryImageFormats(
    VADriverContextP ctx,
    VAImageFormat *format_list,
    int *num_formats
)
{
    *num_formats = ARRAY_SIZE(rockchip_SupportedImageFormat);
    memcpy(format_list, rockchip_SupportedImageFormat,
           sizeof(rockchip_SupportedImageFormat));

    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_CreateImage(
    VADriverContextP ctx,
    VAImageFormat *format,
    int width,
    int height,
    VAImage *out_image
)
{
    INIT_DRIVER_DATA
    VAStatus va_status = VA_STATUS_ERROR_OPERATION_FAILED;

    if (!format || !out_image)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    out_image->image_id = VA_INVALID_ID;
    out_image->buf = VA_INVALID_ID;

    VAImageID image_id = object_heap_allocate(&driver_data->image_heap);
    if (image_id == VA_INVALID_ID) {
        va_status = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto error;
    }

    object_image_p obj_image = IMAGE(image_id);
    if (!obj_image) {
        va_status = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto error;
    }

    obj_image->ref_cnt = 1;

    VAImage * const image = &obj_image->image;
    image->image_id = image_id;
    image->buf = VA_INVALID_ID;

    switch (format->fourcc) {
    case VA_FOURCC_NV12:
        image->num_planes = 2;
        image->pitches[0] = width;
        image->offsets[0] = 0;
        image->pitches[1] = width;
        image->offsets[1] = width * height;
        image->data_size = width * height * 3 / 2;
        image->num_palette_entries = 0;
        image->entry_bytes = 0;
        image->component_order[0] = 'Y';
        image->component_order[1] = 'U';
        image->component_order[2] = 'V';
        image->component_order[3] = '\0';
        break;
    case VA_FOURCC_YV12:
    case VA_FOURCC_IYUV:
        image->num_planes = 3;
        image->pitches[0] = width;
        image->offsets[0] = 0;
        image->pitches[1] = width / 2;
        image->offsets[1] = width * height;
        image->pitches[2] = width / 2;
        image->offsets[2] = width * height * 5 / 4;
        image->data_size = width * height * 3 / 2;
        image->num_palette_entries = 0;
        image->entry_bytes = 0;
        image->component_order[0] = 'Y';
        image->component_order[1] = 'U';
        image->component_order[2] = 'V';
        image->component_order[3] = '\0';
        break;
    default:
        goto error;
    }

    va_status = rockchip_CreateBuffer(ctx, 0, VAImageBufferType,
                                      image->data_size, 1, NULL,
                                      &image->buf);
    if (va_status != VA_STATUS_SUCCESS)
        goto error;

    object_buffer_p obj_buffer = BUFFER(image->buf);
    if (!obj_buffer)
        goto error;

    image->image_id = image_id;
    image->format = *format;
    image->width = width;
    image->height = height;

    *out_image = *image;

    return VA_STATUS_SUCCESS;

error:
    rockchip_DestroyImage(ctx, image_id);
    return va_status;
}

VAStatus rockchip_DestroyImage(
    VADriverContextP ctx,
    VAImageID image_id
)
{
    INIT_DRIVER_DATA
    object_image_p obj_image = IMAGE(image_id);
    if (!obj_image)
        return VA_STATUS_ERROR_INVALID_IMAGE;

    ASSERT(obj_image->ref_cnt);

    --obj_image->ref_cnt;

    if (obj_image->ref_cnt)
        return VA_STATUS_SUCCESS;

    VABufferID buf = obj_image->image.buf;
    object_heap_free(&driver_data->image_heap, (object_base_p)obj_image);
    return rockchip_DestroyBuffer(ctx, buf);
}

VAStatus rockchip_DeriveImage(
    VADriverContextP ctx,
    VASurfaceID surface,
    VAImage *image
)
{
    INIT_DRIVER_DATA
    object_surface_p obj_surface = SURFACE(surface);
    if (!obj_surface)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    object_image_p obj_image = IMAGE(obj_surface->image.image_id);
    if (!obj_image)
        return VA_STATUS_ERROR_INVALID_IMAGE;

    *image = obj_image->image;
    ++obj_image->ref_cnt;

    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_SetImagePalette(
    VADriverContextP ctx,
    VAImageID image,
    unsigned char *palette
)
{
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus rockchip_GetImage(
    VADriverContextP ctx,
    VASurfaceID surface,
    int x,
    int y,
    unsigned int width,
    unsigned int height,
    VAImageID image
)
{
    INIT_DRIVER_DATA
    object_surface_p obj_surface = SURFACE(surface);
    if (!obj_surface)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    object_image_p obj_image = IMAGE(image);
    if (!obj_image)
        return VA_STATUS_ERROR_INVALID_IMAGE;

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus
rockchip_PutImage(
    VADriverContextP ctx,
    VASurfaceID surface,
    VAImageID image,
    int src_x,
    int src_y,
    unsigned int src_width,
    unsigned int src_height,
    int dest_x,
    int dest_y,
    unsigned int dest_width,
    unsigned int dest_height
)
{
    INIT_DRIVER_DATA
    object_surface_p obj_surface = SURFACE(surface);
    if (!obj_surface)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    object_image_p obj_image = IMAGE(image);
    if (!obj_image)
        return VA_STATUS_ERROR_INVALID_IMAGE;

    VAImage *src = &obj_image->image;
    VAImage *dst = &obj_surface->image;

    /**
     * TODO: support more format
     */
    if (src->format.fourcc != dst->format.fourcc) {
        return VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    void *src_buf, *dst_buf;

    rockchip_MapBuffer(ctx, src->buf, &src_buf);
    rockchip_MapBuffer(ctx, dst->buf, &dst_buf);

    memcpy(dst_buf, src_buf, src->data_size);

    rockchip_UnmapBuffer(ctx, src->buf);
    rockchip_UnmapBuffer(ctx, dst->buf);

    return VA_STATUS_SUCCESS;
}
