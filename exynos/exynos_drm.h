/* exynos_drm.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 * Authors:
 *	Inki Dae <inki.dae@samsung.com>
 *	Joonyoung Shim <jy0922.shim@samsung.com>
 *	Seung-Woo Kim <sw0312.kim@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _EXYNOS_DRM_H_
#define _EXYNOS_DRM_H_

#include "drm.h"

/**
 * User-desired buffer creation information structure.
 *
 * @size: user-desired memory allocation size.
 *	- this size value would be page-aligned internally.
 * @flags: user request for setting memory type or cache attributes.
 * @handle: returned a handle to created gem object.
 *	- this handle will be set by gem module of kernel side.
 */
struct drm_exynos_gem_create {
	uint64_t size;
	unsigned int flags;
	unsigned int handle;
};

/**
 * A structure for getting a fake-offset that can be used with mmap.
 *
 * @handle: handle of gem object.
 * @reserved: just padding to be 64-bit aligned.
 * @offset: a fake-offset of gem object.
 */
struct drm_exynos_gem_map {
	__u32 handle;
	__u32 reserved;
	__u64 offset;
};

/**
 * A structure to gem information.
 *
 * @handle: a handle to gem object created.
 * @flags: flag value including memory type and cache attribute and
 *	this value would be set by driver.
 * @size: size to memory region allocated by gem and this size would
 *	be set by driver.
 */
struct drm_exynos_gem_info {
	unsigned int handle;
	unsigned int flags;
	uint64_t size;
};

/**
 * A structure for user connection request of virtual display.
 *
 * @connection: indicate whether doing connection or not by user.
 * @extensions: if this value is 1 then the vidi driver would need additional
 *	128bytes edid data.
 * @edid: the edid data pointer from user side.
 */
struct drm_exynos_vidi_connection {
	unsigned int connection;
	unsigned int extensions;
	uint64_t edid;
};

/* memory type definitions. */
enum e_drm_exynos_gem_mem_type {
	/* Physically Continuous memory and used as default. */
	EXYNOS_BO_CONTIG	= 0 << 0,
	/* Physically Non-Continuous memory. */
	EXYNOS_BO_NONCONTIG	= 1 << 0,
	/* non-cachable mapping and used as default. */
	EXYNOS_BO_NONCACHABLE	= 0 << 1,
	/* cachable mapping. */
	EXYNOS_BO_CACHABLE	= 1 << 1,
	/* write-combine mapping. */
	EXYNOS_BO_WC		= 1 << 2,
	EXYNOS_BO_MASK		= EXYNOS_BO_NONCONTIG | EXYNOS_BO_CACHABLE |
					EXYNOS_BO_WC
};

struct drm_exynos_g2d_get_ver {
	__u32	major;
	__u32	minor;
};

struct drm_exynos_g2d_cmd {
	__u32	offset;
	__u32	data;
};

enum drm_exynos_g2d_buf_type {
	G2D_BUF_USERPTR = 1 << 31,
};

enum drm_exynos_g2d_event_type {
	G2D_EVENT_NOT,
	G2D_EVENT_NONSTOP,
	G2D_EVENT_STOP,		/* not yet */
};

struct drm_exynos_g2d_userptr {
	unsigned long userptr;
	unsigned long size;
};

struct drm_exynos_g2d_set_cmdlist {
	__u64					cmd;
	__u64					cmd_buf;
	__u32					cmd_nr;
	__u32					cmd_buf_nr;

	/* for g2d event */
	__u64					event_type;
	__u64					user_data;
};

struct drm_exynos_g2d_exec {
	__u64					async;
};

/* definition of operations types */
enum drm_exynos_ops_id {
	EXYNOS_DRM_OPS_SRC,
	EXYNOS_DRM_OPS_DST,
	EXYNOS_DRM_OPS_MAX,
};

/* definition of size */
struct drm_exynos_sz {
	__u32	hsize;
	__u32	vsize;
};

/* definition of position */
struct drm_exynos_pos {
	__u32	x;
	__u32	y;
	__u32	w;
	__u32	h;
};

/* definition of flip */
enum drm_exynos_flip {
	EXYNOS_DRM_FLIP_NONE = (0 << 0),
	EXYNOS_DRM_FLIP_VERTICAL = (1 << 0),
	EXYNOS_DRM_FLIP_HORIZONTAL = (1 << 1),
	EXYNOS_DRM_FLIP_BOTH = EXYNOS_DRM_FLIP_VERTICAL |
			EXYNOS_DRM_FLIP_HORIZONTAL,
};

/* definition of rotation degree */
enum drm_exynos_degree {
	EXYNOS_DRM_DEGREE_0,
	EXYNOS_DRM_DEGREE_90,
	EXYNOS_DRM_DEGREE_180,
	EXYNOS_DRM_DEGREE_270,
};

/* definition of planar */
enum drm_exynos_planer {
	EXYNOS_DRM_PLANAR_Y,
	EXYNOS_DRM_PLANAR_CB,
	EXYNOS_DRM_PLANAR_CR,
	EXYNOS_DRM_PLANAR_MAX,
};

/* define of blending operation */
enum drm_exynos_ipp_blending {
	IPP_BLENDING_NO,
	/* [0, 0] */
	IPP_BLENDING_CLR,
	/* [Sa, Sc] */
	IPP_BLENDING_SRC,
	/* [Da, Dc] */
	IPP_BLENDING_DST,
	/* [Sa + (1 - Sa)*Da, Rc = Sc + (1 - Sa)*Dc] */
	IPP_BLENDING_SRC_OVER,
	/* [Sa + (1 - Sa)*Da, Rc = Dc + (1 - Da)*Sc] */
	IPP_BLENDING_DST_OVER,
	/* [Sa * Da, Sc * Da] */
	IPP_BLENDING_SRC_IN,
	/* [Sa * Da, Sa * Dc] */
	IPP_BLENDING_DST_IN,
	/* [Sa * (1 - Da), Sc * (1 - Da)] */
	IPP_BLENDING_SRC_OUT,
	/* [Da * (1 - Sa), Dc * (1 - Sa)] */
	IPP_BLENDING_DST_OUT,
	/* [Da, Sc * Da + (1 - Sa) * Dc] */
	IPP_BLENDING_SRC_ATOP,
	/* [Sa, Sc * (1 - Da) + Sa * Dc ] */
	IPP_BLENDING_DST_ATOP,
	/* [-(Sa * Da), Sc * (1 - Da) + (1 - Sa) * Dc] */
	IPP_BLENDING_XOR,
	/* [Sa + Da - Sa*Da, Sc*(1 - Da) + Dc*(1 - Sa) + min(Sc, Dc)] */
	IPP_BLENDING_DARKEN,
	/* [Sa + Da - Sa*Da, Sc*(1 - Da) + Dc*(1 - Sa) + max(Sc, Dc)] */
	IPP_BLENDING_LIGHTEN,
	/* [Sa * Da, Sc * Dc] */
	IPP_BLENDING_MULTIPLY,
	/* [Sa + Da - Sa * Da, Sc + Dc - Sc * Dc] */
	IPP_BLENDING_SCREEN,
	/* Saturate(S + D) */
	IPP_BLENDING_ADD,
	/* Max */
	IPP_BLENDING_MAX,
};

/* define of dithering operation */
enum drm_exynos_ipp_dithering {
	IPP_DITHERING_NO,
	IPP_DITHERING_8BIT,
	IPP_DITHERING_6BIT,
	IPP_DITHERING_5BIT,
	IPP_DITHERING_4BIT,
	IPP_DITHERING_MAX,
};

/**
 * A structure for ipp supported property list.
 *
 * @version: version of this structure.
 * @ipp_id: id of ipp driver.
 * @count: count of ipp driver.
 * @writeback: flag of writeback supporting.
 * @flip: flag of flip supporting.
 * @degree: flag of degree information.
 * @csc: flag of csc supporting.
 * @crop: flag of crop supporting.
 * @scale: flag of scale supporting.
 * @refresh_min: min hz of refresh.
 * @refresh_max: max hz of refresh.
 * @crop_min: crop min resolution.
 * @crop_max: crop max resolution.
 * @scale_min: scale min resolution.
 * @scale_max: scale max resolution.
 * @rot_max: rotation max resolution.
 */
struct drm_exynos_ipp_prop_list {
	__u32	version;
	__u32	ipp_id;
	__u32	count;
	__u32	writeback;
	__u32	flip;
	__u32	degree;
	__u32	csc;
	__u32	crop;
	__u32	scale;
	__u32	refresh_min;
	__u32	refresh_max;
	__u32	reserved;
	struct drm_exynos_sz	crop_min;
	struct drm_exynos_sz	crop_max;
	struct drm_exynos_sz	scale_min;
	struct drm_exynos_sz	scale_max;
	struct drm_exynos_sz	rot_max;
};

/**
 * A structure for ipp config.
 *
 * @ops_id: property of operation directions.
 * @flip: property of mirror, flip.
 * @degree: property of rotation degree.
 * @fmt: property of image format.
 * @sz: property of image size.
 * @pos: property of image position(src-cropped,dst-scaler).
 */
struct drm_exynos_ipp_config {
	enum drm_exynos_ops_id ops_id;
	enum drm_exynos_flip	flip;
	enum drm_exynos_degree	degree;
	__u32	fmt;
	struct drm_exynos_sz	sz;
	struct drm_exynos_pos	pos;
};

/* definition of command */
enum drm_exynos_ipp_cmd {
	IPP_CMD_NONE,
	IPP_CMD_M2M,
	IPP_CMD_WB,
	IPP_CMD_OUTPUT,
	IPP_CMD_MAX,
};

/* define of M2M command */
enum drm_exynos_ipp_cmd_m2m {
	IPP_CMD_M2M_FILE,
	IPP_CMD_M2M_DISPLAY,
	IPP_CMD_M2M_NONE,
};

/* define of color range */
enum drm_exynos_color_range {
	COLOR_RANGE_LIMITED,	/* Narrow: Y(16 to 235), Cb/Cr(16 to 240) */
	COLOR_RANGE_FULL,	/* Wide: Y/Cb/Cr(0 to 255), Wide default */
};

/**
 * A structure for ipp property.
 *
 * @config: source, destination config.
 * @cmd: definition of command.
 * @ipp_id: id of ipp driver.
 * @prop_id: id of property.
 * @refresh_rate: refresh rate.
 * @range: dynamic range for csc.
 * @blending: blending opeation config.
 * @dithering: dithering opeation config.
 * @color_fill: color fill value.
 */
struct drm_exynos_ipp_property {
	struct drm_exynos_ipp_config config[EXYNOS_DRM_OPS_MAX];
	enum drm_exynos_ipp_cmd	cmd;
	__u32	ipp_id;
	__u32	prop_id;
	__u32	refresh_rate;
	__u32	range;
	__u32	blending;
	__u32	dithering;
	__u32	color_fill;
};

/* definition of buffer */
enum drm_exynos_ipp_buf_type {
	IPP_BUF_ENQUEUE,
	IPP_BUF_DEQUEUE,
};

/**
 * A structure for ipp buffer operations.
 *
 * @ops_id: operation directions.
 * @buf_type: definition of buffer.
 * @prop_id: id of property.
 * @buf_id: id of buffer.
 * @handle: Y, Cb, Cr each planar handle.
 * @user_data: user data.
 */
struct drm_exynos_ipp_queue_buf {
	enum drm_exynos_ops_id	ops_id;
	enum drm_exynos_ipp_buf_type	buf_type;
	__u32	prop_id;
	__u32	buf_id;
	__u32	handle[EXYNOS_DRM_PLANAR_MAX];
	__u32	reserved;
	__u64	user_data;
};

/* definition of control */
enum drm_exynos_ipp_ctrl {
	IPP_CTRL_PLAY,
	IPP_CTRL_STOP,
	IPP_CTRL_PAUSE,
	IPP_CTRL_RESUME,
	IPP_CTRL_MAX,
};

/**
 * A structure for ipp start/stop operations.
 *
 * @prop_id: id of property.
 * @ctrl: definition of control.
 */
struct drm_exynos_ipp_cmd_ctrl {
	__u32	prop_id;
	enum drm_exynos_ipp_ctrl	ctrl;
};

#define DRM_EXYNOS_GEM_CREATE		0x00
#define DRM_EXYNOS_GEM_MAP		0x01
/* Reserved 0x04 ~ 0x05 for exynos specific gem ioctl */
#define DRM_EXYNOS_GEM_GET		0x04
#define DRM_EXYNOS_VIDI_CONNECTION	0x07

/* G2D */
#define DRM_EXYNOS_G2D_GET_VER		0x20
#define DRM_EXYNOS_G2D_SET_CMDLIST	0x21
#define DRM_EXYNOS_G2D_EXEC		0x22

/* IPP - Image Post Processing */
#define DRM_EXYNOS_IPP_GET_PROPERTY	0x30
#define DRM_EXYNOS_IPP_SET_PROPERTY	0x31
#define DRM_EXYNOS_IPP_QUEUE_BUF	0x32
#define DRM_EXYNOS_IPP_CMD_CTRL	0x33

#define DRM_IOCTL_EXYNOS_GEM_CREATE		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_GEM_CREATE, struct drm_exynos_gem_create)
#define DRM_IOCTL_EXYNOS_GEM_MAP		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_GEM_MAP, struct drm_exynos_gem_map)
#define DRM_IOCTL_EXYNOS_GEM_GET	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_GEM_GET,	struct drm_exynos_gem_info)

#define DRM_IOCTL_EXYNOS_VIDI_CONNECTION	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_VIDI_CONNECTION, struct drm_exynos_vidi_connection)

#define DRM_IOCTL_EXYNOS_G2D_GET_VER		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_G2D_GET_VER, struct drm_exynos_g2d_get_ver)
#define DRM_IOCTL_EXYNOS_G2D_SET_CMDLIST	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_G2D_SET_CMDLIST, struct drm_exynos_g2d_set_cmdlist)
#define DRM_IOCTL_EXYNOS_G2D_EXEC		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_G2D_EXEC, struct drm_exynos_g2d_exec)

#define DRM_IOCTL_EXYNOS_IPP_GET_PROPERTY	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_IPP_GET_PROPERTY, struct drm_exynos_ipp_prop_list)
#define DRM_IOCTL_EXYNOS_IPP_SET_PROPERTY	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_IPP_SET_PROPERTY, struct drm_exynos_ipp_property)
#define DRM_IOCTL_EXYNOS_IPP_QUEUE_BUF	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_IPP_QUEUE_BUF, struct drm_exynos_ipp_queue_buf)
#define DRM_IOCTL_EXYNOS_IPP_CMD_CTRL		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_IPP_CMD_CTRL, struct drm_exynos_ipp_cmd_ctrl)

/* EXYNOS specific events */
#define DRM_EXYNOS_G2D_EVENT		0x80000000
#define DRM_EXYNOS_IPP_EVENT		0x80000001

struct drm_exynos_g2d_event {
	struct drm_event	base;
	__u64				user_data;
	__u32				tv_sec;
	__u32				tv_usec;
	__u32				cmdlist_no;
	__u32				reserved;
};

struct drm_exynos_ipp_event {
	struct drm_event	base;
	__u64			user_data;
	__u32			tv_sec;
	__u32			tv_usec;
	__u32			prop_id;
	__u32			reserved;
	__u32			buf_id[EXYNOS_DRM_OPS_MAX];
};

/* Exynos DRM IPP v2 API */

/**
 * Enumerate available IPP hardware modules.
 *
 * @count_ipps: size of ipp_id array / number of ipp modules (set by driver)
 * @reserved: padding
 * @ipp_id_ptr: pointer to ipp_id array or NULL
 */
struct drm_exynos_ioctl_ipp_get_res {
	__u32 count_ipps;
	__u32 reserved;
	__u64 ipp_id_ptr;
};

enum drm_exynos_ipp_format_type {
	DRM_EXYNOS_IPP_FORMAT_SOURCE		= 0x01,
	DRM_EXYNOS_IPP_FORMAT_DESTINATION	= 0x02,
};

struct drm_exynos_ipp_format {
	__u32 fourcc;
	__u32 type;
	__u64 modifier;
};

enum drm_exynos_ipp_capability {
	DRM_EXYNOS_IPP_CAP_CROP		= 0x01,
	DRM_EXYNOS_IPP_CAP_ROTATE	= 0x02,
	DRM_EXYNOS_IPP_CAP_SCALE	= 0x04,
	DRM_EXYNOS_IPP_CAP_CONVERT	= 0x08,
};

/**
 * Get IPP hardware capabilities and supported image formats.
 *
 * @ipp_id: id of IPP module to query
 * @capabilities: bitmask of drm_exynos_ipp_capability (set by driver)
 * @reserved: padding
 * @formats_count: size of formats array (in entries) / number of filled
 *		   formats (set by driver)
 * @formats_ptr: pointer to formats array or NULL
 */
struct drm_exynos_ioctl_ipp_get_caps {
	__u32 ipp_id;
	__u32 capabilities;
	__u32 reserved;
	__u32 formats_count;
	__u64 formats_ptr;
};

enum drm_exynos_ipp_limit_type {
	/* size (horizontal/vertial) limits, in pixels (min, max, alignment) */
	DRM_EXYNOS_IPP_LIMIT_TYPE_SIZE		= 0x0001,
	/* scale ratio (horizonta/vertial), 16.16 fixed point (min, max) */
	DRM_EXYNOS_IPP_LIMIT_TYPE_SCALE		= 0x0002,

	/* image buffer area */
	DRM_EXYNOS_IPP_LIMIT_SIZE_BUFFER	= 0x0001 << 16,
	/* src/dst rectangle area */
	DRM_EXYNOS_IPP_LIMIT_SIZE_AREA		= 0x0002 << 16,
	/* src/dst rectangle area when rotation enabled */
	DRM_EXYNOS_IPP_LIMIT_SIZE_ROTATED	= 0x0003 << 16,

	DRM_EXYNOS_IPP_LIMIT_TYPE_MASK		= 0x000f,
	DRM_EXYNOS_IPP_LIMIT_SIZE_MASK		= 0x000f << 16,
};

struct drm_exynos_ipp_limit_val {
	__u32 min;
	__u32 max;
	__u32 align;
	__u32 reserved;
};

/**
 * IPP module limitation.
 *
 * @type: limit type (see drm_exynos_ipp_limit_type enum)
 * @reserved: padding
 * @h: horizontal limits
 * @v: vertical limits
 */
struct drm_exynos_ipp_limit {
	__u32 type;
	__u32 reserved;
	struct drm_exynos_ipp_limit_val h;
	struct drm_exynos_ipp_limit_val v;
};

/**
 * Get IPP limits for given image format.
 *
 * @ipp_id: id of IPP module to query
 * @fourcc: image format code (see DRM_FORMAT_* in drm_fourcc.h)
 * @modifier: image format modifier (see DRM_FORMAT_MOD_* in drm_fourcc.h)
 * @type: source/destination identifier (drm_exynos_ipp_format_flag enum)
 * @limits_count: size of limits array (in entries) / number of filled entries
 *		 (set by driver)
 * @limits_ptr: pointer to limits array or NULL
 */
struct drm_exynos_ioctl_ipp_get_limits {
	__u32 ipp_id;
	__u32 fourcc;
	__u64 modifier;
	__u32 type;
	__u32 limits_count;
	__u64 limits_ptr;
};

enum drm_exynos_ipp_task_id {
	/* buffer described by struct drm_exynos_ipp_task_buffer */
	DRM_EXYNOS_IPP_TASK_BUFFER		= 0x0001,
	/* rectangle described by struct drm_exynos_ipp_task_rect */
	DRM_EXYNOS_IPP_TASK_RECTANGLE		= 0x0002,
	/* transformation described by struct drm_exynos_ipp_task_transform */
	DRM_EXYNOS_IPP_TASK_TRANSFORM		= 0x0003,
	/* alpha configuration described by struct drm_exynos_ipp_task_alpha */
	DRM_EXYNOS_IPP_TASK_ALPHA		= 0x0004,

	/* source image data (for buffer and rectangle chunks) */
	DRM_EXYNOS_IPP_TASK_TYPE_SOURCE		= 0x0001 << 16,
	/* destination image data (for buffer and rectangle chunks) */
	DRM_EXYNOS_IPP_TASK_TYPE_DESTINATION	= 0x0002 << 16,
};

/**
 * Memory buffer with image data.
 *
 * @id: must be DRM_EXYNOS_IPP_TASK_BUFFER
 * other parameters are same as for AddFB2 generic DRM ioctl
 */
struct drm_exynos_ipp_task_buffer {
	__u32	id;
	__u32	fourcc;
	__u32	width, height;
	__u32	gem_id[4];
	__u32	offset[4];
	__u32	pitch[4];
	__u64	modifier;
};

/**
 * Rectangle for processing.
 *
 * @id: must be DRM_EXYNOS_IPP_TASK_RECTANGLE
 * @reserved: padding
 * @x,@y: left corner in pixels
 * @w,@h: width/height in pixels
 */
struct drm_exynos_ipp_task_rect {
	__u32	id;
	__u32	reserved;
	__u32	x;
	__u32	y;
	__u32	w;
	__u32	h;
};

/**
 * Image tranformation description.
 *
 * @id: must be DRM_EXYNOS_IPP_TASK_TRANSFORM
 * @rotation: DRM_MODE_ROTATE_* and DRM_MODE_REFLECT_* values
 */
struct drm_exynos_ipp_task_transform {
	__u32	id;
	__u32	rotation;
};

/**
 * Image global alpha configuration for formats without alpha values.
 *
 * @id: must be DRM_EXYNOS_IPP_TASK_ALPHA
 * @value: global alpha value (0-255)
 */
struct drm_exynos_ipp_task_alpha {
	__u32	id;
	__u32	value;
};

enum drm_exynos_ipp_flag {
	/* generate DRM event after processing */
	DRM_EXYNOS_IPP_FLAG_EVENT	= 0x01,
	/* dry run, only check task parameters */
	DRM_EXYNOS_IPP_FLAG_TEST_ONLY	= 0x02,
	/* non-blocking processing */
	DRM_EXYNOS_IPP_FLAG_NONBLOCK	= 0x04,
};

#define DRM_EXYNOS_IPP_FLAGS (DRM_EXYNOS_IPP_FLAG_EVENT |\
		DRM_EXYNOS_IPP_FLAG_TEST_ONLY | DRM_EXYNOS_IPP_FLAG_NONBLOCK)

/**
 * Perform image processing described by array of drm_exynos_ipp_task_*
 * structures (parameters array).
 *
 * @ipp_id: id of IPP module to run the task
 * @flags: bitmask of drm_exynos_ipp_flag values
 * @reserved: padding
 * @params_size: size of parameters array (in bytes)
 * @params_ptr: pointer to parameters array or NULL
 * @user_data: (optional) data for drm event
 */
struct drm_exynos_ioctl_ipp_commit {
	__u32 ipp_id;
	__u32 flags;
	__u32 reserved;
	__u32 params_size;
	__u64 params_ptr;
	__u64 user_data;
};

#define DRM_EXYNOS_IPP_GET_RESOURCES	0x40
#define DRM_EXYNOS_IPP_GET_CAPS		0x41
#define DRM_EXYNOS_IPP_GET_LIMITS	0x42
#define DRM_EXYNOS_IPP_COMMIT		0x43

#define DRM_IOCTL_EXYNOS_IPP_GET_RESOURCES	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_IPP_GET_RESOURCES, struct drm_exynos_ioctl_ipp_get_res)
#define DRM_IOCTL_EXYNOS_IPP_GET_CAPS		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_IPP_GET_CAPS, struct drm_exynos_ioctl_ipp_get_caps)
#define DRM_IOCTL_EXYNOS_IPP_GET_LIMITS		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_IPP_GET_LIMITS, struct drm_exynos_ioctl_ipp_get_limits)
#define DRM_IOCTL_EXYNOS_IPP_COMMIT		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_EXYNOS_IPP_COMMIT, struct drm_exynos_ioctl_ipp_commit)

/* EXYNOS specific events */
#define DRM_EXYNOS_IPP2_EVENT		0x80000002

struct drm_exynos_ipp2_event {
	struct drm_event	base;
	__u64			user_data;
	__u32			tv_sec;
	__u32			tv_usec;
	__u32			ipp_id;
	__u32			sequence;
	__u64			reserved;
};

/* rotation property bits */
#define DRM_MODE_ROTATE_0       (1<<0)
#define DRM_MODE_ROTATE_90      (1<<1)
#define DRM_MODE_ROTATE_180     (1<<2)
#define DRM_MODE_ROTATE_270     (1<<3)
#define DRM_MODE_REFLECT_X	(1<<4)
#define DRM_MODE_REFLECT_Y	(1<<5)

#define DRM_MODE_ROTATE_MASK (\
            DRM_MODE_ROTATE_0  | DRM_MODE_ROTATE_90  | \
            DRM_MODE_ROTATE_180 | DRM_MODE_ROTATE_270)


#endif
