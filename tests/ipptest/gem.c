/*
 * DRM based fimc test program
 * Copyright 2012 Samsung Electronics
 *   Eunchul Kim <chulspro.kim@sasmsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "xf86drm.h"
#include "xf86drmMode.h"
#include "libkms.h"

#include "exynos_drm.h"
#include "internal.h"
#include "gem.h"

int exynos_gem_create(int fd, struct drm_exynos_gem_create *gem)
{
	int ret = 0;

	if (!gem) {
		fprintf(stderr, "gem object is null.\n");
		return -EFAULT;
	}
	ret = ioctl(fd, DRM_IOCTL_EXYNOS_GEM_CREATE, gem);
	if (ret < 0)
		fprintf(stderr, "failed to create gem buffer: %d\n", ret);

	return ret;
}

int exynos_gem_mmap(int fd, struct exynos_gem_mmap_data *in_mmap)
{
	int ret;
	void *map;
	struct drm_mode_map_dumb arg;

	memset(&arg, 0, sizeof(arg));
	arg.handle = in_mmap->handle;

	ret = ioctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &arg);
	if (ret) {
		fprintf(stderr, "failed to map dumb buffer: %d\n", errno);
		return ret;
	}

	in_mmap->offset = arg.offset;

	map = mmap(NULL, (size_t)in_mmap->size, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, (off_t)arg.offset);
	if (map == MAP_FAILED) {
		fprintf(stderr, "failed to mmap buffer: %d\n", errno);
		return -EFAULT;
	}

	in_mmap->addr = map;

	return 0;
}

int exynos_gem_close(int fd, struct drm_gem_close *gem_close)
{
	int ret = 0;

	ret = ioctl(fd, DRM_IOCTL_GEM_CLOSE, gem_close);
	if (ret < 0)
		fprintf(stderr, "failed to close: %d\n", ret);
	return ret;
}

struct kms_bo* exynos_kms_gem_create(struct kms_driver *kms,
		unsigned int width, unsigned int height, unsigned int *stride)
{
	struct kms_bo *bo;
	unsigned bo_attribs[] = {
		KMS_WIDTH,   0,
		KMS_HEIGHT,  0,
		KMS_BO_TYPE, KMS_BO_TYPE_SCANOUT_X8R8G8B8,
		KMS_TERMINATE_PROP_LIST
	};
	int ret;

	bo_attribs[1] = width;
	bo_attribs[3] = height;

	ret = kms_bo_create(kms, bo_attribs, &bo);
	if (ret) {
		fprintf(stderr, "failed to alloc buffer: %d\n", ret);
		return NULL;
	}

	ret = kms_bo_get_prop(bo, KMS_PITCH, stride);
	if (ret) {
		fprintf(stderr, "failed to retreive buffer stride: %d\n", ret);
		kms_bo_destroy(&bo);
		return NULL;
	}

	return bo;
}
