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

VAStatus rockchip_BeginPicture(
    VADriverContextP ctx,
    VAContextID context,
    VASurfaceID render_target
)
{
    return rockchip_PrepareEncode(ctx, context, render_target);
}

VAStatus rockchip_RenderPicture(
    VADriverContextP ctx,
    VAContextID context,
    VABufferID *buffers,
    int num_buffers
)
{
    INIT_DRIVER_DATA
    VAStatus vaStatus = VA_STATUS_SUCCESS;

    int i;
    for (i = 0; i < num_buffers; i++) {
        object_buffer_p obj_buffer = BUFFER(buffers[i]);

        switch (obj_buffer->type) {
        case VAEncSequenceParameterBufferType:
            vaStatus = rockchip_ProcessSPS(ctx, context, buffers[i]);
            break;
        case VAEncPictureParameterBufferType:
            vaStatus = rockchip_ProcessPPS(ctx, context, buffers[i]);
            break;
        case VAEncSliceParameterBufferType:
            vaStatus = rockchip_ProcessSliceParam(ctx, context, buffers[i]);
            break;
        case VAEncMiscParameterBufferType:
            vaStatus = rockchip_ProcessMiscParam(ctx, context, buffers[i]);
            break;
        case VAEncPackedHeaderParameterBufferType:
        case VAEncPackedHeaderDataBufferType:
            /**
             * Ignore these
             */
            break;
        default:
            vaStatus = VA_STATUS_ERROR_UNKNOWN;
        }
        if (vaStatus != VA_STATUS_SUCCESS) {
            break;
        }
    }

    return vaStatus;
}

VAStatus rockchip_EndPicture(
    VADriverContextP ctx,
    VAContextID context
)
{
    return rockchip_DoEncode(ctx, context);
}
