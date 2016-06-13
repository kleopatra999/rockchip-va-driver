# Copyright (c) 2016 Rockchip Electronics Co., Ltd.
#
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sub license, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice (including the
# next paragraph) shall be included in all copies or substantial portions
# of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
# IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

rockchip_drv_video_la_LTLIBRARIES = rockchip_drv_video.la
rockchip_drv_video_ladir = $(LIBVA_DRIVERS_PATH)
rockchip_drv_video_la_LDFLAGS = -pthread -module -avoid-version -Wl,--no-undefined
rockchip_drv_video_la_LIBADD = -lm -ldl -lrkenc-h264e
AM_CFLAGS = -fvisibility=hidden

rockchip_drv_video_la_SOURCES = \
		rockchip_drv_video.c object_heap.c \
		rockchip_buffer.c rockchip_image.c \
		rockchip_surface.c rockchip_picture.c \
		rockchip_encoder.c v4l2_utils.c

CFLAGS += -Wall -ffloat-store -fvisibility=hidden -Iinclude

symbol_info:	rockchip_drv_video.la
	objdump -T .libs/rockchip_drv_video.so | grep UND | sort -k 5 > Linker_dependencies.txt
	objdump -T .libs/rockchip_drv_video.so | grep -v UND | sort -k 5 > Linker_exports.txt

#install-exec-am:
#	-rm -f $(rockchip_drv_video_ladir)/rockchip_drv_video.so
#	-$(LN_S) $(rockchip_drv_video_ladir)/rockchip_drv_video.so $(rockchip_drv_video_ladir)/rockchip_drv_video.so
