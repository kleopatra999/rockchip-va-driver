/* Copyright 2014 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * Rockchip VPU (video processing unit) encoder library module.
 *
 * This library is not thread-safe.
 */

#ifndef LIBVPU_RK_VEPU_PLUGIN_H_
#define LIBVPU_RK_VEPU_PLUGIN_H_

#include <linux/videodev2.h>

#define V4L2_CID_CUSTOM_BASE               (V4L2_CID_USER_BASE | 0x1000)
#define V4L2_CID_PRIVATE_ROCKCHIP_HEADER     (V4L2_CID_CUSTOM_BASE)
#define V4L2_CID_PRIVATE_ROCKCHIP_REG_PARAMS (V4L2_CID_CUSTOM_BASE + 1)
#define V4L2_CID_PRIVATE_ROCKCHIP_HW_PARAMS  (V4L2_CID_CUSTOM_BASE + 2)
#define V4L2_CID_PRIVATE_ROCKCHIP_RET_PARAMS (V4L2_CID_CUSTOM_BASE + 3)
#define V4L2_CID_PRIVATE_ROCKCHIP_VAENC_SPS  (V4L2_CID_CUSTOM_BASE + 4)
#define V4L2_CID_PRIVATE_ROCKCHIP_VAENC_PPS  (V4L2_CID_CUSTOM_BASE + 5)
#define V4L2_CID_PRIVATE_ROCKCHIP_VAENC_SLICE (V4L2_CID_CUSTOM_BASE + 6)
#define V4L2_CID_PRIVATE_ROCKCHIP_VAENC_RC   (V4L2_CID_CUSTOM_BASE + 7)

void *plugin_init(int fd);
void plugin_close(void *dev_ops_priv);
int plugin_ioctl(void *dev_ops_priv, int fd, unsigned long int cmd, void *arg);

#endif  // LIBVPU_RK_VEPU_PLUGIN_H_
