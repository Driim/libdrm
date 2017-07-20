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

/*
 * This fairly simple test program dumps output in a similar format to the
 * "xrandr" tool everyone knows & loves.  It's necessarily slightly different
 * since the kernel separates outputs into encoder and connector structures,
 * each with their own unique ID.  The program also allows test testing of the
 * memory management and mode setting APIs by allowing the user to specify a
 * connector and mode to use for mode setting.  If all works as expected, a
 * blue background should be painted on the monitor attached to the specified
 * connector after the selected mode is set.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "libkms.h"

#include "exynos_drm.h"

#include "fimctest.h"
#include "fimc.h"

drmModeRes *resources;
int fd, modes;

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct type_name {
	int type;
	char *name;
};

#define type_name_fn(res) \
char * res##_str(int type) {			\
	int i;						\
	for (i = 0; i < ARRAY_SIZE(res##_names); i++) { \
		if (res##_names[i].type == type)	\
			return res##_names[i].name;	\
	}						\
	return "(invalid)";				\
}

struct type_name encoder_type_names[] = {
	{ DRM_MODE_ENCODER_NONE, "none" },
	{ DRM_MODE_ENCODER_DAC, "DAC" },
	{ DRM_MODE_ENCODER_TMDS, "TMDS" },
	{ DRM_MODE_ENCODER_LVDS, "LVDS" },
	{ DRM_MODE_ENCODER_TVDAC, "TVDAC" },
};

type_name_fn(encoder_type)

struct type_name connector_status_names[] = {
	{ DRM_MODE_CONNECTED, "connected" },
	{ DRM_MODE_DISCONNECTED, "disconnected" },
	{ DRM_MODE_UNKNOWNCONNECTION, "unknown" },
};

type_name_fn(connector_status)

struct type_name connector_type_names[] = {
	{ DRM_MODE_CONNECTOR_Unknown, "unknown" },
	{ DRM_MODE_CONNECTOR_VGA, "VGA" },
	{ DRM_MODE_CONNECTOR_DVII, "DVI-I" },
	{ DRM_MODE_CONNECTOR_DVID, "DVI-D" },
	{ DRM_MODE_CONNECTOR_DVIA, "DVI-A" },
	{ DRM_MODE_CONNECTOR_Composite, "composite" },
	{ DRM_MODE_CONNECTOR_SVIDEO, "s-video" },
	{ DRM_MODE_CONNECTOR_LVDS, "LVDS" },
	{ DRM_MODE_CONNECTOR_Component, "component" },
	{ DRM_MODE_CONNECTOR_9PinDIN, "9-pin DIN" },
	{ DRM_MODE_CONNECTOR_DisplayPort, "displayport" },
	{ DRM_MODE_CONNECTOR_HDMIA, "HDMI-A" },
	{ DRM_MODE_CONNECTOR_HDMIB, "HDMI-B" },
	{ DRM_MODE_CONNECTOR_TV, "TV" },
	{ DRM_MODE_CONNECTOR_eDP, "embedded displayport" },
};

type_name_fn(connector_type)

static void dump_encoders(void)
{
	drmModeEncoder *encoder;
	int i;

	printf("Encoders:\n");
	printf("id\tcrtc\ttype\tpossible crtcs\tpossible clones\t\n");
	for (i = 0; i < resources->count_encoders; i++) {
		encoder = drmModeGetEncoder(fd, resources->encoders[i]);

		if (!encoder) {
			fprintf(stderr, "could not get encoder %i: %d\n",
				resources->encoders[i], errno);
			continue;
		}
		printf("%d\t%d\t%s\t0x%08x\t0x%08x\n",
		       encoder->encoder_id,
		       encoder->crtc_id,
		       encoder_type_str(encoder->encoder_type),
		       encoder->possible_crtcs,
		       encoder->possible_clones);
		drmModeFreeEncoder(encoder);
	}
	printf("\n");
}

static void dump_mode(drmModeModeInfo *mode)
{
	printf("  %s %d %d %d %d %d %d %d %d %d\n",
	       mode->name,
	       mode->vrefresh,
	       mode->hdisplay,
	       mode->hsync_start,
	       mode->hsync_end,
	       mode->htotal,
	       mode->vdisplay,
	       mode->vsync_start,
	       mode->vsync_end,
	       mode->vtotal);
}

static void dump_props(drmModeConnector *connector)
{
	drmModePropertyPtr props;
	int i;

	for (i = 0; i < connector->count_props; i++) {
		props = drmModeGetProperty(fd, connector->props[i]);
		if (!props)
			continue;

		printf("\t%s, flags %d\n", props->name, props->flags);
		drmModeFreeProperty(props);
	}
}

static void dump_connectors(void)
{
	drmModeConnector *connector;
	int i, j;

	printf("Connectors:\n");
	printf("id\tencoder\tstatus\t\ttype\tsize (mm)\tmodes\tencoders\n");
	for (i = 0; i < resources->count_connectors; i++) {
		connector = drmModeGetConnector(fd, resources->connectors[i]);

		if (!connector) {
			fprintf(stderr, "could not get connector %i: %d\n",
				resources->connectors[i], errno);
			continue;
		}

		printf("%d\t%d\t%s\t%s\t%dx%d\t\t%d\t",
		       connector->connector_id,
		       connector->encoder_id,
		       connector_status_str(connector->connection),
		       connector_type_str(connector->connector_type),
		       connector->mmWidth, connector->mmHeight,
		       connector->count_modes);

		for (j = 0; j < connector->count_encoders; j++)
			printf("%s%d", j > 0 ? ", " : "",
							connector->encoders[j]);
		printf("\n");

		if (!connector->count_modes)
			continue;

		printf("  modes:\n");
		printf("  name refresh (Hz) hdisp hss hse htot vdisp "
		       "vss vse vtot)\n");
		for (j = 0; j < connector->count_modes; j++)
			dump_mode(&connector->modes[j]);

		printf("  props:\n");
		dump_props(connector);

		drmModeFreeConnector(connector);
	}
	printf("\n");
}

static void dump_crtcs(void)
{
	drmModeCrtc *crtc;
	int i;

	printf("CRTCs:\n");
	printf("id\tfb\tpos\tsize\n");
	for (i = 0; i < resources->count_crtcs; i++) {
		crtc = drmModeGetCrtc(fd, resources->crtcs[i]);

		if (!crtc) {
			fprintf(stderr, "could not get crtc %i: %d\n",
				resources->crtcs[i], errno);
			continue;
		}
		printf("%d\t%d\t(%d,%d)\t(%dx%d)\n",
		       crtc->crtc_id,
		       crtc->buffer_id,
		       crtc->x, crtc->y,
		       crtc->width, crtc->height);
		dump_mode(&crtc->mode);

		drmModeFreeCrtc(crtc);
	}
	printf("\n");
}

static void dump_framebuffers(void)
{
	drmModeFB *fb;
	int i;

	printf("Frame buffers:\n");
	printf("id\tsize\tpitch\n");
	for (i = 0; i < resources->count_fbs; i++) {
		fb = drmModeGetFB(fd, resources->fbs[i]);

		if (!fb) {
			fprintf(stderr, "could not get fb %i: %d\n",
				resources->fbs[i], errno);
			continue;
		}
		printf("%u\t(%ux%u)\t%u\n",
		       fb->fb_id,
		       fb->width, fb->height,
		       fb->pitch);

		drmModeFreeFB(fb);
	}
	printf("\n");
}

/*
 * Mode setting with the kernel interfaces is a bit of a chore.
 * First you have to find the connector in question and make sure the
 * requested mode is available.
 * Then you need to find the encoder attached to that connector so you
 * can bind it with a free crtc.
 */

void connector_find_mode(struct connector *c)
{
	drmModeModeInfo *mode = NULL;
	drmModeConnector *connector;
	drmModeEncoder *encoder;
	uint32_t encoder_id = 0;
	int i, j;

	/* First, find the connector & mode */
	c->mode = NULL;
	for (i = 0; i < resources->count_connectors; i++) {
		connector = drmModeGetConnector(fd, resources->connectors[i]);

		if (!connector) {
			fprintf(stderr, "could not get connector %i: %d\n",
				resources->connectors[i], errno);
			continue;
		}

		if (!connector->count_modes) {
			drmModeFreeConnector(connector);
			continue;
		}

		if (connector->connector_id != c->id) {
			drmModeFreeConnector(connector);
			continue;
		}

		for (j = 0; j < connector->count_modes; j++) {
			if (!strncmp(connector->modes[j].name, c->mode_str,
			    DRM_DISPLAY_MODE_LEN)) {
				mode = drmMalloc(sizeof(*mode));
				if (!mode)
					break;

				memcpy(mode, &connector->modes[j], sizeof(*mode));
				encoder_id = connector->encoder_id;
				break;
			}
		}

		drmModeFreeConnector(connector);

		/* Found it, break out */
		if (mode)
			break;
	}

	if (!mode) {
		fprintf(stderr, "failed to find mode \"%s\"\n", c->mode_str);
		return;
	}

	c->mode = mode;

	/* Now get the encoder */
	for (i = 0; i < resources->count_encoders; i++) {
		uint32_t curr_encoder_id, crtc_id;

		encoder = drmModeGetEncoder(fd, resources->encoders[i]);

		if (!encoder) {
			fprintf(stderr, "could not get encoder %i: %d\n",
				resources->encoders[i], errno);
			continue;
		}

		curr_encoder_id = encoder->encoder_id;
		crtc_id = encoder->crtc_id;

		drmModeFreeEncoder(encoder);

		if (curr_encoder_id == encoder_id) {
			if (c->crtc == -1)
				c->crtc = crtc_id;
			break;
		}
	}
}

extern char *optarg;
extern int optind, opterr, optopt;
static char optstr[] = "ecpmfvDo:s:d:";

static void usage(char *name)
{
	fprintf(stderr, "usage: %s [-ecpmf]\n", name);
	fprintf(stderr, "\t-e\tlist encoders\n");
	fprintf(stderr, "\t-c\tlist connectors\n");
	fprintf(stderr, "\t-p\tlist CRTCs (pipes)\n");
	fprintf(stderr, "\t-m\tlist modes\n");
	fprintf(stderr, "\t-f\tlist framebuffers\n");
	fprintf(stderr, "\t-v\ttest vsynced page flipping\n");
	fprintf(stderr, "\t-o\tlist of operation id : 0: M2M, 1: Writeback, 2: Output\n");
	fprintf(stderr, "\t-D\ttest M2M Display Mode\n");
	fprintf(stderr, "\t-d\tlist of degree operation : 0: 0, 1: 90, 2, 180, 3, 270\n");
	fprintf(stderr, "\t-s <connector_id>:<mode>\tset a mode\n");
	fprintf(stderr, "\t-s <connector_id>#<src_resolution>:<mode>\tset a mode\n");
	fprintf(stderr, "\t-s <connector_id>@<crtc_id>:<mode>\tset a mode\n");
	fprintf(stderr, "\t-s <connector_id>@<crtc_id>#<src_resolution>:<mode>\tset a mode\n");
	fprintf(stderr, "\n\tDefault is to dump all info.\n");
	exit(0);
}

#define dump_resource(res) if (res) dump_##res()

int main(int argc, char **argv)
{
	struct device dev;
	int c;
	int operations = 0, encoders = 0, connectors = 0, crtcs = 0, framebuffers = 0;
	int degree = EXYNOS_DRM_DEGREE_180;
	int display = IPP_CMD_M2M_FILE;
	int test_vsync = 0;
	const char *module = "exynos";
	int i, count = 0;
	struct connector con_args[2];

	memset(&dev, 0, sizeof(dev));

	opterr = 0;
	while ((c = getopt(argc, argv, optstr)) != -1) {
		switch (c) {
		case 'e':
			encoders = 1;
			break;
		case 'c':
			connectors = 1;
			break;
		case 'p':
			crtcs = 1;
			break;
		case 'm':
			modes = 1;
			break;
		case 'f':
			framebuffers = 1;
			break;
		case 'v':
			test_vsync = 1;
			break;
		case 'o':
			if (optarg)
				sscanf(optarg, "%d", &operations);
			break;
		case 's':
			con_args[count].crtc = -1;
			con_args[count].src_sz.hsize = 720;
			con_args[count].src_sz.vsize = 1280;
			if (sscanf(optarg, "%d:%64s",
				   &con_args[count].id,
				   con_args[count].mode_str) != 2 &&
			    sscanf(optarg, "%d#%ux%u:%64s",
				   &con_args[count].id,
				   &con_args[count].src_sz.hsize,
				   &con_args[count].src_sz.vsize,
				   con_args[count].mode_str) != 4 &&
			    sscanf(optarg, "%d@%d:%64s",
				   &con_args[count].id,
				   &con_args[count].crtc,
				   con_args[count].mode_str) != 3 &&
			    sscanf(optarg, "%d@%d#%ux%u:%64s",
				   &con_args[count].id,
				   &con_args[count].crtc,
				   &con_args[count].src_sz.hsize,
				   &con_args[count].src_sz.vsize,
				   con_args[count].mode_str) != 5)
				usage(argv[0]);
			count++;
			break;
		case 'd':
			if (optarg)
				sscanf(optarg, "%d", &degree);
			break;
		case 'D':
			display = 1;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	if (argc == 1)
		encoders = connectors = crtcs = modes = framebuffers = 1;

	fd = drmOpen(module, NULL);
	if (fd < 0) {
		fprintf(stderr, "failed to load %s module, aborting.\n", module);
		return -1;
	}

	dev.fd = fd;

	dev.resources = get_resources(&dev);
	if (!dev.resources) {
		fprintf(stderr, "get_resources failed: %d\n", errno);
		drmClose(dev.fd);
		return 1;
	}
	resources = dev.resources->res;

	dump_resource(encoders);
	dump_resource(connectors);
	dump_resource(crtcs);
	dump_resource(framebuffers);

	if (count > 0) {
		long int sum = 0, usec[MAX_LOOP] = {0, };
		int ret;

		switch(operations) {
		case 0:
			if (degree < EXYNOS_DRM_DEGREE_0 ||
					degree > EXYNOS_DRM_DEGREE_270) {
				fprintf(stderr, "not support degree\n");
				break;
			}

			ret = kms_create(dev.fd, &dev.kms);
			if (ret) {
				fprintf(stderr,
					"failed to create kms driver: %d\n",
					ret);
				break;
			}

			fimc_m2m_set_mode(&dev, con_args, count,
						degree,	display, usec);
			kms_destroy(&dev.kms);
			break;
		case 1:
			fimc_wb_set_mode(con_args, count, test_vsync, usec);
			break;
		case 2:
			fimc_output_set_mode(con_args, count, test_vsync, usec);
			break;
		default:
			break;
		}

		if (display == IPP_CMD_M2M_FILE) {
			for (i = 0; i < MAX_LOOP && usec[i] != 0; i++) {
				printf("[%d] : %ld\n", i + 1, usec[i]);
				sum += usec[i];
			}
			printf("fimc : result files are in %s\n", RESULT_PATH);
			printf("avg : [%ld]\n", (i ? sum / i : 0));
		}
		getchar();
	}

	free_resources(dev.resources);

	return 0;
}
