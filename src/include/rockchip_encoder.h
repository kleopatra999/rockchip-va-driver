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

#ifndef ROCKCHIP_ENCODER_H
#define ROCKCHIP_ENCODER_H

#include <rockchip_drv_video.h>

typedef struct encode_params_h264 {
    VABufferID      coded_buf;

    VASurfaceID     reference_picture;
    VASurfaceID     reconstructed_picture;

    /**
     * TODO: save more params
     */

} encode_params_h264_t, *encode_params_h264_p;

VAStatus rockchip_InitEncoder(VADriverContextP ctx, VAContextID context);

VAStatus rockchip_DeinitEncoder(VADriverContextP ctx, VAContextID context);

VAStatus rockchip_PrepareEncode(VADriverContextP ctx, VAContextID context, VASurfaceID render_target);

VAStatus rockchip_ProcessSPS(VADriverContextP ctx, VAContextID context, VABufferID buffer);

VAStatus rockchip_ProcessPPS(VADriverContextP ctx, VAContextID context, VABufferID buffer);

VAStatus rockchip_ProcessSliceParam(VADriverContextP ctx, VAContextID context, VABufferID buffer);

VAStatus rockchip_ProcessMiscParam(VADriverContextP ctx, VAContextID context, VABufferID buffer);

VAStatus rockchip_DoEncode(VADriverContextP ctx, VAContextID context);

VAStatus rockchip_SyncEncoder(VADriverContextP ctx, VASurfaceID render_target);

#endif /* ROCKCHIP_ENCODER_H */
