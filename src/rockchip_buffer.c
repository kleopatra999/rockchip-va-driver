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

VAStatus rockchip_allocate_buffer(object_buffer_p obj_buffer, int size)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;

    obj_buffer->buffer_base = malloc(size + 64);
    obj_buffer->buffer_data = obj_buffer->buffer_base + 64;
    obj_buffer->buffer_data -= ((unsigned int)obj_buffer->buffer_data) % 64;
    if (NULL == obj_buffer->buffer_data) {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    return vaStatus;
}

void rockchip_destroy_buffer(
    struct rockchip_driver_data *driver_data,
    object_buffer_p obj_buffer
)
{
    if (NULL != obj_buffer->buffer_base) {
        free(obj_buffer->buffer_base);
        obj_buffer->buffer_data = obj_buffer->buffer_base = NULL;
    }

    object_heap_free(&driver_data->buffer_heap, (object_base_p)obj_buffer);
}

VAStatus rockchip_CreateBuffer(
    VADriverContextP ctx,
    VAContextID context,
    VABufferType type,
    unsigned int size,
    unsigned int num_elements,
    void *data,
    VABufferID *buf_id
)
{
    INIT_DRIVER_DATA
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    VABufferID bufferID;
    object_buffer_p obj_buffer;

    /* Validate type */
    switch (type) {
    case VAImageBufferType:
    case VAEncCodedBufferType:
    case VAEncMiscParameterBufferType:
    case VAEncPackedHeaderDataBufferType:
    case VAEncPackedHeaderParameterBufferType:
    case VAEncPictureParameterBufferType:
    case VAEncSequenceParameterBufferType:
    case VAEncSliceParameterBufferType:
        /* Ok */
        break;
    default:
        vaStatus = VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
        return vaStatus;
    }

    bufferID = object_heap_allocate( &driver_data->buffer_heap );
    obj_buffer = BUFFER(bufferID);
    if (NULL == obj_buffer) {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return vaStatus;
    }

    obj_buffer->buffer_data = NULL;
    obj_buffer->ref_cnt = 1;

    /**
     * TODO: use dma buf for some buffer types
     */
    vaStatus = rockchip_allocate_buffer(obj_buffer, size * num_elements);
    if (VA_STATUS_SUCCESS == vaStatus) {
        obj_buffer->max_num_elements = num_elements;
        obj_buffer->num_elements = num_elements;
        obj_buffer->va_context = context;
        obj_buffer->type = type;
        obj_buffer->buffer_size = size * num_elements;
        if (type == VAEncCodedBufferType) {
            coded_buffer_segment_p segment =
                (coded_buffer_segment_p) obj_buffer->buffer_data;

            segment->base.size = size - CODED_BUFFER_HEADER_SIZE;
            segment->base.bit_offset = 0;
            segment->base.status = 0;
            segment->base.buf =
                obj_buffer->buffer_data + CODED_BUFFER_HEADER_SIZE;
            segment->base.next = NULL;
        } else if (data) {
            memcpy(obj_buffer->buffer_data, data, size * num_elements);
        }
    }

    if (VA_STATUS_SUCCESS == vaStatus) {
        *buf_id = bufferID;
    }

    return vaStatus;
}

VAStatus rockchip_DestroyBuffer(
    VADriverContextP ctx,
    VABufferID buffer_id
)
{
    INIT_DRIVER_DATA
    object_buffer_p obj_buffer = BUFFER(buffer_id);

    if (NULL == obj_buffer)
        return VA_STATUS_ERROR_INVALID_BUFFER;

    ASSERT(obj_buffer->ref_cnt);

    --obj_buffer->ref_cnt;

    if (!obj_buffer->ref_cnt)
        rockchip_destroy_buffer(driver_data, obj_buffer);

    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_BufferSetNumElements(
    VADriverContextP ctx,
    VABufferID buf_id,
    unsigned int num_elements
)
{
    INIT_DRIVER_DATA
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    object_buffer_p obj_buffer = BUFFER(buf_id);
    ASSERT(obj_buffer);

    if ((num_elements < 0) || (num_elements > obj_buffer->max_num_elements)) {
        vaStatus = VA_STATUS_ERROR_UNKNOWN;
    }
    if (VA_STATUS_SUCCESS == vaStatus) {
        obj_buffer->num_elements = num_elements;
    }

    return vaStatus;
}

VAStatus rockchip_MapBuffer(
    VADriverContextP ctx,
    VABufferID buf_id,
    void **pbuf
)
{
    INIT_DRIVER_DATA
    VAStatus vaStatus = VA_STATUS_ERROR_UNKNOWN;
    object_buffer_p obj_buffer = BUFFER(buf_id);
    if (NULL == obj_buffer) {
        vaStatus = VA_STATUS_ERROR_INVALID_BUFFER;
        return vaStatus;
    }

    if (NULL != obj_buffer->buffer_data) {
        *pbuf = obj_buffer->buffer_data;
        vaStatus = VA_STATUS_SUCCESS;

        ++obj_buffer->ref_cnt;
    }
    return vaStatus;
}

VAStatus rockchip_UnmapBuffer(
    VADriverContextP ctx,
    VABufferID buf_id
)
{
    INIT_DRIVER_DATA
    object_buffer_p obj_buffer = BUFFER(buf_id);
    ASSERT(obj_buffer);
    if (NULL == obj_buffer) {
        return VA_STATUS_ERROR_INVALID_BUFFER;
    }

    --obj_buffer->ref_cnt;
    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_BufferInfo(
    VADriverContextP ctx,
    VABufferID buf_id,
    VABufferType *type,
    unsigned int *size,
    unsigned int *num_elements
)
{
    INIT_DRIVER_DATA
    object_buffer_p obj_buffer = BUFFER(buf_id);
    if (!obj_buffer)
        return VA_STATUS_ERROR_INVALID_BUFFER;

    if (type)
        *type = obj_buffer->type;
    if (size)
        *size = obj_buffer->buffer_size / obj_buffer->num_elements;
    if (num_elements)
        *num_elements = obj_buffer->num_elements;
    return VA_STATUS_SUCCESS;
}
