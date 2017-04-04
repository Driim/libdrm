/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: hyejung, kwon <cjscld15@nexell.co.kr>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

#include <xf86drm.h>

#include "nexell_drm.h"
#include "nexell_drmif.h"

/**
 * return gem_fd
 */
int nx_alloc_gem(int drm_fd, int size, int flags)
{
	struct nx_drm_gem_create arg = { 0, };
	int ret;

	arg.size = (uint64_t)size;
	arg.flags = flags;

	ret = drmCommandWriteRead(drm_fd, DRM_NX_GEM_CREATE, &arg,
				     sizeof(arg));
	if (ret) {
		perror("drmCommandWriteRead\n");
		return ret;
	}

	return arg.handle;
}

void nx_free_gem(int drm_fd, int gem)
{
	struct drm_gem_close arg = {0, };

	arg.handle = gem;
	drmIoctl(drm_fd, DRM_IOCTL_GEM_CLOSE, &arg);
}

/**
 * return dmabuf fd
 */
int nx_gem_to_dmafd(int drm_fd, int gem_fd)
{
	int ret, fd;

	ret = drmPrimeHandleToFD(drm_fd, gem_fd, 0, &fd);
	if (ret)
		return ret;

	return fd;
}
