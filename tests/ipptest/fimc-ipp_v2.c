/*
 * DRM based IPPv2 test program
 * Copyright 2018 Samsung Electronics
 *   Marek Szyprowski <m.szyprowski@sasmsung.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "exynos_drm.h"
#include "fimctest.h"
#include "gem.h"
#include "util.h"
#include "fimc.h"


#include "drm_fourcc.h"
#include "drm_mode.h"
#include "libkms.h"
#include "internal.h"

struct exynos_drm_ipp_std_task {
	struct drm_exynos_ipp_task_buffer buf[2];
	struct drm_exynos_ipp_task_rect rect[2];
	struct drm_exynos_ipp_task_transform transform;
} __packed;

void fimc_v2_m2m_set_mode(struct device *dev, struct connector *c, int count,
	enum drm_exynos_degree degree, enum drm_exynos_ipp_cmd_m2m display,
	long int *usec)
{
	struct drm_exynos_sz def_sz;
	struct drm_exynos_sz src_sz;
	void *usr_addr1[MAX_BUF], *usr_addr2[MAX_BUF];
	uint32_t handles[4], pitches[4], offsets[4] = {0};
	struct kms_bo *bo_src[MAX_BUF] = { NULL, }, *bo_dst = NULL;
	struct pipe_arg pipe;
	unsigned int fb_id_dst;
	int ret, i, j;

	struct timeval begin, end;
	char filename[80];

	struct exynos_drm_ipp_std_task task = { 0 };
	struct drm_exynos_ioctl_ipp_commit arg = { 0 };

	src_sz = c[0].src_sz;
	printf("source image resolution [%ux%u]\n", src_sz.hsize, src_sz.vsize);

	dev->mode.width = 0;
	dev->mode.height = 0;

	/* For crtc and mode */
	for (i = 0; i < count; i++) {
		ret = pipe_find_crtc_and_mode(dev, &pipe, &c[i]);
		if (ret < 0)
			continue;

		dev->mode.width += pipe.mode->hdisplay;
		if (dev->mode.height < pipe.mode->vdisplay)
			dev->mode.height = pipe.mode->vdisplay;
	}

	for (i = 0; i < MAX_BUF; i++) {
		/* For source buffer */
		bo_src[i] = util_kms_gem_create_mmap(dev->kms, DRM_FORMAT_XRGB8888,
			src_sz.hsize, src_sz.vsize,
			handles, pitches, offsets);
		if (!bo_src[i])
			goto err_ipp_src_buff_close;

		usr_addr1[i] = bo_src[i]->ptr;
	}

	/* Create test Image
	 * Display is YUV422 format, file is RGB888 format
	 */
	if (display == IPP_CMD_M2M_DISPLAY)
		util_draw_buffer_yuv(usr_addr1, src_sz.hsize, src_sz.vsize);
	else
		fill_smpte_rgb32(usr_addr1[0], src_sz.hsize, src_sz.vsize,
				src_sz.hsize * 4);

	/* For destination buffer */
	bo_dst = util_kms_gem_create_mmap(dev->kms, DRM_FORMAT_XRGB8888,
			dev->mode.width, dev->mode.height,
			handles, pitches, offsets);
	if (!bo_dst)
		goto err_ipp_src_buff_close;

	usr_addr2[0] = bo_dst->ptr;

	def_sz.hsize = dev->mode.width;
	def_sz.vsize = dev->mode.height;

	/* src buf */
	task.buf[0].id = DRM_EXYNOS_IPP_TASK_BUFFER | DRM_EXYNOS_IPP_TASK_TYPE_SOURCE;
	task.buf[0].width = src_sz.hsize;
	task.buf[0].height = src_sz.vsize;
	task.buf[0].fourcc = (display == IPP_CMD_M2M_FILE) ? DRM_FORMAT_XRGB8888 : DRM_FORMAT_YUV422;
	for (i = 0; i < MAX_BUF; i++)
		task.buf[0].gem_id[i] = bo_src[i]->handle;

	/* dst buf */
	task.buf[1].id = DRM_EXYNOS_IPP_TASK_BUFFER | DRM_EXYNOS_IPP_TASK_TYPE_DESTINATION;
	task.buf[1].fourcc = DRM_FORMAT_XRGB8888;
	task.buf[1].width = def_sz.hsize;
	task.buf[1].height = def_sz.vsize;
	task.buf[1].gem_id[0] = bo_dst->handle;
	task.buf[1].pitch[0] = bo_dst->pitch;
	task.buf[1].offset[0] = bo_dst->offset;

	task.rect[0].id = DRM_EXYNOS_IPP_TASK_RECTANGLE | DRM_EXYNOS_IPP_TASK_TYPE_SOURCE;
	task.rect[0].x = 0;
	task.rect[0].y = 0;
	task.rect[0].w = src_sz.hsize;
	task.rect[0].h = src_sz.vsize;

	task.rect[1].id = DRM_EXYNOS_IPP_TASK_RECTANGLE | DRM_EXYNOS_IPP_TASK_TYPE_DESTINATION;
	task.rect[1].x = 0;
	task.rect[1].y = 0;
	task.rect[1].w = def_sz.hsize;
	task.rect[1].h = def_sz.vsize;

	task.transform.id = DRM_EXYNOS_IPP_TASK_TRANSFORM;
	if (degree == EXYNOS_DRM_DEGREE_0)
		task.transform.rotation = DRM_MODE_ROTATE_0;
	else if (degree == EXYNOS_DRM_DEGREE_90)
		task.transform.rotation = DRM_MODE_ROTATE_90;
	else if (degree == EXYNOS_DRM_DEGREE_180)
		task.transform.rotation = DRM_MODE_ROTATE_180;
	else if (degree == EXYNOS_DRM_DEGREE_270)
		task.transform.rotation = DRM_MODE_ROTATE_270;
	else {
		fprintf(stderr, "invalid rotation value\n");
		goto err_ipp_close;
	}

	/* prepare ioctl data */
	arg.flags = 0; /* BLOCKING call, no event */
	arg.ipp_id = 0; /* use first IPP module available */
	arg.params_size = sizeof(task);
	arg.params_ptr = (unsigned long)(&task);
	arg.user_data = 0; /* unused */

	/* Display OR File */
	switch (display) {
	case IPP_CMD_M2M_FILE:
		/* For src image write file */
		sprintf(filename, RESULT_PATH "fimc_m2m_org_src.bmp");
		util_write_bmp(filename, usr_addr1[0],
				src_sz.hsize, src_sz.vsize);

		for (j = 0; j < MAX_LOOP; j++) {
			gettimeofday(&begin, NULL);
			ret = ioctl(dev->fd, DRM_IOCTL_EXYNOS_IPP_COMMIT, &arg);
			if (ret) {
				fprintf(stderr, "failed to process image\n");
				goto err_ipp_close;
			}

			gettimeofday(&end, NULL);

			usec[j] = (end.tv_sec - begin.tv_sec) * 1000000 +
					(end.tv_usec - begin.tv_usec);

			sprintf(filename, RESULT_PATH "fimc_m2m_dst%d.bmp", j);
			util_write_bmp(filename, usr_addr2[0], def_sz.hsize, def_sz.vsize);
		}
		break;
	case IPP_CMD_M2M_DISPLAY:
		/* Add fb2 dst */
		ret = drmModeAddFB2(dev->fd, dev->mode.width, dev->mode.height,
				pipe.fourcc, handles, pitches,
				offsets, &fb_id_dst, 0);
		if (ret) {
			fprintf(stderr, "failed to add fb (%ux%u):%d\n",
					dev->mode.width, dev->mode.height,
					errno);
			goto err_ipp_close;
		}

		for (j = 0; j < 2; j++) {
			gettimeofday(&begin, NULL);
			ret = ioctl(dev->fd, DRM_IOCTL_EXYNOS_IPP_COMMIT, &arg);
			if (ret) {
				fprintf(stderr, "failed to process image\n");
				goto err_ipp_close;
			}
			gettimeofday(&end, NULL);
			usec[j] = (end.tv_sec - begin.tv_sec) * 1000000 +
				(end.tv_usec - begin.tv_usec);

			if (j == 0) {
				/* Set crtc */
				ret = drmModeSetCrtc(dev->fd, pipe.crtc->crtc->crtc_id,
					fb_id_dst, 0, 0, pipe.con_ids,
					pipe.num_cons, pipe.mode);
				if (ret) {
					fprintf(stderr, "failed to set crtc: %d\n", errno);
					goto err_ipp_close;
				}
				/* reset rotation for the next frame */
				task.transform.rotation = DRM_MODE_ROTATE_0;
			} else {
				/* Set Flip */
				ret = drmModePageFlip(dev->fd, pipe.crtc->crtc->crtc_id,
						fb_id_dst, DRM_MODE_PAGE_FLIP_EVENT, &pipe);
				if (ret) {
					fprintf(stderr, "failed to page flip: %d\n",
						errno);
					goto err_ipp_close;
				}
			}

			getchar();
		}
		break;
	case IPP_CMD_M2M_NONE:
		break;
	}

err_ipp_close:
err_ipp_dst_buff_close:
	util_kms_gem_destroy_mmap(&bo_dst);
err_ipp_src_buff_close:
	/* Close source buffer */
	for (i = 0; i < MAX_BUF; i++) {
		ret = util_kms_gem_destroy_mmap(&bo_src[i]);
		if (ret < 0)
			break;
	}
}

void fimc_v2_wb_set_mode(struct connector *c, int count, int page_flip,
								long int *usec)
{
	fprintf(stderr, "not supported.\n");
}

void fimc_v2_output_set_mode(struct connector *c, int count, int page_flip,
								long int *usec)
{
	fprintf(stderr, "not supported.\n");
}
