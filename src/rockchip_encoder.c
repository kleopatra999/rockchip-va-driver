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

#define DEV_NAME_RK3288_NEW     "rockchip-vpu-enc"
#define DEV_NAME_RK3288_LEGACY  "rk3288-vpu-enc"

/**
 * TODO: Seperate h264 encoder from this
 */

VAStatus rockchip_DeinitEncoder(
        VADriverContextP ctx,
        VAContextID context)
{
    INIT_DRIVER_DATA
    object_context_p obj_context;

    obj_context = CONTEXT(context);
    ASSERT(obj_context);

    v4l2_streamoff(obj_context->enc_ctx);
    v4l2_deinit(obj_context->enc_ctx);

    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_InitEncoder(
        VADriverContextP ctx,
        VAContextID context)
{
    INIT_DRIVER_DATA
    object_context_p obj_context;

    obj_context = CONTEXT(context);
    ASSERT(obj_context);

    obj_context->enc_ctx = v4l2_init_by_name(DEV_NAME_RK3288_NEW);
    if (!obj_context->enc_ctx) {
        obj_context->enc_ctx = v4l2_init_by_name(DEV_NAME_RK3288_LEGACY);
        if (!obj_context->enc_ctx)
            return VA_STATUS_ERROR_UNKNOWN;
    }

    obj_context->enc_ctx->width = obj_context->picture_width;
    obj_context->enc_ctx->height = obj_context->picture_height;
    obj_context->streaming = 0;

    if (v4l2_s_fmt(obj_context->enc_ctx) < 0)
        goto failed_v4l2;

    if (v4l2_reqbufs(obj_context->enc_ctx) < 0)
        goto failed_v4l2;

    if (v4l2_querybuf(obj_context->enc_ctx) < 0)
        goto failed_v4l2;

    return VA_STATUS_SUCCESS;

failed_v4l2:
    rockchip_DeinitEncoder(ctx, context);

    return VA_STATUS_ERROR_UNKNOWN;
}
VAStatus rockchip_PrepareEncode(
        VADriverContextP ctx,
        VAContextID context,
        VASurfaceID render_target)
{
    INIT_DRIVER_DATA
    object_context_p obj_context;
    object_surface_p obj_surface;

    obj_context = CONTEXT(context);
    ASSERT(obj_context);

    obj_surface = SURFACE(render_target);
    ASSERT(obj_surface);

    obj_context->current_render_target = obj_surface->base.id;
    obj_surface->context_id = context;

    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_ProcessSPS(VADriverContextP ctx, VAContextID context, VABufferID buffer)
{
    INIT_DRIVER_DATA
    object_context_p obj_context;
    object_buffer_p obj_buffer;

    obj_context = CONTEXT(context);
    ASSERT(obj_context);
   
    obj_buffer = BUFFER(buffer);
    ASSERT(obj_buffer);

    ASSERT(obj_buffer->type == VAEncSequenceParameterBufferType);
    ASSERT(obj_buffer->num_elements == 1);
    ASSERT(obj_buffer->buffer_size == sizeof(VAEncSequenceParameterBufferH264));

    if ((obj_buffer->num_elements != 1) ||
        (obj_buffer->buffer_size != sizeof(VAEncSequenceParameterBufferH264))) {
        return VA_STATUS_ERROR_UNKNOWN;
    }

    VAEncSequenceParameterBufferH264 *sps;
    sps = (VAEncSequenceParameterBufferH264 *) obj_buffer->buffer_data;

    struct v4l2_ext_controls *ext_ctrls;

    obj_context->ctrl[0].id = V4L2_CID_PRIVATE_ROCKCHIP_VAENC_SPS;
    obj_context->ctrl[0].ptr = sps;
    obj_context->ctrl[0].size = sizeof(*sps);

    ext_ctrls = calloc(1, sizeof(*ext_ctrls));
    ext_ctrls->ctrl_class = 0;
    ext_ctrls->count = 1;
    ext_ctrls->controls = &obj_context->ctrl[0];

    v4l2_s_ext_ctrls(obj_context->enc_ctx, ext_ctrls);

    free(ext_ctrls);

    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_ProcessPPS(VADriverContextP ctx, VAContextID context, VABufferID buffer)
{
    INIT_DRIVER_DATA
    object_context_p obj_context;
    object_buffer_p obj_buffer;

    obj_context = CONTEXT(context);
    ASSERT(obj_context);
   
    obj_buffer = BUFFER(buffer);
    ASSERT(obj_buffer);

    ASSERT(obj_buffer->type == VAEncPictureParameterBufferType);

    if ((obj_buffer->num_elements != 1) ||
        (obj_buffer->buffer_size != sizeof(VAEncPictureParameterBufferH264))) {
        return VA_STATUS_ERROR_UNKNOWN;
    }

    VAEncPictureParameterBufferH264 *pps;
    pps = (VAEncPictureParameterBufferH264 *) obj_buffer->buffer_data;

    struct v4l2_ext_controls *ext_ctrls;

    obj_context->ctrl[0].id = V4L2_CID_PRIVATE_ROCKCHIP_VAENC_PPS;
    obj_context->ctrl[0].ptr = pps;
    obj_context->ctrl[0].size = sizeof(*pps);

    ext_ctrls = calloc(1, sizeof(*ext_ctrls));
    ext_ctrls->ctrl_class = 0;
    ext_ctrls->count = 1;
    ext_ctrls->controls = &obj_context->ctrl[0];

    v4l2_s_ext_ctrls(obj_context->enc_ctx, ext_ctrls);
    free(ext_ctrls);

    obj_context->h264_params.coded_buf = pps->coded_buf;

    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_ProcessSliceParam(VADriverContextP ctx, VAContextID context, VABufferID buffer)
{
    INIT_DRIVER_DATA
    object_context_p obj_context;
    object_buffer_p obj_buffer;

    obj_context = CONTEXT(context);
    ASSERT(obj_context);
   
    obj_buffer = BUFFER(buffer);
    ASSERT(obj_buffer);

    ASSERT(obj_buffer->type == VAEncSliceParameterBufferType);

    VAEncSliceParameterBuffer *slice_param;
    slice_param = (VAEncSliceParameterBuffer *) obj_buffer->buffer_data;

    /**
     * TODO: Convert slice_param into encode_params_h264
     */

    struct v4l2_ext_controls *ext_ctrls;

    obj_context->ctrl[0].id = V4L2_CID_PRIVATE_ROCKCHIP_VAENC_SLICE;
    obj_context->ctrl[0].ptr = slice_param;
    obj_context->ctrl[0].size = sizeof(*slice_param);

    ext_ctrls = calloc(1, sizeof(*ext_ctrls));
    ext_ctrls->ctrl_class = 0;
    ext_ctrls->count = 1;
    ext_ctrls->controls = &obj_context->ctrl[0];

    v4l2_s_ext_ctrls(obj_context->enc_ctx, ext_ctrls);
    free(ext_ctrls);

    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_ProcessMiscParam(VADriverContextP ctx, VAContextID context, VABufferID buffer)
{
    INIT_DRIVER_DATA
    object_context_p obj_context;
    object_buffer_p obj_buffer;

    obj_context = CONTEXT(context);
    ASSERT(obj_context);
   
    obj_buffer = BUFFER(buffer);
    ASSERT(obj_buffer);

    VAEncMiscParameterFrameRate *frame_rate;
    VAEncMiscParameterRateControl *rate_control;
#if 0
    VAEncMiscParameterAIR *air;
    VAEncMiscParameterMaxSliceSize *max_slice_size;
    VAEncMiscParameterHRD *hrd;
#endif

    ASSERT(obj_buffer->type == VAEncMiscParameterBufferType);

    VAEncMiscParameterBuffer *misc_param;
    misc_param = (VAEncMiscParameterBuffer *) obj_buffer->buffer_data;

    /**
     * TODO: Do real controls
     */
    switch (misc_param->type) {
    case VAEncMiscParameterTypeFrameRate:
        frame_rate = (VAEncMiscParameterFrameRate *)obj_buffer->buffer_data;

	struct v4l2_streamparm parms;
	memset(&parms, 0, sizeof(parms));
	parms.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	// Note that we are provided "frames per second" but V4L2 expects "time per
	// frame"; hence we provide the reciprocal of the framerate here.
	parms.parm.output.timeperframe.numerator = 1;
	parms.parm.output.timeperframe.denominator = frame_rate->framerate;

	v4l2_s_parm(obj_context->enc_ctx, &parms);

        break;
    case VAEncMiscParameterTypeRateControl:
        rate_control = (VAEncMiscParameterRateControl *)obj_buffer->buffer_data;

        struct v4l2_ext_controls *ext_ctrls;

        obj_context->ctrl[0].id = V4L2_CID_PRIVATE_ROCKCHIP_VAENC_RC;
        obj_context->ctrl[0].ptr = rate_control;
        obj_context->ctrl[0].size = sizeof(*rate_control);

        ext_ctrls = calloc(1, sizeof(*ext_ctrls));
        ext_ctrls->ctrl_class = 0;
        ext_ctrls->count = 1;
        ext_ctrls->controls = &obj_context->ctrl[0];

        v4l2_s_ext_ctrls(obj_context->enc_ctx, ext_ctrls);

        break;
    case VAEncMiscParameterTypeAIR:
#if 0
        air = (VAEncMiscParameterAIR *)obj_buffer->buffer_data;
#endif
        break;
    case VAEncMiscParameterTypeMaxSliceSize:
#if 0
        max_slice_size = (VAEncMiscParameterMaxSliceSize *)obj_buffer->buffer_data;
#endif
        break;
    case VAEncMiscParameterTypeHRD:
#if 0
        hrd = (VAEncMiscParameterHRD *)obj_buffer->buffer_data;
#endif
        break;
    default:
        return VA_STATUS_ERROR_UNKNOWN;
        break;
    }

    return VA_STATUS_SUCCESS;
}
struct timeval last_tv;
struct timeval tv;

void log_time(char *msg)
{
#define TIME_TO_MS(tv) (tv.tv_sec * 1000 + tv.tv_usec / 1000)
#define DURATION(tv1, tv2) (TIME_TO_MS(tv2) - TIME_TO_MS(tv1))

    if (getenv("LOG_TIME")) {
        gettimeofday(&tv, NULL);
        if (msg)
            printf("\n%s: %ld ms\n", msg, DURATION(last_tv, tv));
        last_tv = tv;
    }
}

VAStatus rockchip_DoEncode(
    VADriverContextP ctx,
    VAContextID context
)
{
    INIT_DRIVER_DATA
    object_context_p obj_context;
    object_surface_p obj_surface;

    obj_context = CONTEXT(context);
    ASSERT(obj_context);

    obj_surface = SURFACE(obj_context->current_render_target);
    ASSERT(obj_surface);

    object_buffer_p obj_buffer = BUFFER(obj_surface->image.buf);
    ASSERT(obj_buffer);

    if (!obj_context->streaming) {
        if (v4l2_streamon(obj_context->enc_ctx) < 0)
            return VA_STATUS_ERROR_UNKNOWN;

        if (v4l2_qbuf_output(obj_context->enc_ctx) < 0)
            return VA_STATUS_ERROR_UNKNOWN;

        obj_context->streaming = 1;
    } else {
        log_time("before dq input");
        v4l2_dqbuf_input(obj_context->enc_ctx);
    }

    log_time("start encode");
    v4l2_qbuf_input(obj_context->enc_ctx, obj_buffer->buffer_data,
            obj_buffer->buffer_size);
    log_time("after queue input");

    obj_surface->coded_buffer = obj_context->h264_params.coded_buf;

    obj_context->current_render_target = -1;

    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_SyncEncoder(
        VADriverContextP ctx,
        VASurfaceID render_target)
{
    INIT_DRIVER_DATA
    object_context_p obj_context;
    object_surface_p obj_surface;

    obj_surface = SURFACE(render_target);
    ASSERT(obj_surface);

    obj_context = CONTEXT(obj_surface->context_id);
    ASSERT(obj_context);

    log_time("before dque out\n");
    v4l2_dqbuf_output(obj_context->enc_ctx);
    log_time("after encode");

    object_buffer_p obj_buffer = BUFFER(obj_surface->coded_buffer);
    ASSERT(obj_buffer);

    coded_buffer_segment_p segment =
        (coded_buffer_segment_p) obj_buffer->buffer_data;

    memcpy(segment->base.buf, obj_context->enc_ctx->coded_buffer,
            obj_context->enc_ctx->coded_size);
    segment->base.size = obj_context->enc_ctx->coded_size;

    v4l2_qbuf_output(obj_context->enc_ctx);

    obj_surface->context_id = VA_INVALID_ID;
    obj_surface->coded_buffer = VA_INVALID_ID;

    return VA_STATUS_SUCCESS;
}
