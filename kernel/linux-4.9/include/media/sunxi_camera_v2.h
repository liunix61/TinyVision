/*
 * include/media/sunxi_camera_v2.h -- Ctrl IDs definitions for sunxi-vin
 *
 * Copyright (C) 2014 Allwinnertech Co., Ltd.
 * Copyright (C) 2015 Yang Feng
 *
 * Author: Yang Feng <yangfeng@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */
#ifndef _SUNXI_CAMERA_H_
#define _SUNXI_CAMERA_H_

#include <linux/types.h>
#include <linux/videodev2.h>
#include <stdbool.h>

/*  Flags for 'capability' and 'capturemode' fields */
#define V4L2_MODE_HIGHQUALITY		0x0001
#define V4L2_MODE_VIDEO			0x0002
#define V4L2_MODE_IMAGE			0x0003
#define V4L2_MODE_PREVIEW		0x0004

/*  for yuv420 FBC mode*/
#define V4L2_PIX_FMT_FBC           v4l2_fourcc('F', 'C', '2', '1')
#define V4L2_PIX_FMT_LBC_2_0X      v4l2_fourcc('L', 'C', '2', '1')
#define V4L2_PIX_FMT_LBC_2_5X      v4l2_fourcc('L', 'C', '2', '2')
#define V4L2_PIX_FMT_LBC_1_0X      v4l2_fourcc('L', 'C', '2', '3')
#define V4L2_PIX_FMT_LBC_1_5X      v4l2_fourcc('L', 'C', '2', '4')

/*
 *	USER CIDS
 */
struct v4l2_win_coordinate {
	__s32 x1;
	__s32 y1;
	__s32 x2;
	__s32 y2;
};

#define V4L2_FLASH_LED_MODE_AUTO		(V4L2_FLASH_LED_MODE_TORCH + 1)
#define V4L2_FLASH_LED_MODE_RED_EYE		(V4L2_FLASH_LED_MODE_TORCH + 2)

struct v4l2_win_setting {
	__s32 metering_mode;
	struct v4l2_win_coordinate coor;
};

enum v4l2_gain_shift {
	V4L2_GAIN_SHIFT = 0,
	V4L2_SHARP_LEVEL_SHIFT = 8,
	V4L2_SHARP_MIN_SHIFT = 20,
	V4L2_NDF_SHIFT = 26,
};

#define MAX_EXP_FRAMES     5

/*
 * The base for the sunxi-vfe controls.
 * Total of 64 controls is reserved for this driver, add by yangfeng
 */
#define V4L2_CID_USER_SUNXI_CAMERA_BASE		(V4L2_CID_USER_BASE + 0x1050)

#define V4L2_CID_AUTO_FOCUS_INIT	(V4L2_CID_USER_SUNXI_CAMERA_BASE + 2)
#define V4L2_CID_AUTO_FOCUS_RELEASE	(V4L2_CID_USER_SUNXI_CAMERA_BASE + 3)
#define V4L2_CID_GSENSOR_ROTATION	(V4L2_CID_USER_SUNXI_CAMERA_BASE + 4)
#define V4L2_CID_FRAME_RATE             (V4L2_CID_USER_SUNXI_CAMERA_BASE + 5)

enum v4l2_take_picture {
	V4L2_TAKE_PICTURE_STOP = 0,
	V4L2_TAKE_PICTURE_NORM = 1,
	V4L2_TAKE_PICTURE_FAST = 2,
	V4L2_TAKE_PICTURE_FLASH = 3,
	V4L2_TAKE_PICTURE_HDR = 4,
};
struct isp_hdr_setting_t {
	__s32 hdr_en;
	__s32 hdr_mode;
	__s32 frames_count;
	__s32 total_frames;
	__s32 values[MAX_EXP_FRAMES];
};
struct csi_sync_ctrl {
	__s32 type;
	__s32 prs_sync_en;
	__s32 prs_sync_scr_sel;
	__s32 prs_sync_bench_sel;
	__s32 prs_sync_input_vsync_en;
	__s32 prs_sync_singal_via_by;
	__s32 prs_sync_singal_scr_sel;
	__s32 prs_sync_pulse_cfg;
	__s32 prs_sync_dist;
	__s32 prs_sync_wait_n;
	__s32 prs_sync_wait_m;
	__s32 dma_clr_dist;

	__s32 prs_xvs_out_en;
	__s32 prs_xhs_out_en;
	__s32 prs_xvs_t;
	__s32 prs_xhs_t;
	__s32 prs_xvs_len;
	__s32 prs_xhs_len;
};

#define HDR_CTRL_GET    0
#define HDR_CTRL_SET     1
struct isp_hdr_ctrl {
	__s32 flag;
	__s32 count;
	struct isp_hdr_setting_t hdr_t;
};

#define V4L2_CID_TAKE_PICTURE	(V4L2_CID_USER_SUNXI_CAMERA_BASE + 6)

typedef union {
	unsigned int dwval;
	struct {
		unsigned int af_sharp:16;
		unsigned int hdr_cnt:4;
		unsigned int flash_ok:1;
		unsigned int capture_ok:1;
		unsigned int fast_capture_ok:1;
		unsigned int res0:9;
	} bits;
} IMAGE_FLAG_t;

typedef struct isp_to_user_params {
	IMAGE_FLAG_t image_para;
} isp_to_user_params_t;

typedef struct user_to_isp_params {
	IMAGE_FLAG_t image_para;
} user_to_isp_params_t;

typedef struct isp_image_params {
	isp_to_user_params_t isp_image_params;
	user_to_isp_params_t user_image_params;
} isp_image_params_t;

enum user_v4l2_colorspace {
#if 0	// v4l2_colorspace sample
	/*
	 * Default colorspace, i.e. let the driver figure it out.
	 * Can only be used with video capture.
	 */
	V4L2_COLORSPACE_DEFAULT       = 0,

	/* SMPTE 170M: used for broadcast NTSC/PAL SDTV */
	V4L2_COLORSPACE_SMPTE170M     = 1,

	/* Obsolete pre-1998 SMPTE 240M HDTV standard, superseded by Rec 709 */
	V4L2_COLORSPACE_SMPTE240M     = 2,

	/* Rec.709: used for HDTV */
	V4L2_COLORSPACE_REC709        = 3,

	/*
	 * Deprecated, do not use. No driver will ever return this. This was
	 * based on a misunderstanding of the bt878 datasheet.
	 */
	V4L2_COLORSPACE_BT878         = 4,

	/*
	 * NTSC 1953 colorspace. This only makes sense when dealing with
	 * really, really old NTSC recordings. Superseded by SMPTE 170M.
	 */
	V4L2_COLORSPACE_470_SYSTEM_M  = 5,

	/*
	 * EBU Tech 3213 PAL/SECAM colorspace. This only makes sense when
	 * dealing with really old PAL/SECAM recordings. Superseded by
	 * SMPTE 170M.
	 */
	V4L2_COLORSPACE_470_SYSTEM_BG = 6,

	/*
	 * Effectively shorthand for V4L2_COLORSPACE_SRGB, V4L2_YCBCR_ENC_601
	 * and V4L2_QUANTIZATION_FULL_RANGE. To be used for (Motion-)JPEG.
	 */
	V4L2_COLORSPACE_JPEG          = 7,

	/* For RGB colorspaces such as produces by most webcams. */
	V4L2_COLORSPACE_SRGB          = 8,

	/* AdobeRGB colorspace */
	V4L2_COLORSPACE_ADOBERGB      = 9,

	/* BT.2020 colorspace, used for UHDTV. */
	V4L2_COLORSPACE_BT2020        = 10,

	/* Raw colorspace: for RAW unprocessed images */
	V4L2_COLORSPACE_RAW           = 11,

	/* DCI-P3 colorspace, used by cinema projectors */
	V4L2_COLORSPACE_DCI_P3        = 12,
#endif
	V4L2_COLORSPACE_REC709_PART_RANGE = 31,
};

#define  V4L2_CID_HOR_VISUAL_ANGLE	(V4L2_CID_USER_SUNXI_CAMERA_BASE + 7)
#define  V4L2_CID_VER_VISUAL_ANGLE	(V4L2_CID_USER_SUNXI_CAMERA_BASE + 8)
#define  V4L2_CID_FOCUS_LENGTH		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 9)
#define  V4L2_CID_R_GAIN		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 10)
#define  V4L2_CID_GR_GAIN		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 11)
#define  V4L2_CID_GB_GAIN		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 12)
#define  V4L2_CID_B_GAIN		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 13)

enum v4l2_sensor_type {
	V4L2_SENSOR_TYPE_YUV = 0,
	V4L2_SENSOR_TYPE_RAW = 1,
};

#define V4L2_CID_SENSOR_TYPE		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 14)

#define  V4L2_CID_AE_WIN_X1		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 15)
#define  V4L2_CID_AE_WIN_Y1		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 16)
#define  V4L2_CID_AE_WIN_X2		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 17)
#define  V4L2_CID_AE_WIN_Y2		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 18)

#define  V4L2_CID_AF_WIN_X1		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 19)
#define  V4L2_CID_AF_WIN_Y1		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 20)
#define  V4L2_CID_AF_WIN_X2		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 21)
#define  V4L2_CID_AF_WIN_Y2		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 22)

static const char *const flash_led_mode_v1[] = {
	"Off",
	"Auto",
	"Red Eye",
	NULL,
};

enum v4l2_flash_led_mode_v1{
	V4L2_FLASH_MODE_NONE = 0,
	V4L2_FLASH_MODE_AUTO,
	V4L2_FLASH_MODE_RED_EYE,
};

#define	V4L2_CID_FLASH_LED_MODE_V1		(V4L2_CID_USER_SUNXI_CAMERA_BASE + 23)

/*
 *	PRIVATE IOCTRLS
 */

struct isp_stat_buf {
	void __user *buf;
	__u32 buf_size;
};
struct isp_exif_attribute {
	struct v4l2_fract exposure_time;
	struct v4l2_fract shutter_speed;
	__u32 fnumber;
	__u32 focal_length;
	__s32 exposure_bias;
	__u32 iso_speed;
	__u32 flash_fire;
	__u32 brightness;
	__s32 reserved[16];
};

struct vin_top_clk {
	__u32 clk_rate;
};

struct vin_fps_ds {
	__u32 fps_ds;
};

struct isp_debug_mode {
	__u32 debug_en;
	__u32 debug_sel;
};

struct vin_pattern_config {
	__u32 ptn_en;
	void __user *ptn_addr;
	void __user *drc_tab;
	void __user *gamma_tab;
	void __user *isp_reg;
	__u32 ptn_size;
	__u32 ptn_w;
	__u32 ptn_h;
	__u32 ptn_fmt;
	__u32 ptn_type;
};

struct vin_reset_time {
	__u32 reset_time;
};

struct sensor_standby_status {
	__u32 stby_stat;
};

struct parser_fps_ds {
	__u32 ch0_fps_ds;
	__u32 ch1_fps_ds;
	__u32 ch2_fps_ds;
	__u32 ch3_fps_ds;
};

struct sensor_isp_cfg {
	__u8 isp_wdr_mode;
	__u8 large_image;
};

enum dma_buffer_num {
	BK_MUL_BUFFER = 0,
	BK_ONE_BUFFER,
	BK_TWO_BUFFER,
};

struct csi_ve_online_cfg {
	__u8 ve_online_en;
	enum dma_buffer_num dma_buf_num;
};

struct vipp_shrink_cfg {
	unsigned int left;
	unsigned int top;
};

enum mipi_pix_num {
	MIPI_ONE_PIXEL    = 0x0,
	MIPI_TWO_PIXEL    = 0x1,
};

struct tdm_speeddn_cfg {
	enum mipi_pix_num pix_num;
	unsigned char tdm_speed_down_en;
	unsigned char tdm_tx_valid_num;
	unsigned char tdm_tx_invalid_num;
};

struct isp_ir_awb_gain {
	int awb_rgain_ir;
	int awb_bgain_ir;
};

struct isp_h3a_coor_win {
	int x1;
	int y1;
	int x2;
	int y2;
};

struct isp_ae_roi_attr {
	unsigned short enable;
	unsigned short force_ae_target;
	struct isp_h3a_coor_win coor;
};

struct enc_MovingLevelInfo {
	unsigned char is_overflow;
	unsigned short moving_level_table[484];
};

struct enc_VencVe2IspParam {
	int d2d_level; //[1,1024], 256 means 1X
	int d3d_level; //[1,1024], 256 means 1X
	struct enc_MovingLevelInfo mMovingLevelInfo;
};

struct isp_cfg_attr_data {
	unsigned short cfg_id;
	unsigned short update_flag;
	int ev_digital_gain;
	int pltmwdr_level;
	int denoise_level;
	int tdf_level;
	int ir_status;
	int ae_ev_idx;
	int ae_max_ev_idx;
	int ae_lum_idx;
	int ae_ev_lv;
	int ae_ev_lv_adj;
	int ae_lock;
	int awb_color_temp;
	struct isp_ir_awb_gain awb_ir_gain;
	struct ae_table_info *ae_table;
	char path[100];
	struct isp_ae_roi_attr ae_roi_area;
	unsigned char ae_stat_avg[432];
	struct enc_VencVe2IspParam VencVe2IspParam;
};

struct isp_memremap_cfg {
	unsigned char en;
	void *vir_addr;
	unsigned int size;
};

struct bk_buffer_align {
	unsigned char lbc_align_en;
	unsigned char yuv_align_en;
};

#define VIDIOC_ISP_AE_STAT_REQ \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 1, struct isp_stat_buf)
#define VIDIOC_ISP_HIST_STAT_REQ \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 2, struct isp_stat_buf)
#define VIDIOC_ISP_AF_STAT_REQ \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 3, struct isp_stat_buf)
#define VIDIOC_ISP_EXIF_REQ \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 4, struct isp_exif_attribute)
#define VIDIOC_ISP_GAMMA_REQ \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 5, struct isp_stat_buf)
#define VIDIOC_SET_TOP_CLK \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 6, struct vin_top_clk)
#define VIDIOC_SET_FPS_DS \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 7, struct vin_fps_ds)
#define VIDIOC_HDR_CTRL \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 8, struct isp_hdr_ctrl)
#define VIDIOC_SYNC_CTRL \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 9, struct csi_sync_ctrl)
#define VIDIOC_ISP_DEBUG \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 10, struct isp_debug_mode)
#define VIDIOC_VIN_PTN_CFG \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 11, struct vin_pattern_config)
#define VIDIOC_VIN_RESET_TIME \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 12, struct vin_reset_time)
#define VIDIOC_SET_PARSER_FPS \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 13, struct parser_fps_ds)
#define VIDIOC_SET_STANDBY \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 14, struct sensor_standby_status)
#define VIDIOC_SET_SENSOR_ISP_CFG \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 15, struct sensor_isp_cfg)
#define VIDIOC_SET_VE_ONLINE \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 16, struct csi_ve_online_cfg)
#define VIDIOC_SET_VIPP_SHRINK \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 17, struct vipp_shrink_cfg)
#define VIDIOC_SET_TDM_SPEEDDN_CFG \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 18, struct tdm_speeddn_cfg)
#define VIDIOC_SET_ISP_CFG_ATTR \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 19, struct isp_cfg_attr_data)
#define VIDIOC_GET_ISP_CFG_ATTR \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 20, struct isp_cfg_attr_data)
#define VIDIOC_SET_TDM_DEPTH \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 21, unsigned int)
#define VIDIOC_GET_ISP_ENCPP_CFG_ATTR \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 22, struct isp_encpp_cfg_attr_data)
#define VIDIOC_SET_PHY2VIR \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 23, struct isp_memremap_cfg)
#define VIDIOC_SET_D3DLBCRATIO \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 24, unsigned int)
#define VIDIOC_SET_BKBUFFER_ALIGN \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 25, struct bk_buffer_align)
#define VIDIOC_SET_BK_SET_WSTRIDE \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 26, unsigned char)
/*
 * Events
 *
 * V4L2_EVENT_VIN_H3A: Histogram and AWB AE AF statistics data ready
 * V4L2_EVENT_VIN_ISP_OFF: ISP stream off
 */

#define V4L2_EVENT_VIN_CLASS		(V4L2_EVENT_PRIVATE_START | 0x100)
#define V4L2_EVENT_VIN_H3A		(V4L2_EVENT_VIN_CLASS | 0x1)
#define V4L2_EVENT_VIN_HDR		(V4L2_EVENT_VIN_CLASS | 0x2)
#define V4L2_EVENT_VIN_ISP_OFF		(V4L2_EVENT_VIN_CLASS | 0x3)

struct vin_isp_h3a_config {
	__u32 buf_size;
	__u32 config_counter;
};

/**
 * struct vin_isp_stat_data - Statistic data sent to or received from user
 * @ts: Timestamp of returned framestats.
 * @buf: Pointer to pass to user.
 * @frame_number: Frame number of requested stats.
 * @cur_frame: Current frame number being processed.
 * @config_counter: Number of the configuration associated with the data.
 */
struct vin_isp_stat_data {
	void __user *buf;
	__u32 buf_size;
	__u32 frame_number;
	__u32 config_counter;
};

struct vin_isp_stat_event_status {
	__u32 frame_number;
	__u16 config_counter;
	__u8 buf_err;
};

struct vin_isp_hdr_event_data {
	__u32			cmd;
	struct isp_hdr_ctrl	hdr;
};

struct vin_vsync_event_data {
	__u64 frame_number;
};

/*
 * Statistics IOCTLs
 *
 * VIDIOC_VIN_ISP_H3A_CFG: Set AE configuration
 * VIDIOC_VIN_ISP_STAT_REQ: Read statistics (AE/AWB/AF/histogram) data
 * VIDIOC_VIN_ISP_STAT_EN: Enable/disable a statistics module
 */

#define VIDIOC_VIN_ISP_H3A_CFG \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 31, struct vin_isp_h3a_config)
#define VIDIOC_VIN_ISP_STAT_REQ \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 32, struct vin_isp_stat_data)
#define VIDIOC_VIN_ISP_STAT_EN \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 33, unsigned int)

//#define ISP_MSC_TBL_SIZE	484
//#define ISP_MSC_TBL_LENGTH			(3*ISP_MSC_TBL_SIZE)

struct sensor_config {
	int width;
	int height;
	unsigned int hoffset;	/*receive hoffset from sensor output*/
	unsigned int voffset;	/*receive voffset from sensor output*/
	unsigned int hts;	/*h size of timing, unit: pclk      */
	unsigned int vts;	/*v size of timing, unit: line      */
	unsigned int pclk;	/*pixel clock in Hz                 */
	unsigned int fps_fixed;	/*sensor fps            */
	unsigned int bin_factor;/*binning factor                    */
	unsigned int intg_min;	/*integration min, unit: line, Q4   */
	unsigned int intg_max;	/*integration max, unit: line, Q4   */
	unsigned int intg_mid_min;	/*middle integration min, unit: line, Q4   */
	unsigned int intg_mid_max;	/*middle integration max, unit: line, Q4   */
	unsigned int intg_short_min;	/*short integration min, unit: line, Q4   */
	unsigned int intg_short_max;	/*short integration max, unit: line, Q4   */
	unsigned int gain_min;	/*sensor gain min, Q4               */
	unsigned int gain_max;	/*sensor gain max, Q4               */
	unsigned int mbus_code;	/*media bus code                    */
	unsigned int wdr_mode;	/*isp wdr mode                    */
#if 0
	/* otp information*/
	int otp_enable;
	/*msc table  22x22x3 = ISP_MSC_TBL_LENGTH, default mode using 16x16x3*/
	void *pmsc_table;
#endif
};

struct sensor_exp_gain {
	int exp_val;
	int exp_mid_val;
	int exp_short_val;
	int gain_val;
	int gain_mid_val;
	int gain_short_val;
	int r_gain;
	int b_gain;
};

struct sensor_fps {
	int fps;
};

struct sensor_flip {
	unsigned char hflip;
	unsigned char vflip;
};

struct sensor_temp {
	int temp;
};

struct isp_table_reg_map {
	void __user *addr;
	unsigned int size;
};

struct isp_debug_info {
	int exp_val;
	int gain_val;
	unsigned int lum_idx;
	unsigned int awb_color_temp;
	unsigned int awb_rgain;
	unsigned int awb_bgain;
	int contrast_level;
	int brightness_level;
	int sharpness_level;
	int saturation_level;
	int tdf_level;
	int denoise_level;
	int pltmwdr_level;
	int sensor_temper;
	char libs_version[64];
	char isp_cfg_version[128];
};

typedef enum _switch_choice_type {
	SWITCH_A = 0,
	SWITCH_B = 1,
	SWITCH_MAX,
} switch_choice_type;

typedef enum _switch_ctrl_type {
	GET_SWITCH = 0,
	SET_SWITCH = 1,
	CTRL_MAX,
} switch_ctrl_type;

struct sensor_mipi_switch_entity {
	switch_ctrl_type switch_ctrl;
	unsigned int mipi_switch_status;
};

struct actuator_ctrl {
	unsigned int code;
};

struct actuator_para {
	unsigned short code_min;
	unsigned short code_max;
};

struct flash_para {
	unsigned int mode; //enum v4l2_flash_led_mode mode;
};

struct msc_para {
	unsigned char data[2048];
};

struct ir_switch {
	int ir_on;
	int ir_flash_on;
	int ir_hold;
};

struct sensor_resolution {
	__u32 width_max;
	__u32 height_max;
	__u32 width_min;
	__u32 height_min;
};

/*
 * Camera Sensor IOCTLs
 */

#define VIDIOC_VIN_SENSOR_CFG_REQ \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 60, struct sensor_config)

#define VIDIOC_VIN_SENSOR_EXP_GAIN \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 61, struct sensor_exp_gain)
#define VIDIOC_VIN_SENSOR_SET_FPS \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 62, struct sensor_fps)
#define VIDIOC_VIN_SENSOR_GET_TEMP \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 63, struct sensor_temp)

#define VIDIOC_VIN_ACT_SET_CODE \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 64, struct actuator_ctrl)
#define VIDIOC_VIN_ACT_INIT \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 65, struct actuator_para)
#define VIDIOC_VIN_FLASH_EN \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 66, struct flash_para)
#define VIDIOC_VIN_SET_IR \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 67, struct ir_switch)

#define VIDIOC_VIN_ISP_LOAD_REG \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 70, struct isp_table_reg_map)

#define VIDIOC_VIN_ISP_TABLE1_MAP \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 71, struct isp_table_reg_map)

#define VIDIOC_VIN_ISP_TABLE2_MAP \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 72, struct isp_table_reg_map)

#define VIDIOC_VIN_GET_SENSOR_CODE \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 73, int)

#define VIDIOC_VIN_GET_SENSOR_OTP_INFO \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 74, struct msc_para)

#define VIDIOC_VIN_SET_SENSOR_OTP_INFO \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 75, unsigned long long)
#define VIDIOC_VIN_ISP_SYNC_DEBUG_INFO \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 76, struct isp_debug_info)
#define VIDIOC_VIN_SENSOR_GET_FPS \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 77, struct sensor_fps)
#define VIDIOC_VIN_SENSOR_GET_FLIP \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 78, struct sensor_flip)
#define VIDIOC_VIN_SENSOR_MIPI_SWITCH \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 79, struct sensor_mipi_switch_entity)

#endif /*_SUNXI_CAMERA_H_*/

