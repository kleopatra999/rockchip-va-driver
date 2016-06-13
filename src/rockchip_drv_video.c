/*
 * Copyright (c) 2016 Rockchip Electronics Co., Ltd.
 *
 * Based on libva's dummy_drv_video.
 *
 * Copyright (c) 2007 Intel Corporation. All Rights Reserved.
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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

VAStatus rockchip_QueryConfigProfiles(
    VADriverContextP ctx,
    VAProfile *profile_list,    /* out */
    int *num_profiles           /* out */
)
{
    int i = 0;

    profile_list[i++] = VAProfileH264Main;
    profile_list[i++] = VAProfileH264Baseline;
    profile_list[i++] = VAProfileH264ConstrainedBaseline;

    /* If the assert fails then ROCKCHIP_MAX_PROFILES needs to be bigger */
    ASSERT(i <= ROCKCHIP_MAX_PROFILES);
    *num_profiles = i;

    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_QueryConfigEntrypoints(
    VADriverContextP ctx,
    VAProfile profile,
    VAEntrypoint  *entrypoint_list, /* out */
    int *num_entrypoints        /* out */
)
{
    if (NULL == entrypoint_list) {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    if (NULL == num_entrypoints) {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    switch (profile) {
    case VAProfileH264Main:
    case VAProfileH264Baseline:
    case VAProfileH264ConstrainedBaseline:
        *num_entrypoints = 1;
        entrypoint_list[0] = VAEntrypointEncSlice;
        break;
    default:
        *num_entrypoints = 0;
        break;
    }

    /* If the assert fails then ROCKCHIP_MAX_ENTRYPOINTS needs to be bigger */
    ASSERT(*num_entrypoints <= ROCKCHIP_MAX_ENTRYPOINTS);
    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_GetConfigAttributes(
    VADriverContextP ctx,
    VAProfile profile,
    VAEntrypoint entrypoint,
    VAConfigAttrib *attrib_list,    /* in/out */
    int num_attribs
)
{
    int i;

    /**
     * TODO: Set real attrs
     */
    for (i = 0; i < num_attribs; i++) {
        switch (attrib_list[i].type) {
        case VAConfigAttribRTFormat:
            attrib_list[i].value = VA_RT_FORMAT_YUV420;
            break;
        case VAConfigAttribRateControl:
            attrib_list[i].value = VA_RC_VBR | VA_RC_CQP | VA_RC_VBR_CONSTRAINED | VA_RC_CBR | VA_RC_VCM | VA_RC_NONE;
            break;
        case VAConfigAttribEncPackedHeaders:
            attrib_list[i].value =
                VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_PICTURE |
                VA_ENC_PACKED_HEADER_SLICE | VA_ENC_PACKED_HEADER_MISC |
                VA_ENC_PACKED_HEADER_RAW_DATA;
            break;
        default:
            /* Do nothing */
            attrib_list[i].value = VA_ATTRIB_NOT_SUPPORTED;
            break;
        }
    }

    return VA_STATUS_SUCCESS;
}

static VAStatus rockchip_update_attribute(object_config_p obj_config, VAConfigAttrib *attrib)
{
    int i;
    /* Check existing attrbiutes */
    for(i = 0; obj_config->attrib_count < i; i++) {
        if (obj_config->attrib_list[i].type == attrib->type) {
            /* Update existing attribute */
            obj_config->attrib_list[i].value = attrib->value;
            return VA_STATUS_SUCCESS;
        }
    }
    if (obj_config->attrib_count < ROCKCHIP_MAX_CONFIG_ATTRIBUTES) {
        i = obj_config->attrib_count;
        obj_config->attrib_list[i].type = attrib->type;
        obj_config->attrib_list[i].value = attrib->value;
        obj_config->attrib_count++;
        return VA_STATUS_SUCCESS;
    }
    return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
}

VAStatus rockchip_CreateConfig(
    VADriverContextP ctx,
    VAProfile profile,
    VAEntrypoint entrypoint,
    VAConfigAttrib *attrib_list,
    int num_attribs,
    VAConfigID *config_id       /* out */
)
{
    INIT_DRIVER_DATA
    VAStatus vaStatus;
    int configID;
    object_config_p obj_config;
    int i;

    /* Validate profile & entrypoint */
    switch (profile) {
    case VAProfileH264Main:
    case VAProfileH264Baseline:
    case VAProfileH264ConstrainedBaseline:
        if (VAEntrypointEncSlice == entrypoint) {
            vaStatus = VA_STATUS_SUCCESS;
        } else {
            vaStatus = VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
        }
        break;
    default:
        vaStatus = VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
        break;
    }

    if (VA_STATUS_SUCCESS != vaStatus) {
        return vaStatus;
    }

    configID = object_heap_allocate( &driver_data->config_heap );
    obj_config = CONFIG(configID);
    if (NULL == obj_config) {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return vaStatus;
    }

    obj_config->profile = profile;
    obj_config->entrypoint = entrypoint;
    obj_config->attrib_list[0].type = VAConfigAttribRTFormat;
    obj_config->attrib_list[0].value = VA_RT_FORMAT_YUV420;
    obj_config->attrib_count = 1;

    for(i = 0; i < num_attribs; i++) {
        vaStatus = rockchip_update_attribute(obj_config, &(attrib_list[i]));
        if (VA_STATUS_SUCCESS != vaStatus) {
            break;
        }
    }

    /* Error recovery */
    if (VA_STATUS_SUCCESS != vaStatus) {
        object_heap_free( &driver_data->config_heap, (object_base_p) obj_config);
    } else {
        *config_id = configID;
    }

    return vaStatus;
}

VAStatus rockchip_DestroyConfig(
    VADriverContextP ctx,
    VAConfigID config_id
)
{
    INIT_DRIVER_DATA
    VAStatus vaStatus;
    object_config_p obj_config;

    obj_config = CONFIG(config_id);
    if (NULL == obj_config) {
        vaStatus = VA_STATUS_ERROR_INVALID_CONFIG;
        return vaStatus;
    }

    object_heap_free( &driver_data->config_heap, (object_base_p) obj_config);
    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_QueryConfigAttributes(
    VADriverContextP ctx,
    VAConfigID config_id,
    VAProfile *profile,     /* out */
    VAEntrypoint *entrypoint,   /* out */
    VAConfigAttrib *attrib_list,    /* out */
    int *num_attribs        /* out */
)
{
    INIT_DRIVER_DATA
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    object_config_p obj_config;
    int i;

    obj_config = CONFIG(config_id);
    ASSERT(obj_config);

    *profile = obj_config->profile;
    *entrypoint = obj_config->entrypoint;
    *num_attribs =  obj_config->attrib_count;
    for(i = 0; i < obj_config->attrib_count; i++) {
        attrib_list[i] = obj_config->attrib_list[i];
    }

    return vaStatus;
}

VAStatus rockchip_QuerySubpictureFormats(
    VADriverContextP ctx,
    VAImageFormat *format_list,        /* out */
    unsigned int *flags,       /* out */
    unsigned int *num_formats  /* out */
)
{
    /* TODO */
    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_CreateSubpicture(
    VADriverContextP ctx,
    VAImageID image,
    VASubpictureID *subpicture   /* out */
)
{
    /* TODO */
    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_DestroySubpicture(
    VADriverContextP ctx,
    VASubpictureID subpicture
)
{
    /* TODO */
    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_SetSubpictureImage(
    VADriverContextP ctx,
    VASubpictureID subpicture,
    VAImageID image
)
{
    /* TODO */
    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_SetSubpicturePalette(
    VADriverContextP ctx,
    VASubpictureID subpicture,
    /*
     * pointer to an array holding the palette data.  The size of the array is
     * num_palette_entries * entry_bytes in size.  The order of the components
     * in the palette is described by the component_order in VASubpicture struct
     */
    unsigned char *palette
)
{
    /* TODO */
    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_SetSubpictureChromakey(
    VADriverContextP ctx,
    VASubpictureID subpicture,
    unsigned int chromakey_min,
    unsigned int chromakey_max,
    unsigned int chromakey_mask
)
{
    /* TODO */
    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_SetSubpictureGlobalAlpha(
    VADriverContextP ctx,
    VASubpictureID subpicture,
    float global_alpha
)
{
    /* TODO */
    return VA_STATUS_SUCCESS;
}


VAStatus rockchip_AssociateSubpicture(
    VADriverContextP ctx,
    VASubpictureID subpicture,
    VASurfaceID *target_surfaces,
    int num_surfaces,
    short src_x, /* upper left offset in subpicture */
    short src_y,
    unsigned short src_width,
    unsigned short src_height,
    short dest_x, /* upper left offset in surface */
    short dest_y,
    unsigned short dest_width,
    unsigned short dest_height,
    /*
     * whether to enable chroma-keying or global-alpha
     * see VA_SUBPICTURE_XXX values
     */
    unsigned int flags
)
{
    /* TODO */
    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_DeassociateSubpicture(
    VADriverContextP ctx,
    VASubpictureID subpicture,
    VASurfaceID *target_surfaces,
    int num_surfaces
)
{
    /* TODO */
    return VA_STATUS_SUCCESS;
}

VAStatus rockchip_CreateContext(
    VADriverContextP ctx,
    VAConfigID config_id,
    int picture_width,
    int picture_height,
    int flag,
    VASurfaceID *render_targets,
    int num_render_targets,
    VAContextID *context        /* out */
)
{
    INIT_DRIVER_DATA
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    object_config_p obj_config;
    int i;

    obj_config = CONFIG(config_id);
    if (NULL == obj_config) {
        vaStatus = VA_STATUS_ERROR_INVALID_CONFIG;
        return vaStatus;
    }

    /* Validate flag */
    /* Validate picture dimensions */

    int contextID = object_heap_allocate( &driver_data->context_heap );
    object_context_p obj_context = CONTEXT(contextID);
    if (NULL == obj_context) {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return vaStatus;
    }

    obj_context->context_id  = contextID;
    *context = contextID;
    obj_context->current_render_target = -1;
    obj_context->config_id = config_id;
    obj_context->picture_width = picture_width;
    obj_context->picture_height = picture_height;
    obj_context->num_render_targets = num_render_targets;
    obj_context->render_targets = (VASurfaceID *) malloc(num_render_targets * sizeof(VASurfaceID));
    if (obj_context->render_targets == NULL) {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return vaStatus;
    }

    for(i = 0; i < num_render_targets; i++) {
        if (NULL == SURFACE(render_targets[i])) {
            vaStatus = VA_STATUS_ERROR_INVALID_SURFACE;
            break;
        }
        obj_context->render_targets[i] = render_targets[i];
    }
    obj_context->flags = flag;

    vaStatus = rockchip_InitEncoder(ctx, contextID);

    /* Error recovery */
    if (VA_STATUS_SUCCESS != vaStatus) {
        obj_context->context_id = -1;
        obj_context->config_id = -1;
        free(obj_context->render_targets);
        obj_context->render_targets = NULL;
        obj_context->num_render_targets = 0;
        obj_context->flags = 0;
        object_heap_free( &driver_data->context_heap, (object_base_p) obj_context);
    }

    return vaStatus;
}

VAStatus rockchip_DestroyContext(
    VADriverContextP ctx,
    VAContextID context
)
{
    INIT_DRIVER_DATA
    object_context_p obj_context = CONTEXT(context);
    ASSERT(obj_context);

    rockchip_DeinitEncoder(ctx, context);

    obj_context->context_id = -1;
    obj_context->config_id = -1;
    obj_context->picture_width = 0;
    obj_context->picture_height = 0;
    if (obj_context->render_targets) {
        free(obj_context->render_targets);
    }
    obj_context->render_targets = NULL;
    obj_context->num_render_targets = 0;
    obj_context->flags = 0;

    obj_context->current_render_target = -1;

    object_heap_free( &driver_data->context_heap, (object_base_p) obj_context);

    return VA_STATUS_SUCCESS;
}

/*
 * Query display attributes
 * The caller must provide a "attr_list" array that can hold at
 * least vaMaxNumDisplayAttributes() entries. The actual number of attributes
 * returned in "attr_list" is returned in "num_attributes".
 */
VAStatus rockchip_QueryDisplayAttributes (
    VADriverContextP ctx,
    VADisplayAttribute *attr_list,  /* out */
    int *num_attributes     /* out */
)
{
    /* TODO */
    return VA_STATUS_ERROR_UNKNOWN;
}

/*
 * Get display attributes
 * This function returns the current attribute values in "attr_list".
 * Only attributes returned with VA_DISPLAY_ATTRIB_GETTABLE set in the "flags" field
 * from vaQueryDisplayAttributes() can have their values retrieved.
 */
VAStatus rockchip_GetDisplayAttributes (
    VADriverContextP ctx,
    VADisplayAttribute *attr_list,  /* in/out */
    int num_attributes
)
{
    /* TODO */
    return VA_STATUS_ERROR_UNKNOWN;
}

/*
 * Set display attributes
 * Only attributes returned with VA_DISPLAY_ATTRIB_SETTABLE set in the "flags" field
 * from vaQueryDisplayAttributes() can be set.  If the attribute is not settable or
 * the value is out of range, the function returns VA_STATUS_ERROR_ATTR_NOT_SUPPORTED
 */
VAStatus rockchip_SetDisplayAttributes (
    VADriverContextP ctx,
    VADisplayAttribute *attr_list,
    int num_attributes
)
{
    /* TODO */
    return VA_STATUS_ERROR_UNKNOWN;
}

VAStatus rockchip_Terminate( VADriverContextP ctx )
{
    INIT_DRIVER_DATA
    object_surface_p obj_surface;
    object_image_p obj_image;
    object_buffer_p obj_buffer;
    object_config_p obj_config;
    object_heap_iterator iter;

    /* Clean up left over surfaces */
    obj_surface = (object_surface_p) object_heap_first( &driver_data->surface_heap, &iter);
    while (obj_surface) {
        printf("vaTerminate: surfaceID %08x still allocated, destroying\n", obj_surface->base.id);
        while (VA_STATUS_SUCCESS ==
               rockchip_DestroySurfaces(ctx, &obj_surface->base.id, 1));
        obj_surface = (object_surface_p) object_heap_next( &driver_data->surface_heap, &iter);
    }
    object_heap_destroy( &driver_data->surface_heap );

    /* Clean up left over images */
    obj_image = (object_image_p) object_heap_first( &driver_data->image_heap, &iter);
    while (obj_image) {
        printf("vaTerminate: imageID %08x still allocated, destroying\n", obj_image->base.id);
        while (VA_STATUS_SUCCESS ==
               rockchip_DestroyImage(ctx, obj_image->base.id));
        obj_image = (object_image_p) object_heap_next( &driver_data->image_heap, &iter);
    }
    object_heap_destroy( &driver_data->image_heap );

    /* Clean up left over buffers */
    obj_buffer = (object_buffer_p) object_heap_first( &driver_data->buffer_heap, &iter);
    while (obj_buffer) {
        printf("vaTerminate: bufferID %08x still allocated, destroying\n", obj_buffer->base.id);
        while (VA_STATUS_SUCCESS ==
               rockchip_DestroyBuffer(ctx, obj_buffer->base.id));
        obj_buffer = (object_buffer_p) object_heap_next( &driver_data->buffer_heap, &iter);
    }
    object_heap_destroy( &driver_data->buffer_heap );

    /* TODO cleanup */
    object_heap_destroy( &driver_data->context_heap );

    /* Clean up configIDs */
    obj_config = (object_config_p) object_heap_first( &driver_data->config_heap, &iter);
    while (obj_config) {
        object_heap_free( &driver_data->config_heap, (object_base_p) obj_config);
        obj_config = (object_config_p) object_heap_next( &driver_data->config_heap, &iter);
    }
    object_heap_destroy( &driver_data->config_heap );

    free(ctx->pDriverData);
    ctx->pDriverData = NULL;

    return VA_STATUS_SUCCESS;
}

EXPORT VAStatus __vaDriverInit_0_32(VADriverContextP ctx)
{
    struct VADriverVTable * const vtable = ctx->vtable;
    int result;
    struct rockchip_driver_data *driver_data;

    ctx->version_major = VA_MAJOR_VERSION;
    ctx->version_minor = VA_MINOR_VERSION;
    ctx->max_profiles = ROCKCHIP_MAX_PROFILES;
    ctx->max_entrypoints = ROCKCHIP_MAX_ENTRYPOINTS;
    ctx->max_attributes = ROCKCHIP_MAX_CONFIG_ATTRIBUTES;
    ctx->max_image_formats = ROCKCHIP_MAX_IMAGE_FORMATS;
    ctx->max_subpic_formats = ROCKCHIP_MAX_SUBPIC_FORMATS;
    ctx->max_display_attributes = ROCKCHIP_MAX_DISPLAY_ATTRIBUTES;
    ctx->str_vendor = ROCKCHIP_STR_VENDOR;

    vtable->vaTerminate = rockchip_Terminate;
    vtable->vaQueryConfigProfiles = rockchip_QueryConfigProfiles;
    vtable->vaQueryConfigEntrypoints = rockchip_QueryConfigEntrypoints;
    vtable->vaQueryConfigAttributes = rockchip_QueryConfigAttributes;
    vtable->vaCreateConfig = rockchip_CreateConfig;
    vtable->vaDestroyConfig = rockchip_DestroyConfig;
    vtable->vaGetConfigAttributes = rockchip_GetConfigAttributes;
    vtable->vaCreateSurfaces = rockchip_CreateSurfaces;
    vtable->vaDestroySurfaces = rockchip_DestroySurfaces;
    vtable->vaCreateContext = rockchip_CreateContext;
    vtable->vaDestroyContext = rockchip_DestroyContext;
    vtable->vaCreateBuffer = rockchip_CreateBuffer;
    vtable->vaBufferSetNumElements = rockchip_BufferSetNumElements;
    vtable->vaMapBuffer = rockchip_MapBuffer;
    vtable->vaUnmapBuffer = rockchip_UnmapBuffer;
    vtable->vaDestroyBuffer = rockchip_DestroyBuffer;
    vtable->vaBeginPicture = rockchip_BeginPicture;
    vtable->vaRenderPicture = rockchip_RenderPicture;
    vtable->vaEndPicture = rockchip_EndPicture;
    vtable->vaSyncSurface = rockchip_SyncSurface;
    vtable->vaQuerySurfaceStatus = rockchip_QuerySurfaceStatus;
    vtable->vaPutSurface = rockchip_PutSurface;
    vtable->vaQueryImageFormats = rockchip_QueryImageFormats;
    vtable->vaCreateImage = rockchip_CreateImage;
    vtable->vaDeriveImage = rockchip_DeriveImage;
    vtable->vaDestroyImage = rockchip_DestroyImage;
    vtable->vaSetImagePalette = rockchip_SetImagePalette;
    vtable->vaGetImage = rockchip_GetImage;
    vtable->vaPutImage = rockchip_PutImage;
    vtable->vaQuerySubpictureFormats = rockchip_QuerySubpictureFormats;
    vtable->vaCreateSubpicture = rockchip_CreateSubpicture;
    vtable->vaDestroySubpicture = rockchip_DestroySubpicture;
    vtable->vaSetSubpictureImage = rockchip_SetSubpictureImage;
    vtable->vaSetSubpictureChromakey = rockchip_SetSubpictureChromakey;
    vtable->vaSetSubpictureGlobalAlpha = rockchip_SetSubpictureGlobalAlpha;
    vtable->vaAssociateSubpicture = rockchip_AssociateSubpicture;
    vtable->vaDeassociateSubpicture = rockchip_DeassociateSubpicture;
    vtable->vaQueryDisplayAttributes = rockchip_QueryDisplayAttributes;
    vtable->vaGetDisplayAttributes = rockchip_GetDisplayAttributes;
    vtable->vaSetDisplayAttributes = rockchip_SetDisplayAttributes;
    vtable->vaLockSurface = rockchip_LockSurface;
    vtable->vaUnlockSurface = rockchip_UnlockSurface;
    vtable->vaBufferInfo = rockchip_BufferInfo;

    driver_data = (struct rockchip_driver_data *) malloc( sizeof(*driver_data) );
    ctx->pDriverData = (void *) driver_data;

    result = object_heap_init( &driver_data->config_heap, sizeof(struct object_config), CONFIG_ID_OFFSET );
    ASSERT( result == 0 );

    result = object_heap_init( &driver_data->context_heap, sizeof(struct object_context), CONTEXT_ID_OFFSET );
    ASSERT( result == 0 );

    result = object_heap_init( &driver_data->surface_heap, sizeof(struct object_surface), SURFACE_ID_OFFSET );
    ASSERT( result == 0 );

    result = object_heap_init( &driver_data->image_heap, sizeof(struct object_image), IMAGE_ID_OFFSET );
    ASSERT( result == 0 );

    result = object_heap_init( &driver_data->buffer_heap, sizeof(struct object_buffer), BUFFER_ID_OFFSET );
    ASSERT( result == 0 );

    return VA_STATUS_SUCCESS;
}

EXPORT VAStatus VA_DRIVER_INIT_FUNC(  VADriverContextP ctx )
{
    return __vaDriverInit_0_32(ctx);
}
