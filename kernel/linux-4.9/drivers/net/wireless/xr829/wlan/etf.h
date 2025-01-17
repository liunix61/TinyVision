/*
 * Common interfaces for XRadio drivers
 *
 * Copyright (c) 2013
 * Xradio Technology Co., Ltd. <www.xradiotech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef XRADIO_ETF_H
#define XRADIO_ETF_H
#include <linux/version.h>

/* ETF Adapter layer */
#define  NL_FOR_XRADIO  30
/*
 * When queue mode is enable, user space operate is non-block if items in rx_pool,
 * but it may sleep and will drop msg when rx_pool is empty for a long time.
 * When queue mode is disable, user space operate is block and user packet
 * will not be drop, but driver can be panic if etf cli be kill during operate.
 * So, for more safe in driver, enable this by default.
 */
#define  ETF_QUEUEMODE  1
#define  ETF_QUEUE_TIMEOUT 50 /* about 2s.*/

#define  ADAPTER_RX_BUF_LEN  (528*3) /* pay respect for SDIO_BLOCK_SIZE*/
#if (ETF_QUEUEMODE)
#define  ADAPTER_ITEM_MAX    (32)
#define  XRADIO_ADAPTER  ("xradio_etf")
struct  adapter_item {
	struct list_head head;
	void   *skb;
};
#endif

typedef int (*msg_proc)(void *data, int len);
struct xradio_adapter {
	struct sock *sock;
	msg_proc  handler;
	void     *msg_buf;
	int       msg_len;
#if (ETF_QUEUEMODE)
	struct adapter_item rx_items[ADAPTER_ITEM_MAX];
	struct list_head rx_queue;
	struct list_head rx_pool;
	spinlock_t  recv_lock;
	atomic_t    rx_cnt;
	struct semaphore proc_sem;
	struct task_struct *thread_tsk;
	int    exit;
#endif
	int       send_pid;
	struct    semaphore send_lock;
};

/* ETF internal connect state*/
#define ETF_STAT_NULL        0
#define ETF_STAT_CONNECTING  1
#define ETF_STAT_CONNECTED   2

#define ADAPTER_MAIN_VER 1
#define ADAPTER_SUB_VER  1
#define ADAPTER_REV_VER  2

#define REF_ETF_MAIN_VER 2
#define REF_ETF_SUB_VER  0
#define REF_ETF_REV_VER  5

struct xradio_etf {
	int    etf_state;
	int    is_wlan;
	void  *core_priv;
	struct semaphore etf_lock;
	struct xradio_adapter *adapter;
	int    seq_send;
	int    seq_recv;
	const u8    *fw_path;
	const u8    *sdd_path;
	u8 *cli_data;
	u8 cli_data_len;
	u32 version;
};

/* ETF fw cmd defines*/
#define  ETF_REQ_BASE               0x0000
#define  ETF_CNF_BASE               0x0400
#define  ETF_IND_BASE               0x0800
#define  ETF_MSG_STARTUP_IND_ID     0x0801
#define  ETF_DOWNLOAD_SDD          (0x20 + 22)
#define  ETF_HWT_REQ                0x0004
struct etf_sdd_req {
	u16    len;
	u16    id;
	u32    sdd_cmd;
};

typedef enum {
    ETF_CHANNEL_BANDWIDTH_20MHz,
    ETF_CHANNEL_BANDWIDTH_10MHz,
    ETF_CHANNEL_BANDWIDTH_40MHz
} ETF_CHANNEL_BANDWIDTH_T;

typedef enum {
    ETF_SUB_CHANNEL_UPPER,
    ETF_SUB_CHANNEL_LOWER
} ETF_SUB_CHANNEL_T;

typedef struct ETF_CONNECT_REQ{
	u16 MsgLen;
	u16 MsgId;
	u32 version;
	u32 data_len;
} ETF_CONNECT_REQ_T;

typedef struct CLI_PARAM_SAVE_REQ {
	u16 Msglen;
	u16 MsgID;
	u32 result;
	u32 data_len;
	u32 version;
	u8  data[1];
} CLI_PARAM_SAVE_T;

/* ETF driver cmd defines */
#define  ETF_DRIVER_CMD_START_ID  (ETF_REQ_BASE + 0x03F0)
#define  ETF_GET_API_CONTEXT_ID   (ETF_REQ_BASE + 0x03F8)
#define  ETF_GET_SDD_PARAM_ID     (ETF_REQ_BASE + 0x03F9)
#define  ETF_CONNECT_ID           (ETF_REQ_BASE + 0x03FA)
#define  ETF_DISCONNECT_ID        (ETF_REQ_BASE + 0x03FB)
#define  ETF_RECONNECT_ID         (ETF_REQ_BASE + 0x03FC)
#define  ETF_DOWNLOAD_SDD_ID      (ETF_REQ_BASE + 0x03FD)
#define  ETF_DOWNLOAD_ID          (ETF_REQ_BASE + 0x03FE)
#define  ETF_SOFT_RESET_REQ_ID    (ETF_REQ_BASE + 0x03FF)
#define  ETF_DRIVER_IND_ID        (ETF_IND_BASE + 0x03FF)
#define  ETF_GET_SDD_POWER_DEFT   (ETF_REQ_BASE + 0x1FFF)
#define  ETF_SET_CLI_PAR_DEFT     (ETF_REQ_BASE + 0x03E2)
#define  ETF_GET_CLI_PAR_DEFT     (ETF_REQ_BASE + 0x03E1)
#define  ETF_GET_SDD_PATH_ID      (ETF_REQ_BASE + 0x03E3)

#define  FLAG_GET_SDD_ALL         0x1

typedef struct {
    u16 MsgLen;
    u16 MsgId;
    u32 result;
} ETF_PARAM0;

struct get_sdd_patch_req {
	u16    len;
	u16    id;
	u32    result;
};
struct get_sdd_patch_ret {
	u16    len;
	u16    id;
	u32    result;
	u32    length;
	/* sdd patch follow */
};
struct get_sdd_param_req {
	u16    len;
	u16    id;
	u8     flags;
	u8     ies;
};
struct get_sdd_result {
	u16    len;
	u16    id;
	u32    result;
	u32    length;
	/* sdd data follow */
};

struct get_cli_data_req {
	u16    len;
	u16    id;
	u32    result;
	u32    version;
};

struct get_cli_data_s {
	u16          msg_len;
	u16          id;
	u32          result;
	u32          data_len;
	u32          version;
};

struct etf_api_context_req {
	u16    len;
	u16    id;
	u32    param;
};

#define HI_SW_LABEL_MAX	     128
struct etf_api_context_result {
	u16    len;
	u16    id;
	u32    result;
	u8     is_etf_fw_run;
	u8     reversed[3];
	u32    mib_baseaddr;
	u8     fw_label[HI_SW_LABEL_MAX];
	u16    fw_api_ver;
};

#define ETF_CONTEXT_OFFSET  (20 + HI_SW_LABEL_MAX)
#define HI_MAX_CONFIG_TABLES	4
struct etf_api_context {
	u32    config[HI_MAX_CONFIG_TABLES];
};

/* boot State, for download file from GUI */
#define BOOT_STATE_NULL              0
#define BOOT_WAITING_DOWNLOAD        1
#define BOOT_IN_PROGRESS             2
#define BOOT_COMPLETE                3
#define BOOT_SDD_COMPLETE            4

/* Driver Result code */
#define BOOT_SUCCESS                 (0)
#define BOOT_ERR_DECODE              (1)
#define BOOT_ERR_CHECKSUM            (2)
#define BOOT_ERR_FILE_SIZE           (3)
#define BOOT_ERR_BAD_OP              (4)
#define ETF_ERR_CONNECTED            (5)
#define ETF_ERR_WLAN_MODE            (6)
#define ETF_ERR_NOT_CONNECT          (7)
#define ETF_ERR_IO_FAILED            (8)
#define ETF_ERR_DRIVER_HANG          (9)
#define ETF_ERR_CHECK_VERSION        (10)

struct drv_resp {
	u16    len;
	u16    id;
	u32    state;
	u32    result;
};

#define  MSG_ID(x)     ((u16)((x)&0x1fff))
#define  MSG_SEQ(x)    (((x)>>13)&0x0007)
#define  SEQ_MASK(x)   ((x) & 0x0007)

#define DOWNLOAD_F_START          0x01
#define DOWNLOAD_F_END            0x02
#define DOWNLOAD_F_PATH_ONLY      0x04
struct drv_download {
	u32   offset;
	u32   flags;
	/* data followed, max size is HI_MEM_BLK_BYTES */
};

/* ETF interfaces called by WLAN core */
int xradio_etf_init(void);
void xradio_etf_deinit(void);
const char *etf_get_fwpath(void);
const char *etf_get_sddpath(void);
void etf_set_fwpath(const char *fw_path);
void etf_set_sddpath(const char *sdd_path);
bool etf_is_connect(void);
void etf_set_core(void *core_priv);
int xradio_etf_from_device(struct sk_buff **skb);
void xradio_etf_save_context(void *buf, int len);
int xradio_etf_suspend(void);
int xradio_etf_resume(void);

/* External interfaces called by etf */
extern int  xradio_core_init(void);
extern void xradio_core_deinit(void);

#endif /*XRADIO_ETF_H*/
