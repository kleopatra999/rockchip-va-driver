#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <linux/media.h>

#include "v4l2_utils.h"

#define PRINT(fmt, args...) \
    printf("%s[%d] " fmt "\n", __func__, __LINE__, ## args)

#define IOCTL(type, arg) plugin_ioctl(ctx->enc, ctx->fd, type, arg)

#define IOCTL_OR_ERROR_RETURN_VALUE(type, arg, value, type_str) \
    do {                                                        \
        if (IOCTL(type, arg) != 0) {                            \
            PRINT("ioctl() failed: " type_str);                 \
            return value;                                       \
        }                                                       \
    } while (0)

#define IOCTL_OR_ERROR_RETURN(type, arg) \
    IOCTL_OR_ERROR_RETURN_VALUE(type, arg, -1, #type)

#define IOCTL_OR_LOG_ERROR(type, arg)        \
    do {                                     \
        if (IOCTL(type, arg) != 0)           \
            PRINT("ioctl() failed: " #type); \
    } while (0)

enc_context_p v4l2_init(const char *device_path) {
    int fd = open(device_path, O_RDWR | O_NONBLOCK | O_CLOEXEC);

    if (fd <= 0) {
        PRINT("failed to open %s", device_path);
        return NULL;
    }
    PRINT(" got %s", device_path);

    enc_context_t *ctx = (enc_context_t *) calloc(1, sizeof(enc_context_t));
    if (ctx == NULL)
        goto failed_ctx;
    ctx->fd = fd;

    ctx->enc = plugin_init(ctx->fd);
    if (!ctx->enc)
        goto failed_plugin;

    return ctx;

failed_ctx:
    close(ctx->fd);
failed_plugin:
    free(ctx);

    return NULL;
}

enc_context_p v4l2_init_by_name(const char *name) {
    DIR *dir;
    struct dirent *ent;

    enc_context_p ctx = NULL;

#define SYS_PATH		"/sys/class/video4linux/"
#define DEV_PATH		"/dev/"

    if ((dir = opendir(SYS_PATH)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            FILE *fp;
            char path[64];
            char dev_name[64];

            snprintf(path, 64, SYS_PATH "%s/name",
                    ent->d_name);
            fp = fopen(path, "r");
            if (!fp)
                continue;
            if (!fgets(dev_name, 32, fp)) {
                dev_name[0] = '\0';
            }
            fclose(fp);

            if (!strstr(dev_name, name))
                continue;

            snprintf(path, sizeof(path), DEV_PATH "%s",
                    ent->d_name);

            ctx = v4l2_init(path);
            if (ctx)
                break;
        }
        closedir (dir);
    }
    return ctx;
}

int v4l2_deinit(enc_context_p ctx) {
    struct v4l2_requestbuffers reqbufs;
    memset(&reqbufs, 0, sizeof(reqbufs));
    reqbufs.count = 0;
    reqbufs.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    reqbufs.memory = V4L2_MEMORY_MMAP;
    IOCTL_OR_LOG_ERROR(VIDIOC_REQBUFS, &reqbufs);

    memset(&reqbufs, 0, sizeof(reqbufs));
    reqbufs.count = 0;
    reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    reqbufs.memory = V4L2_MEMORY_MMAP;
    IOCTL_OR_ERROR_RETURN(VIDIOC_REQBUFS, &reqbufs);

    plugin_close(ctx->enc);
    close(ctx->fd);
    free(ctx);

    return 0;
}

int v4l2_reqbufs(enc_context_p ctx) {
    struct v4l2_requestbuffers reqbufs;
    memset(&reqbufs, 0, sizeof(reqbufs));
    reqbufs.count = 1;
    reqbufs.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    reqbufs.memory = V4L2_MEMORY_MMAP;
    IOCTL_OR_ERROR_RETURN(VIDIOC_REQBUFS, &reqbufs);

    memset(&reqbufs, 0, sizeof(reqbufs));
    reqbufs.count = 1;
    reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    reqbufs.memory = V4L2_MEMORY_MMAP;
    IOCTL_OR_ERROR_RETURN(VIDIOC_REQBUFS, &reqbufs);

    return 0;
}

int v4l2_querybuf(enc_context_p ctx) {
    struct v4l2_plane planes[VIDEO_MAX_PLANES];
    struct v4l2_buffer buffer;
    memset(&buffer, 0, sizeof(buffer));
    memset(planes, 0, sizeof(planes));
    buffer.index = 0;
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.m.planes = planes;
    buffer.length = 1;
    IOCTL_OR_ERROR_RETURN(VIDIOC_QUERYBUF, &buffer);

    ctx->coded_size = buffer.m.planes[0].length;
    ctx->coded_buffer = mmap(NULL, ctx->coded_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED, ctx->fd,
            buffer.m.planes[0].m.mem_offset);
    if (ctx->coded_buffer == MAP_FAILED) {
        PRINT("create coded buffer: mmap() failed");
        return -1;
    }

    memset(&buffer, 0, sizeof(buffer));
    memset(planes, 0, sizeof(planes));
    buffer.index = 0;
    buffer.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.m.planes = planes;
    buffer.length = 3;
    IOCTL_OR_ERROR_RETURN(VIDIOC_QUERYBUF, &buffer);

    int i;
    for (i = 0; i < 3; i++) {
        ctx->input_size[i] = buffer.m.planes[i].length;
        ctx->input_buffer[i] = mmap(NULL, ctx->input_size[i],
                PROT_READ | PROT_WRITE,
                MAP_SHARED, ctx->fd,
                buffer.m.planes[i].m.mem_offset);
        if (ctx->input_buffer[i] == MAP_FAILED) {
            PRINT("create input buffer[%d]: mmap() failed", i);
            return -1;
        }
    }
    return 0;
}

int v4l2_s_fmt(enc_context_p ctx) {
    struct v4l2_format format;
    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    format.fmt.pix_mp.width = ctx->width;
    format.fmt.pix_mp.height = ctx->height;
    format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
    format.fmt.pix_mp.plane_fmt[0].sizeimage = (2 * 1024 * 1024);
    format.fmt.pix_mp.num_planes = 1;
    IOCTL_OR_ERROR_RETURN(VIDIOC_S_FMT, &format);

    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YUV420M;
    format.fmt.pix_mp.width = ctx->width;
    format.fmt.pix_mp.height = ctx->height;
    format.fmt.pix_mp.num_planes = 3;
    IOCTL_OR_ERROR_RETURN(VIDIOC_S_FMT, &format);

    return 0;
}

int v4l2_streamon(enc_context_p ctx) {
    __u32 type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    IOCTL_OR_ERROR_RETURN(VIDIOC_STREAMON, &type);

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    IOCTL_OR_ERROR_RETURN(VIDIOC_STREAMON, &type);

    return 0;
}

int v4l2_streamoff(enc_context_p ctx) {
    __u32 type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    IOCTL_OR_ERROR_RETURN(VIDIOC_STREAMOFF, &type);

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    IOCTL_OR_ERROR_RETURN(VIDIOC_STREAMOFF, &type);

    return 0;
}

int v4l2_s_ext_ctrls(enc_context_p ctx, struct v4l2_ext_controls* ext_ctrls) {
    IOCTL_OR_ERROR_RETURN(VIDIOC_S_EXT_CTRLS, ext_ctrls);

    return 0;
}

int v4l2_s_parm(enc_context_p ctx, struct v4l2_streamparm *parm) {
    IOCTL_OR_ERROR_RETURN(VIDIOC_S_PARM, parm);

    return 0;
}

int v4l2_qbuf_input(enc_context_p ctx, void *data, int size) {
    struct v4l2_buffer qbuf;
    struct v4l2_plane qbuf_planes[VIDEO_MAX_PLANES];
    memset(&qbuf, 0, sizeof(qbuf));
    memset(qbuf_planes, 0, sizeof(qbuf_planes));
    qbuf.index = 0;
    qbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    qbuf.m.planes = qbuf_planes;

    qbuf.m.planes[0].bytesused = ctx->width * ctx->height;
    qbuf.m.planes[1].bytesused = ctx->width * ctx->height / 4;
    qbuf.m.planes[2].bytesused = ctx->width * ctx->height / 4;

    memcpy(ctx->input_buffer[0], data, qbuf.m.planes[0].bytesused);
    data += qbuf.m.planes[0].bytesused;
    memcpy(ctx->input_buffer[1], data, qbuf.m.planes[1].bytesused);
    data += qbuf.m.planes[1].bytesused;
    memcpy(ctx->input_buffer[2], data, qbuf.m.planes[2].bytesused);

    qbuf.memory = V4L2_MEMORY_MMAP;
    qbuf.length = 3;

    IOCTL_OR_ERROR_RETURN(VIDIOC_QBUF, &qbuf);

    return 0;
}

int v4l2_qbuf_output(enc_context_p ctx) {
    struct v4l2_buffer qbuf;
    struct v4l2_plane qbuf_planes[VIDEO_MAX_PLANES];
    memset(&qbuf, 0, sizeof(qbuf));
    memset(qbuf_planes, 0, sizeof(qbuf_planes));
    qbuf.index = 0;
    qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    qbuf.memory = V4L2_MEMORY_MMAP;
    qbuf.m.planes = qbuf_planes;
    qbuf.length = 1;
    IOCTL_OR_ERROR_RETURN(VIDIOC_QBUF, &qbuf);

    return 0;
}

int v4l2_dqbuf_input(enc_context_p ctx) {
    struct v4l2_buffer dqbuf;
    struct v4l2_plane planes[VIDEO_MAX_PLANES];
    memset(&dqbuf, 0, sizeof(dqbuf));
    memset(&planes, 0, sizeof(planes));
    dqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    dqbuf.memory = V4L2_MEMORY_MMAP;
    dqbuf.m.planes = planes;
    dqbuf.length = 3;
    while (IOCTL(VIDIOC_DQBUF, &dqbuf) != 0) {
        if (errno == EAGAIN) {
            usleep(1000);
            continue;
        }
        PRINT("ioctl() failed: VIDIOC_DQBUF");
        return -1;
    }

    return 0;
}

int v4l2_dqbuf_output(enc_context_p ctx) {
    struct v4l2_buffer dqbuf;
    struct v4l2_plane planes[VIDEO_MAX_PLANES];

    memset(&dqbuf, 0, sizeof(dqbuf));
    memset(&planes, 0, sizeof(planes));
    dqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    dqbuf.memory = V4L2_MEMORY_MMAP;
    dqbuf.m.planes = planes;
    dqbuf.length = 1;

    while (IOCTL(VIDIOC_DQBUF, &dqbuf) != 0) {
        if (errno == EAGAIN) {
            usleep(1000);
            continue;
        }
        PRINT("ioctl() failed: VIDIOC_DQBUF");
        return -1;
    }

    ctx->coded_size = dqbuf.m.planes[0].bytesused;

    return 0;
}
