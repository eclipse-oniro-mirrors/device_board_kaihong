/*
 * Custom OID/ioctl definitions for
 *
 *
 * Broadcom 802.11abg Networking Device Driver
 *
 * Definitions subject to change without notice.
 *
 * Copyright (C) 1999-2019, Broadcom.
 *
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 *
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 *
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wlioctl.h 824900 2019-06-12 05:42:13Z $
 */

#ifndef _wlioctl_h_
#define	_wlioctl_h_

#include <typedefs.h>
#include <ethernet.h>
#include <bcmip.h>
#include <bcmeth.h>
#include <bcmip.h>
#include <bcmipv6.h>
#include <bcmevent.h>
#include <802.11.h>
#include <802.11s.h>
#include <802.1d.h>
#include <bcmwifi_channels.h>
#ifdef WL11AX
#include <802.11ax.h>
#endif /* WL11AX */
#include <bcmwifi_rates.h>
#include <wlioctl_defs.h>
#include <bcmipv6.h>

#include <bcm_mpool_pub.h>
#include <bcmcdc.h>
#define SSSR_NEW_API

/* Include bcmerror.h for error codes or aliases */
#ifdef BCMUTILS_ERR_CODES
#include <bcmerror.h>
#endif	/* BCMUTILS_ERR_CODES */

/* NOTE re: Module specific error codes.
 *
 * BCME_.. error codes are extended by various features - e.g. FTM, NAN, SAE etc.
 * The current process is to allocate a range of 1024 negative 32 bit integers to
 * each module that extends the error codes to indicate a module specific status.
 *
 * The next range to use is below. If that range is used for a new feature, please
 * update the range to be used by the next feature.
 *
 * The error codes -4096 ... -5119 are reserved for firmware signing.
 *
 * Next available (inclusive) range: [-8*1024 + 1, -7*1024]
 *
 * End Note
 */

/* 11ax trigger frame format - versioning info */
#define TRIG_FRAME_FORMAT_11AX_DRAFT_1P1 0

typedef struct {
	uint32 num;
	chanspec_t list[1];
} chanspec_list_t;

#define RSN_KCK_LENGTH	16
#define RSN_KEK_LENGTH	16
#define TPK_FTM_LEN		16
#ifndef INTF_NAME_SIZ
#define INTF_NAME_SIZ	16
#endif // endif

#define WL_ASSOC_START_EVT_DATA_VERSION      1

typedef struct assoc_event_data {
	uint32 version;
	uint32 flags;
	chanspec_t join_chspec;
} assoc_event_data_t;

/**Used to send ioctls over the transport pipe */
typedef struct remote_ioctl {
	cdc_ioctl_t	msg;
	uint32		data_len;
	char           intf_name[INTF_NAME_SIZ];
} rem_ioctl_t;
#define REMOTE_SIZE	sizeof(rem_ioctl_t)

#define BCM_IOV_XTLV_VERSION 0

#define MAX_NUM_D11CORES 2

/**DFS Forced param */
typedef struct wl_dfs_forced_params {
	chanspec_t chspec;
	uint16 version;
	chanspec_list_t chspec_list;
} wl_dfs_forced_t;

#define DFS_PREFCHANLIST_VER 0x01
#define WL_CHSPEC_LIST_FIXED_SIZE	OFFSETOF(chanspec_list_t, list)
/* size of dfs forced param size given n channels are in the list */
#define WL_DFS_FORCED_PARAMS_SIZE(n) \
	(sizeof(wl_dfs_forced_t) + (((n) < 1) ? (0) : (((n) - 1)* sizeof(chanspec_t))))
#define WL_DFS_FORCED_PARAMS_FIXED_SIZE \
	(WL_CHSPEC_LIST_FIXED_SIZE + OFFSETOF(wl_dfs_forced_t, chspec_list))
#define WL_DFS_FORCED_PARAMS_MAX_SIZE \
	WL_DFS_FORCED_PARAMS_FIXED_SIZE + (WL_NUMCHANNELS * sizeof(chanspec_t))

/**association decision information */
typedef struct {
	uint8		assoc_approved;		/**< (re)association approved */
	uint8		pad;
	uint16		reject_reason;		/**< reason code for rejecting association */
	struct		ether_addr   da;
	uint8		pad1[6];
	int64		sys_time;		/**< current system time */
} assoc_decision_t;

#define DFS_SCAN_S_IDLE		-1
#define DFS_SCAN_S_RADAR_FREE 0
#define DFS_SCAN_S_RADAR_FOUND 1
#define DFS_SCAN_S_INPROGESS 2
#define DFS_SCAN_S_SCAN_ABORTED 3
#define DFS_SCAN_S_SCAN_MODESW_INPROGRESS 4
#define DFS_SCAN_S_MAX 5

#define ACTION_FRAME_SIZE 1800

typedef struct wl_action_frame {
	struct ether_addr 	da;
	uint16 			len;
	uint32 			packetId;
	uint8			data[ACTION_FRAME_SIZE];
} wl_action_frame_t;

#define WL_WIFI_ACTION_FRAME_SIZE sizeof(struct wl_action_frame)

typedef struct ssid_info
{
	uint8		ssid_len;	/**< the length of SSID */
	uint8		ssid[32];	/**< SSID string */
} ssid_info_t;

typedef struct wl_af_params {
	uint32			channel;
	int32			dwell_time;
	struct ether_addr	BSSID;
	uint8 PAD[2];
	wl_action_frame_t	action_frame;
} wl_af_params_t;

#define WL_WIFI_AF_PARAMS_SIZE sizeof(struct wl_af_params)

#define MFP_TEST_FLAG_NORMAL	0
#define MFP_TEST_FLAG_ANY_KEY	1
typedef struct wl_sa_query {
	uint32 flag;
	uint8  action;
	uint8  PAD;
	uint16 id;
	struct ether_addr da;
	uint16  PAD;
} wl_sa_query_t;

/* EXT_STA */
/**association information */
typedef struct {
	uint32		assoc_req;	/**< offset to association request frame */
	uint32		assoc_req_len;	/**< association request frame length */
	uint32		assoc_rsp;	/**< offset to association response frame */
	uint32		assoc_rsp_len;	/**< association response frame length */
	uint32		bcn;		/**< offset to AP beacon */
	uint32		bcn_len;	/**< AP beacon length */
	uint32		wsec;		/**< ucast security algo */
	uint32		wpaie;		/**< offset to WPA ie */
	uint8		auth_alg;	/**< 802.11 authentication mode */
	uint8		WPA_auth;	/**< WPA: authenticated key management */
	uint8		ewc_cap;	/**< EWC (MIMO) capable */
	uint8		ofdm;		/**< OFDM */
} assoc_info_t;
/* defined(EXT_STA) */

/* Flags for OBSS IOVAR Parameters */
#define WL_OBSS_DYN_BWSW_FLAG_ACTIVITY_PERIOD        (0x01)
#define WL_OBSS_DYN_BWSW_FLAG_NOACTIVITY_PERIOD      (0x02)
#define WL_OBSS_DYN_BWSW_FLAG_NOACTIVITY_INCR_PERIOD (0x04)
#define WL_OBSS_DYN_BWSW_FLAG_PSEUDO_SENSE_PERIOD    (0x08)
#define WL_OBSS_DYN_BWSW_FLAG_RX_CRS_PERIOD          (0x10)
#define WL_OBSS_DYN_BWSW_FLAG_DUR_THRESHOLD          (0x20)
#define WL_OBSS_DYN_BWSW_FLAG_TXOP_PERIOD            (0x40)

/* OBSS IOVAR Version information */
#define WL_PROT_OBSS_CONFIG_PARAMS_VERSION 1

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint8 obss_bwsw_activity_cfm_count_cfg; /**< configurable count in
		* seconds before we confirm that OBSS is present and
		* dynamically activate dynamic bwswitch.
		*/
	uint8 obss_bwsw_no_activity_cfm_count_cfg; /**< configurable count in
		* seconds before we confirm that OBSS is GONE and
		* dynamically start pseudo upgrade. If in pseudo sense time, we
		* will see OBSS, [means that, we false detected that OBSS-is-gone
		* in watchdog] this count will be incremented in steps of
		* obss_bwsw_no_activity_cfm_count_incr_cfg for confirming OBSS
		* detection again. Note that, at present, max 30seconds is
		* allowed like this. [OBSS_BWSW_NO_ACTIVITY_MAX_INCR_DEFAULT]
		*/
	uint8 obss_bwsw_no_activity_cfm_count_incr_cfg; /* see above
		*/
	uint16 obss_bwsw_pseudo_sense_count_cfg; /**< number of msecs/cnt to be in
		* pseudo state. This is used to sense/measure the stats from lq.
		*/
	uint8 obss_bwsw_rx_crs_threshold_cfg; /**< RX CRS default threshold */
	uint8 obss_bwsw_dur_thres; /**< OBSS dyn bwsw trigger/RX CRS Sec */
	uint8 obss_bwsw_txop_threshold_cfg; /**< TXOP default threshold */
} BWL_POST_PACKED_STRUCT wlc_obss_dynbwsw_config_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint32 version;	/**< version field */
	uint32 config_mask;
	uint32 reset_mask;
	wlc_obss_dynbwsw_config_t config_params;
} BWL_POST_PACKED_STRUCT obss_config_params_t;
#include <packed_section_end.h>

/**bsscfg type */
typedef enum bsscfg_type {
	BSSCFG_TYPE_GENERIC = 0,	/**< Generic AP/STA/IBSS BSS */
	BSSCFG_TYPE_P2P = 1,		/**< P2P BSS */
	/* index 2 earlier used for BTAMP */
	BSSCFG_TYPE_PSTA = 3,
	BSSCFG_TYPE_TDLS = 4,
	BSSCFG_TYPE_SLOTTED_BSS = 5,
	BSSCFG_TYPE_PROXD = 6,
	BSSCFG_TYPE_NAN = 7,
	BSSCFG_TYPE_MESH = 8,
	BSSCFG_TYPE_AIBSS = 9
} bsscfg_type_t;

/* bsscfg subtype */
typedef enum bsscfg_subtype {
	BSSCFG_SUBTYPE_NONE = 0,
	BSSCFG_GENERIC_STA = 1,		/* GENERIC */
	BSSCFG_GENERIC_AP = 2,
	BSSCFG_GENERIC_IBSS = 6,
	BSSCFG_P2P_GC = 3,		/* P2P */
	BSSCFG_P2P_GO = 4,
	BSSCFG_P2P_DISC = 5,
	/* Index 7 & 8 earlier used for BTAMP */
	BSSCFG_SUBTYPE_AWDL = 9, /* SLOTTED_BSS_TYPE */
	BSSCFG_SUBTYPE_NAN_MGMT = 10,
	BSSCFG_SUBTYPE_NAN_DATA = 11,
	BSSCFG_SUBTYPE_NAN_MGMT_DATA = 12
} bsscfg_subtype_t;

typedef struct wlc_bsscfg_info {
	uint32 type;
	uint32 subtype;
} wlc_bsscfg_info_t;

/* ULP SHM Offsets info */
typedef struct ulp_shm_info {
	uint32 m_ulp_ctrl_sdio;
	uint32 m_ulp_wakeevt_ind;
	uint32 m_ulp_wakeind;
} ulp_shm_info_t;

/* Legacy structure to help keep backward compatible wl tool and tray app */

#define	LEGACY_WL_BSS_INFO_VERSION	107	/**< older version of wl_bss_info struct */

typedef struct wl_bss_info_107 {
	uint32		version;		/**< version field */
	uint32		length;			/**< byte length of data in this record,
						 * starting at version and including IEs
						 */
	struct ether_addr BSSID;
	uint16		beacon_period;		/**< units are Kusec */
	uint16		capability;		/**< Capability information */
	uint8		SSID_len;
	uint8		SSID[32];
	uint8		PAD;
	struct {
		uint32	count;			/**< # rates in this set */
		uint8	rates[16];		/**< rates in 500kbps units w/hi bit set if basic */
	} rateset;				/**< supported rates */
	uint8		channel;		/**< Channel no. */
	uint8		PAD;
	uint16		atim_window;		/**< units are Kusec */
	uint8		dtim_period;		/**< DTIM period */
	uint8		PAD;
	int16		RSSI;			/**< receive signal strength (in dBm) */
	int8		phy_noise;		/**< noise (in dBm) */
	uint8		PAD[3];
	uint32		ie_length;		/**< byte length of Information Elements */
	/* variable length Information Elements */
} wl_bss_info_107_t;

/*
 * Per-BSS information structure.
 */

#define	LEGACY2_WL_BSS_INFO_VERSION	108		/**< old version of wl_bss_info struct */

/**
 * BSS info structure
 * Applications MUST CHECK ie_offset field and length field to access IEs and
 * next bss_info structure in a vector (in wl_scan_results_t)
 */
typedef struct wl_bss_info_108 {
	uint32		version;		/**< version field */
	uint32		length;			/**< byte length of data in this record,
						 * starting at version and including IEs
						 */
	struct ether_addr BSSID;
	uint16		beacon_period;		/**< units are Kusec */
	uint16		capability;		/**< Capability information */
	uint8		SSID_len;
	uint8		SSID[32];
	uint8		PAD[1];
	struct {
		uint32	count;			/**< # rates in this set */
		uint8	rates[16];		/**< rates in 500kbps units w/hi bit set if basic */
	} rateset;				/**< supported rates */
	chanspec_t	chanspec;		/**< chanspec for bss */
	uint16		atim_window;		/**< units are Kusec */
	uint8		dtim_period;		/**< DTIM period */
	uint8		PAD;
	int16		RSSI;			/**< receive signal strength (in dBm) */
	int8		phy_noise;		/**< noise (in dBm) */

	uint8		n_cap;			/**< BSS is 802.11N Capable */
	uint8		PAD[2];
	uint32		nbss_cap;		/**< 802.11N BSS Capabilities (based on HT_CAP_*) */
	uint8		ctl_ch;			/**< 802.11N BSS control channel number */
	uint8		PAD[3];
	uint32		reserved32[1];		/**< Reserved for expansion of BSS properties */
	uint8		flags;			/**< flags */
	uint8		reserved[3];		/**< Reserved for expansion of BSS properties */
	uint8		basic_mcs[MCSSET_LEN];	/**< 802.11N BSS required MCS set */

	uint16		ie_offset;		/**< offset at which IEs start, from beginning */
	uint8		PAD[2];
	uint32		ie_length;		/**< byte length of Information Elements */
	/* Add new fields here */
	/* variable length Information Elements */
} wl_bss_info_108_t;

#define	WL_BSS_INFO_VERSION	109		/**< current version of wl_bss_info struct */

/**
 * BSS info structure
 * Applications MUST CHECK ie_offset field and length field to access IEs and
 * next bss_info structure in a vector (in wl_scan_results_t)
 */
typedef struct wl_bss_info {
	uint32		version;		/**< version field */
	uint32		length;			/**< byte length of data in this record,
						 * starting at version and including IEs
						 */
	struct ether_addr BSSID;
	uint16		beacon_period;		/**< units are Kusec */
	uint16		capability;		/**< Capability information */
	uint8		SSID_len;
	uint8		SSID[32];
	uint8		bcnflags;		/* additional flags w.r.t. beacon */
	struct {
		uint32	count;			/**< # rates in this set */
		uint8	rates[16];		/**< rates in 500kbps units w/hi bit set if basic */
	} rateset;				/**< supported rates */
	chanspec_t	chanspec;		/**< chanspec for bss */
	uint16		atim_window;		/**< units are Kusec */
	uint8		dtim_period;		/**< DTIM period */
	uint8		accessnet;		/* from beacon interwork IE (if bcnflags) */
	int16		RSSI;			/**< receive signal strength (in dBm) */
	int8		phy_noise;		/**< noise (in dBm) */
	uint8		n_cap;			/**< BSS is 802.11N Capable */
	uint16		freespace1;		/* make implicit padding explicit */
	uint32		nbss_cap;		/**< 802.11N+AC BSS Capabilities */
	uint8		ctl_ch;			/**< 802.11N BSS control channel number */
	uint8		padding1[3];		/**< explicit struct alignment padding */
	uint16		vht_rxmcsmap;	/**< VHT rx mcs map (802.11ac IE, VHT_CAP_MCS_MAP_*) */
	uint16		vht_txmcsmap;	/**< VHT tx mcs map (802.11ac IE, VHT_CAP_MCS_MAP_*) */
	uint8		flags;			/**< flags */
	uint8		vht_cap;		/**< BSS is vht capable */
	uint8		reserved[2];		/**< Reserved for expansion of BSS properties */
	uint8		basic_mcs[MCSSET_LEN];	/**< 802.11N BSS required MCS set */

	uint16		ie_offset;		/**< offset at which IEs start, from beginning */
	uint16		freespace2;		/* making implicit padding explicit */
	uint32		ie_length;		/**< byte length of Information Elements */
	int16		SNR;			/**< average SNR of during frame reception */
	uint16		vht_mcsmap;		/**< STA's Associated vhtmcsmap */
	uint16		vht_mcsmap_prop;	/**< STA's Associated prop vhtmcsmap */
	uint16		vht_txmcsmap_prop;	/**< prop VHT tx mcs prop */
} wl_bss_info_v109_t;

/**
 * BSS info structure
 * Applications MUST CHECK ie_offset field and length field to access IEs and
 * next bss_info structure in a vector (in wl_scan_results_t)
 */
typedef struct wl_bss_info_v109_1 {
	uint32		version;		/**< version field */
	uint32		length;			/**< byte length of data in this record,
						 * starting at version and including IEs
						 */
	struct ether_addr BSSID;
	uint16		beacon_period;		/**< units are Kusec */
	uint16		capability;		/**< Capability information */
	uint8		SSID_len;
	uint8		SSID[32];
	uint8		bcnflags;		/* additional flags w.r.t. beacon */
	struct {
		uint32	count;			/**< # rates in this set */
		uint8	rates[16];		/**< rates in 500kbps units w/hi bit set if basic */
	} rateset;				/**< supported rates */
	chanspec_t	chanspec;		/**< chanspec for bss */
	uint16		atim_window;		/**< units are Kusec */
	uint8		dtim_period;		/**< DTIM period */
	uint8		accessnet;		/* from beacon interwork IE (if bcnflags) */
	int16		RSSI;			/**< receive signal strength (in dBm) */
	int8		phy_noise;		/**< noise (in dBm) */
	uint8		n_cap;			/**< BSS is 802.11N Capable */
	uint8		he_cap;			/**< BSS is he capable */
	uint8		freespace1;		/* make implicit padding explicit */
	uint32		nbss_cap;		/**< 802.11N+AC BSS Capabilities */
	uint8		ctl_ch;			/**< 802.11N BSS control channel number */
	uint8		padding1[3];		/**< explicit struct alignment padding */
	uint16		vht_rxmcsmap;	/**< VHT rx mcs map (802.11ac IE, VHT_CAP_MCS_MAP_*) */
	uint16		vht_txmcsmap;	/**< VHT tx mcs map (802.11ac IE, VHT_CAP_MCS_MAP_*) */
	uint8		flags;			/**< flags */
	uint8		vht_cap;		/**< BSS is vht capable */
	uint8		reserved[2];		/**< Reserved for expansion of BSS properties */
	uint8		basic_mcs[MCSSET_LEN];	/**< 802.11N BSS required MCS set */

	uint16		ie_offset;		/**< offset at which IEs start, from beginning */
	uint16		freespace2;		/* making implicit padding explicit */
	uint32		ie_length;		/**< byte length of Information Elements */
	int16		SNR;			/**< average SNR of during frame reception */
	uint16		vht_mcsmap;		/**< STA's Associated vhtmcsmap */
	uint16		vht_mcsmap_prop;	/**< STA's Associated prop vhtmcsmap */
	uint16		vht_txmcsmap_prop;	/**< prop VHT tx mcs prop */
	uint32		he_mcsmap;	/**< STA's Associated hemcsmap */
	uint32		he_rxmcsmap;	/**< HE rx mcs map (802.11ax IE, HE_CAP_MCS_MAP_*) */
	uint32		he_txmcsmap;	/**< HE tx mcs map (802.11ax IE, HE_CAP_MCS_MAP_*) */
} wl_bss_info_v109_1_t;

/**
 * BSS info structure
 * Applications MUST CHECK ie_offset field and length field to access IEs and
 * next bss_info structure in a vector (in wl_scan_results_t)
 */
typedef struct wl_bss_info_v109_2 {
	uint32		version;		/**< version field */
	uint32		length;			/**< byte length of data in this record,
						 * starting at version and including IEs
						 */
	struct ether_addr BSSID;
	uint16		beacon_period;		/**< units are Kusec */
	uint16		capability;		/**< Capability information */
	uint8		SSID_len;
	uint8		SSID[32];
	uint8		bcnflags;		/* additional flags w.r.t. beacon */
	struct {
		uint32	count;			/**< # rates in this set */
		uint8	rates[16];		/**< rates in 500kbps units w/hi bit set if basic */
	} rateset;				/**< supported rates */
	chanspec_t	chanspec;		/**< chanspec for bss */
	uint16		atim_window;		/**< units are Kusec */
	uint8		dtim_period;		/**< DTIM period */
	uint8		accessnet;		/* from beacon interwork IE (if bcnflags) */
	int16		RSSI;			/**< receive signal strength (in dBm) */
	int8		phy_noise;		/**< noise (in dBm) */
	uint8		n_cap;			/**< BSS is 802.11N Capable */
	uint8		he_cap;			/**< BSS is he capable */
	uint8		freespace1;		/* make implicit padding explicit */
	uint32		nbss_cap;		/**< 802.11N+AC BSS Capabilities */
	uint8		ctl_ch;			/**< 802.11N BSS control channel number */
	uint8		padding1[3];		/**< explicit struct alignment padding */
	uint16		vht_rxmcsmap;	/**< VHT rx mcs map (802.11ac IE, VHT_CAP_MCS_MAP_*) */
	uint16		vht_txmcsmap;	/**< VHT tx mcs map (802.11ac IE, VHT_CAP_MCS_MAP_*) */
	uint8		flags;			/**< flags */
	uint8		vht_cap;		/**< BSS is vht capable */
	uint8		reserved[2];		/**< Reserved for expansion of BSS properties */
	uint8		basic_mcs[MCSSET_LEN];	/**< 802.11N BSS required MCS set */

	uint16		ie_offset;		/**< offset at which IEs start, from beginning */
	uint16		freespace2;		/* making implicit padding explicit */
	uint32		ie_length;		/**< byte length of Information Elements */
	int16		SNR;			/**< average SNR of during frame reception */
	uint16		vht_mcsmap;		/**< STA's Associated vhtmcsmap */
	uint16		vht_mcsmap_prop;	/**< STA's Associated prop vhtmcsmap */
	uint16		vht_txmcsmap_prop;	/**< prop VHT tx mcs prop */
	uint32		he_mcsmap;	/**< STA's Associated hemcsmap */
	uint32		he_rxmcsmap;	/**< HE rx mcs map (802.11ax IE, HE_CAP_MCS_MAP_*) */
	uint32		he_txmcsmap;	/**< HE tx mcs map (802.11ax IE, HE_CAP_MCS_MAP_*) */
	uint32		timestamp[2];  /* Beacon Timestamp for FAKEAP req */
} wl_bss_info_v109_2_t;

#ifndef WL_BSS_INFO_TYPEDEF_HAS_ALIAS
typedef wl_bss_info_v109_t wl_bss_info_t;
#endif // endif

#define WL_GSCAN_FULL_RESULT_VERSION	2	/* current version of wl_gscan_result_t struct */

typedef struct wl_gscan_bss_info {
	uint32      timestamp[2];
	wl_bss_info_v109_t info;
	/* Do not add any more members below, fixed  */
	/* and variable length Information Elements to follow */
} wl_gscan_bss_info_v2_t;

typedef struct wl_gscan_bss_info_v3 {
	uint32      timestamp[2];
	uint8 info[];	/* var length wl_bss_info_X structures */
	/* Do not add any more members below, fixed  */
	/* and variable length Information Elements to follow */
} wl_gscan_bss_info_v3_t;

#ifndef WL_BSS_INFO_TYPEDEF_HAS_ALIAS
typedef wl_gscan_bss_info_v2_t wl_gscan_bss_info_t;
#define WL_GSCAN_INFO_FIXED_FIELD_SIZE   (sizeof(wl_gscan_bss_info_t) - sizeof(wl_bss_info_t))
#endif // endif

typedef struct wl_bsscfg {
	uint32  bsscfg_idx;
	uint32  wsec;
	uint32  WPA_auth;
	uint32  wsec_index;
	uint32  associated;
	uint32  BSS;
	uint32  phytest_on;
	struct ether_addr   prev_BSSID;
	struct ether_addr   BSSID;
	uint32  targetbss_wpa2_flags;
	uint32 assoc_type;
	uint32 assoc_state;
} wl_bsscfg_t;

typedef struct wl_if_add {
	uint32  bsscfg_flags;
	uint32  if_flags;
	uint32  ap;
	struct ether_addr   mac_addr;
	uint16  PAD;
	uint32  wlc_index;
} wl_if_add_t;

typedef struct wl_bss_config {
	uint32	atim_window;
	uint32	beacon_period;
	uint32	chanspec;
} wl_bss_config_t;

/* Number of Bsscolor supported per core */
#ifndef HE_MAX_BSSCOLOR_RES
#define HE_MAX_BSSCOLOR_RES		2
#endif // endif

#ifndef HE_MAX_STAID_PER_BSSCOLOR
#define HE_MAX_STAID_PER_BSSCOLOR	4
#endif // endif

/* BSSColor indices */
#define BSSCOLOR0_IDX	0
#define BSSCOLOR1_IDX	1
#define HE_BSSCOLOR0	0
#define HE_BSSCOLOR_MAX_VAL 63

/* STAID indices */
#define STAID0_IDX	0
#define STAID1_IDX	1
#define STAID2_IDX	2
#define STAID3_IDX	3
#define HE_STAID_MAX_VAL	0x07FF

typedef struct wl_bsscolor_info {
	uint16 version;		/**< structure version */
	uint16 length;		/**< length of the bsscolor info */
	uint8	bsscolor_index;	/**< bsscolor index 0-1 */
	uint8	bsscolor;	/**<bsscolor value from 0 to 63 */
	uint8	partial_bsscolor_ind;
	uint8	disable_bsscolor_ind;	/**< To disable particular bsscolor */
	/* bsscolor_disable to be added as part of D1.0 */
	uint16	staid_info[HE_MAX_STAID_PER_BSSCOLOR];	/**< 0-3 staid info of each bsscolor */
} wl_bsscolor_info_t;

#define WL_BSS_USER_RADAR_CHAN_SELECT	0x1	/**< User application will randomly select
						 * radar channel.
						 */

#define DLOAD_HANDLER_VER		1	/**< Downloader version */
#define DLOAD_FLAG_VER_MASK		0xf000	/**< Downloader version mask */
#define DLOAD_FLAG_VER_SHIFT		12	/**< Downloader version shift */

#define DL_CRC_NOT_INUSE	0x0001
#define DL_BEGIN		0x0002
#define DL_END			0x0004

/* Flags for Major/Minor/Date number shift and mask */
#define EPI_VER_SHIFT     16
#define EPI_VER_MASK      0xFFFF
/** generic download types & flags */
enum {
	DL_TYPE_UCODE = 1,
	DL_TYPE_CLM = 2
};

/** ucode type values */
enum {
	UCODE_FW,
	INIT_VALS,
	BS_INIT_VALS
};

struct wl_dload_data {
	uint16 flag;
	uint16 dload_type;
	uint32 len;
	uint32 crc;
	uint8  data[1];
};
typedef struct wl_dload_data wl_dload_data_t;

struct wl_ucode_info {
	uint32 ucode_type;
	uint32 num_chunks;
	uint32 chunk_len;
	uint32 chunk_num;
	uint8  data_chunk[1];
};
typedef struct wl_ucode_info wl_ucode_info_t;

struct wl_clm_dload_info {
	uint32 ds_id;
	uint32 clm_total_len;
	uint32 num_chunks;
	uint32 chunk_len;
	uint32 chunk_offset;
	uint8  data_chunk[1];
};
typedef struct wl_clm_dload_info wl_clm_dload_info_t;

typedef struct wlc_ssid {
	uint32		SSID_len;
	uint8		SSID[DOT11_MAX_SSID_LEN];
} wlc_ssid_t;

typedef struct wlc_ssid_ext {
	uint8      hidden;
	uint8      PAD;
	uint16     flags;
	uint8      SSID_len;
	int8       rssi_thresh;
	uint8      SSID[DOT11_MAX_SSID_LEN];
} wlc_ssid_ext_t;

#define MAX_PREFERRED_AP_NUM     5
typedef struct wlc_fastssidinfo {
	uint32			SSID_channel[MAX_PREFERRED_AP_NUM];
	wlc_ssid_t		SSID_info[MAX_PREFERRED_AP_NUM];
} wlc_fastssidinfo_t;

typedef struct wnm_url {
	uint8   len;
	uint8   data[1];
} wnm_url_t;

typedef struct chan_scandata {
	uint8		txpower;
	uint8		pad;
	chanspec_t	channel;		/**< Channel num, bw, ctrl_sb and band */
	uint32		channel_mintime;
	uint32		channel_maxtime;
} chan_scandata_t;

typedef enum wl_scan_type {
	EXTDSCAN_FOREGROUND_SCAN,
	EXTDSCAN_BACKGROUND_SCAN,
	EXTDSCAN_FORCEDBACKGROUND_SCAN
} wl_scan_type_t;

#define WLC_EXTDSCAN_MAX_SSID		5

typedef struct wl_extdscan_params {
	int8		nprobes;		/**< 0, passive, otherwise active */
	int8		split_scan;		/**< split scan */
	int8		band;			/**< band */
	int8		pad;
	wlc_ssid_t	ssid[WLC_EXTDSCAN_MAX_SSID]; /**< ssid list */
	uint32		tx_rate;		/**< in 500ksec units */
	wl_scan_type_t	scan_type;		/**< enum */
	int32		channel_num;
	chan_scandata_t channel_list[1];	/**< list of chandata structs */
} wl_extdscan_params_t;

#define WL_EXTDSCAN_PARAMS_FIXED_SIZE	(sizeof(wl_extdscan_params_t) - sizeof(chan_scandata_t))

#define WL_SCAN_PARAMS_SSID_MAX		10

struct wl_scan_params {
	wlc_ssid_t ssid;		/**< default: {0, ""} */
	struct ether_addr bssid;	/**< default: bcast */
	int8 bss_type;			/**< default: any,
					 * DOT11_BSSTYPE_ANY/INFRASTRUCTURE/INDEPENDENT
					 */
	uint8 scan_type;		/**< flags, 0 use default */
	int32 nprobes;			/**< -1 use default, number of probes per channel */
	int32 active_time;		/**< -1 use default, dwell time per channel for
					 * active scanning
					 */
	int32 passive_time;		/**< -1 use default, dwell time per channel
					 * for passive scanning
					 */
	int32 home_time;		/**< -1 use default, dwell time for the home channel
					 * between channel scans
					 */
	int32 channel_num;		/**< count of channels and ssids that follow
					 *
					 * low half is count of channels in channel_list, 0
					 * means default (use all available channels)
					 *
					 * high half is entries in wlc_ssid_t array that
					 * follows channel_list, aligned for int32 (4 bytes)
					 * meaning an odd channel count implies a 2-byte pad
					 * between end of channel_list and first ssid
					 *
					 * if ssid count is zero, single ssid in the fixed
					 * parameter portion is assumed, otherwise ssid in
					 * the fixed portion is ignored
					 */
	uint16 channel_list[1];		/**< list of chanspecs */
};

/* changes in wl_scan_params_v2 as comapred to wl_scan_params (v1)
* unit8 scantype to uint32
*/
typedef struct wl_scan_params_v2 {
	uint16 version;			/* Version of wl_scan_params, change value of
					 * WL_SCAN_PARAM_VERSION on version update
					 */
	uint16 length;			/* length of structure wl_scan_params_v1_t
					 * without implicit pad
					 */
	wlc_ssid_t ssid;		/**< default: {0, ""} */
	struct ether_addr bssid;	/**< default: bcast */
	int8 bss_type;			/**< default: any,
					 * DOT11_BSSTYPE_ANY/INFRASTRUCTURE/INDEPENDENT
					 */
	uint8 PAD;
	uint32 scan_type;		/**< flags, 0 use default, and flags specified in
					 * WL_SCANFLAGS_XXX
					 */
	int32 nprobes;			/**< -1 use default, number of probes per channel */
	int32 active_time;		/**< -1 use default, dwell time per channel for
					 * active scanning
					 */
	int32 passive_time;		/**< -1 use default, dwell time per channel
					 * for passive scanning
					 */
	int32 home_time;		/**< -1 use default, dwell time for the home channel
					 * between channel scans
					 */
	int32 channel_num;		/**< count of channels and ssids that follow
					 *
					 * low half is count of channels in channel_list, 0
					 * means default (use all available channels)
					 *
					 * high half is entries in wlc_ssid_t array that
					 * follows channel_list, aligned for int32 (4 bytes)
					 * meaning an odd channel count implies a 2-byte pad
					 * between end of channel_list and first ssid
					 *
					 * if ssid count is zero, single ssid in the fixed
					 * parameter portion is assumed, otherwise ssid in
					 * the fixed portion is ignored
					 */
	uint16 channel_list[1];		/**< list of chanspecs */
} wl_scan_params_v2_t;

#define WL_SCAN_PARAMS_VERSION_V2		2

/** size of wl_scan_params not including variable length array */
#define WL_SCAN_PARAMS_V2_FIXED_SIZE	(OFFSETOF(wl_scan_params_v2_t, channel_list))
#define WL_MAX_ROAMSCAN_DATSZ	\
	(WL_SCAN_PARAMS_FIXED_SIZE + (WL_NUMCHANNELS * sizeof(uint16)))
#define WL_MAX_ROAMSCAN_V2_DATSZ \
	(WL_SCAN_PARAMS_V2_FIXED_SIZE + (WL_NUMCHANNELS * sizeof(uint16)))

#define ISCAN_REQ_VERSION 1
#define ISCAN_REQ_VERSION_V2 2

/** incremental scan struct */
struct wl_iscan_params {
	uint32 version;
	uint16 action;
	uint16 scan_duration;
	struct wl_scan_params params;
};

/** incremental scan struct */
typedef struct wl_iscan_params_v2 {
	uint32 version;
	uint16 action;
	uint16 scan_duration;
	wl_scan_params_v2_t params;
} wl_iscan_params_v2_t;

/** 3 fields + size of wl_scan_params, not including variable length array */
#define WL_ISCAN_PARAMS_FIXED_SIZE	(OFFSETOF(wl_iscan_params_t, params) + sizeof(wlc_ssid_t))
#define WL_ISCAN_PARAMS_V2_FIXED_SIZE \
	(OFFSETOF(wl_iscan_params_v2_t, params) + sizeof(wlc_ssid_t))

typedef struct wl_scan_results {
	uint32 buflen;
	uint32 version;
	uint32 count;
	wl_bss_info_v109_t bss_info[1];
} wl_scan_results_v109_t;

typedef struct wl_scan_results_v2 {
	uint32 buflen;
	uint32 version;
	uint32 count;
	uint8 bss_info[];	/* var length wl_bss_info_X structures */
} wl_scan_results_v2_t;

#ifndef WL_BSS_INFO_TYPEDEF_HAS_ALIAS
typedef wl_scan_results_v109_t wl_scan_results_t;
/** size of wl_scan_results not including variable length array */
#define WL_SCAN_RESULTS_FIXED_SIZE (sizeof(wl_scan_results_t) - sizeof(wl_bss_info_t))
#endif // endif

#if defined(SIMPLE_ISCAN)
/** the buf lengh can be WLC_IOCTL_MAXLEN (8K) to reduce iteration */
#define WLC_IW_ISCAN_MAXLEN   2048
typedef struct iscan_buf {
	struct iscan_buf * next;
	int8   iscan_buf[WLC_IW_ISCAN_MAXLEN];
} iscan_buf_t;
#endif /* SIMPLE_ISCAN */
#define ESCAN_REQ_VERSION 1
#define ESCAN_REQ_VERSION_V2 2

/** event scan reduces amount of SOC memory needed to store scan results */
struct wl_escan_params {
	uint32 version;
	uint16 action;
	uint16 sync_id;
	struct wl_scan_params params;
};

typedef struct wl_escan_params_v2 {
	uint32 version;
	uint16 action;
	uint16 sync_id;
	wl_scan_params_v2_t params;
} wl_escan_params_v2_t;

#define WL_ESCAN_PARAMS_FIXED_SIZE (OFFSETOF(wl_escan_params_t, params) + sizeof(wlc_ssid_t))
#define WL_ESCAN_PARAMS_V2_FIXED_SIZE (OFFSETOF(wl_escan_params_v2_t, params) + sizeof(wlc_ssid_t))

/* New scan version is defined then change old version of scan to
 * wl_scan_params_v1_t and new one to wl_scan_params_t
 */
#ifdef WL_SCAN_PARAMS_V2
typedef struct wl_scan_params	wl_scan_params_v1_t;
typedef struct wl_escan_params	wl_escan_params_v1_t;
typedef struct wl_iscan_params	wl_iscan_params_v1_t;
typedef struct wl_scan_params_v2	wl_scan_params_t;
typedef struct wl_escan_params_v2	wl_escan_params_t;
typedef struct wl_iscan_params_v2	wl_iscan_params_t;
#define WL_SCAN_PARAMS_FIXED_SIZE	(OFFSETOF(wl_scan_params_t, channel_list))
#else
typedef struct wl_scan_params wl_scan_params_t;
typedef struct wl_escan_params wl_escan_params_t;
typedef struct wl_iscan_params wl_iscan_params_t;
#define WL_SCAN_PARAMS_FIXED_SIZE	64
#endif // endif

/** event scan reduces amount of SOC memory needed to store scan results */
typedef struct wl_escan_result {
	uint32 buflen;
	uint32 version;
	uint16 sync_id;
	uint16 bss_count;
	wl_bss_info_v109_t bss_info[1];
} wl_escan_result_v109_t;

/** event scan reduces amount of SOC memory needed to store scan results */
typedef struct wl_escan_result_v2 {
	uint32 buflen;
	uint32 version;
	uint16 sync_id;
	uint16 bss_count;
	uint8 bss_info[];	/* var length wl_bss_info_X structures */
} wl_escan_result_v2_t;

#ifndef WL_BSS_INFO_TYPEDEF_HAS_ALIAS
typedef wl_escan_result_v109_t wl_escan_result_t;
#define WL_ESCAN_RESULTS_FIXED_SIZE (sizeof(wl_escan_result_t) - sizeof(wl_bss_info_t))
#endif // endif

typedef struct wl_gscan_result {
	uint32 buflen;
	uint32 version;
	uint32 scan_ch_bucket;
	wl_gscan_bss_info_v2_t bss_info[1];
} wl_gscan_result_v2_t;

typedef struct wl_gscan_result_v2_1 {
	uint32 buflen;
	uint32 version;
	uint32 scan_ch_bucket;
	uint8 bss_info[];	/* var length wl_bss_info_X structures */
} wl_gscan_result_v2_1_t;

#ifndef WL_BSS_INFO_TYPEDEF_HAS_ALIAS
typedef wl_gscan_result_v2_t wl_gscan_result_t;
#define WL_GSCAN_RESULTS_FIXED_SIZE (sizeof(wl_gscan_result_t) - sizeof(wl_gscan_bss_info_t))
#endif // endif

/** incremental scan results struct */
typedef struct wl_iscan_results {
	uint32 status;
	wl_scan_results_v109_t results;
} wl_iscan_results_v109_t;

/** incremental scan results struct */
typedef struct wl_iscan_results_v2 {
	uint32 status;
	wl_scan_results_v2_t results;
} wl_iscan_results_v2_t;

#ifndef WL_BSS_INFO_TYPEDEF_HAS_ALIAS
typedef wl_iscan_results_v109_t wl_iscan_results_t;
/** size of wl_iscan_results not including variable length array */
#define WL_ISCAN_RESULTS_FIXED_SIZE \
	(WL_SCAN_RESULTS_FIXED_SIZE + OFFSETOF(wl_iscan_results_t, results))
#endif // endif

typedef struct wl_probe_params {
	wlc_ssid_t ssid;
	struct ether_addr bssid;
	struct ether_addr mac;
} wl_probe_params_t;

#define WL_MAXRATES_IN_SET		16	/**< max # of rates in a rateset */

typedef struct wl_rateset {
	uint32	count;				/**< # rates in this set */
	uint8	rates[WL_MAXRATES_IN_SET];	/**< rates in 500kbps units w/hi bit set if basic */
} wl_rateset_t;

#define WL_VHT_CAP_MCS_MAP_NSS_MAX	8

typedef struct wl_rateset_args_v1 {
	uint32	count;				/**< # rates in this set */
	uint8	rates[WL_MAXRATES_IN_SET];	/**< rates in 500kbps units w/hi bit set if basic */
	uint8   mcs[MCSSET_LEN];	/**< supported mcs index bit map */
	uint16 vht_mcs[WL_VHT_CAP_MCS_MAP_NSS_MAX]; /**< supported mcs index bit map per nss */
} wl_rateset_args_v1_t;

#define RATESET_ARGS_V1		(1)
#define RATESET_ARGS_V2		(2)

/* RATESET_VERSION_ENABLED is defined in wl.mk post J branch.
 * Guidelines to use wl_rateset_args_t:
 * [a] in wlioctl.h: Add macro RATESET_ARGS_VX where X is the new version number.
 * [b] in wlioctl.h: Add a new structure with wl_rateset_args_vX_t
 * [c] in wlu.c app: Add support to parse new structure under RATESET_ARGS_VX
 * [d] in wlc_types.h: in respective branch and trunk: redefine wl_rateset_args_t with
 *	new wl_rateset_args_vX_t
 */
#ifndef RATESET_VERSION_ENABLED
/* rateset structure before versioning. legacy. DONOT update anymore here */
#define RATESET_ARGS_VERSION	(RATESET_ARGS_V1)
typedef wl_rateset_args_v1_t wl_rateset_args_t;
#endif /* RATESET_VERSION_ENABLED */

/* Note: dependent structures: sta_info_vX_t. When any update to this structure happens,
 *	update sta_info_vX_t also.
 */
#define WL_HE_CAP_MCS_MAP_NSS_MAX	8

typedef struct wl_rateset_args_v2 {
	uint16 version;		/**< version. */
	uint16 len;		/**< length */
	uint32	count;		/**< # rates in this set */
	uint8	rates[WL_MAXRATES_IN_SET];	/**< rates in 500kbps units w/hi bit set if basic */
	uint8   mcs[MCSSET_LEN];		/**< supported mcs index bit map */
	uint16 vht_mcs[WL_VHT_CAP_MCS_MAP_NSS_MAX]; /**< supported mcs index bit map per nss */
	uint16 he_mcs[WL_HE_CAP_MCS_MAP_NSS_MAX]; /**< supported he mcs index bit map per nss */
} wl_rateset_args_v2_t;

/* HE Rates BITMAP */
#define WL_HE_CAP_MCS_0_7_MAP		0x00ff
#define WL_HE_CAP_MCS_0_8_MAP		0x01ff
#define WL_HE_CAP_MCS_0_9_MAP		0x03ff
#define WL_HE_CAP_MCS_0_10_MAP		0x07ff
#define WL_HE_CAP_MCS_0_11_MAP		0x0fff

#define TXBF_RATE_MCS_ALL		4
#define TXBF_RATE_VHT_ALL		4
#define TXBF_RATE_OFDM_ALL		8

typedef struct wl_txbf_rateset {
	uint8	txbf_rate_mcs[TXBF_RATE_MCS_ALL];	/**< one for each stream */
	uint8	txbf_rate_mcs_bcm[TXBF_RATE_MCS_ALL];	/**< one for each stream */
	uint16	txbf_rate_vht[TXBF_RATE_VHT_ALL];	/**< one for each stream */
	uint16	txbf_rate_vht_bcm[TXBF_RATE_VHT_ALL];	/**< one for each stream */
	uint8	txbf_rate_ofdm[TXBF_RATE_OFDM_ALL]; /**< bitmap of ofdm rates that enables txbf */
	uint8	txbf_rate_ofdm_bcm[TXBF_RATE_OFDM_ALL]; /* bitmap of ofdm rates that enables txbf */
	uint8	txbf_rate_ofdm_cnt;
	uint8	txbf_rate_ofdm_cnt_bcm;
} wl_txbf_rateset_t;

#define NUM_BFGAIN_ARRAY_1RX	2
#define NUM_BFGAIN_ARRAY_2RX	3
#define NUM_BFGAIN_ARRAY_3RX	4
#define NUM_BFGAIN_ARRAY_4RX	5

typedef struct wl_txbf_expgainset {
	/* bitmap for each element: B[4:0]=>c0, B[9:5]=>c1, B[14:10]=>c2, B[19:15]=>c[3-7]
	 * B[24:20]=>c[8-9], B[29:25]=>c[10-11]
	 */
	uint32	bfgain_2x1[NUM_BFGAIN_ARRAY_1RX]; /* exp     1ss, imp 1ss */
	uint32	bfgain_2x2[NUM_BFGAIN_ARRAY_2RX]; /* exp [1-2]ss, imp 1ss */
	uint32	bfgain_3x1[NUM_BFGAIN_ARRAY_1RX];
	uint32	bfgain_3x2[NUM_BFGAIN_ARRAY_2RX];
	uint32	bfgain_3x3[NUM_BFGAIN_ARRAY_3RX]; /* exp [1-3]ss, imp 1ss */
	uint32	bfgain_4x1[NUM_BFGAIN_ARRAY_1RX];
	uint32	bfgain_4x2[NUM_BFGAIN_ARRAY_2RX];
	uint32	bfgain_4x3[NUM_BFGAIN_ARRAY_3RX];
	uint32	bfgain_4x4[NUM_BFGAIN_ARRAY_4RX]; /* exp [1-4]ss, imp 1ss */
} wl_txbf_expgainset_t;

#define OFDM_RATE_MASK			0x0000007f
typedef uint8 ofdm_rates_t;

typedef struct wl_rates_info {
	wl_rateset_t rs_tgt;
	uint32 phy_type;
	int32 bandtype;
	uint8 cck_only;
	uint8 rate_mask;
	uint8 mcsallow;
	uint8 bw;
	uint8 txstreams;
	uint8 PAD[3];
} wl_rates_info_t;

/**uint32 list */
typedef struct wl_uint32_list {
	/** in - # of elements, out - # of entries */
	uint32 count;
	/** variable length uint32 list */
	uint32 element[1];
} wl_uint32_list_t;

/* WLC_SET_ALLOW_MODE values */
#define ALLOW_MODE_ANY_BSSID		0
#define ALLOW_MODE_ONLY_DESIRED_BSSID	1
#define ALLOW_MODE_NO_BSSID		2

/** used for association with a specific BSSID and chanspec list */
typedef struct wl_assoc_params {
	struct ether_addr bssid;	/**< 00:00:00:00:00:00: broadcast scan */
	uint16 bssid_cnt;		/**< 0: use chanspec_num, and the single bssid,
					* otherwise count of chanspecs in chanspec_list
					* AND paired bssids following chanspec_list
					* also, chanspec_num has to be set to zero
					* for bssid list to be used
					*/
	int32 chanspec_num;		/**< 0: all available channels,
					* otherwise count of chanspecs in chanspec_list
					*/
	chanspec_t chanspec_list[1];	/**< list of chanspecs */
} wl_assoc_params_t;

#define WL_ASSOC_PARAMS_FIXED_SIZE 	OFFSETOF(wl_assoc_params_t, chanspec_list)

/** used for reassociation/roam to a specific BSSID and channel */
typedef wl_assoc_params_t wl_reassoc_params_t;
#define WL_REASSOC_PARAMS_FIXED_SIZE	WL_ASSOC_PARAMS_FIXED_SIZE

/** used for association to a specific BSSID and channel */
typedef wl_assoc_params_t wl_join_assoc_params_t;
#define WL_JOIN_ASSOC_PARAMS_FIXED_SIZE	WL_ASSOC_PARAMS_FIXED_SIZE

/** used for join with or without a specific bssid and channel list */
typedef struct wl_join_params {
	wlc_ssid_t ssid;
	wl_assoc_params_t params;	/**< optional field, but it must include the fixed portion
					 * of the wl_assoc_params_t struct when it does present.
					 */
} wl_join_params_t;

#define WL_JOIN_PARAMS_FIXED_SIZE 	(OFFSETOF(wl_join_params_t, params) + \
					 WL_ASSOC_PARAMS_FIXED_SIZE)

typedef struct wlc_roam_exp_params {
	int8 a_band_boost_threshold;
	int8 a_band_penalty_threshold;
	int8 a_band_boost_factor;
	int8 a_band_penalty_factor;
	int8 cur_bssid_boost;
	int8 alert_roam_trigger_threshold;
	int16 a_band_max_boost;
} wlc_roam_exp_params_t;

#define ROAM_EXP_CFG_VERSION     1

#define ROAM_EXP_ENABLE_FLAG             (1 << 0)

#define ROAM_EXP_CFG_PRESENT             (1 << 1)

typedef struct wl_roam_exp_cfg {
	uint16 version;
	uint16 flags;
	wlc_roam_exp_params_t params;
} wl_roam_exp_cfg_t;

typedef struct wl_bssid_pref_list {
	struct ether_addr bssid;
	/* Add this to modify rssi */
	int8 rssi_factor;
	int8 flags;
} wl_bssid_pref_list_t;

#define BSSID_PREF_LIST_VERSION        1
#define ROAM_EXP_CLEAR_BSSID_PREF        (1 << 0)

typedef struct wl_bssid_pref_cfg {
	uint16 version;
	uint16 flags;
	uint16 count;
	uint16 reserved;
	wl_bssid_pref_list_t bssids[];
} wl_bssid_pref_cfg_t;

#define SSID_WHITELIST_VERSION         1

#define ROAM_EXP_CLEAR_SSID_WHITELIST    (1 << 0)

/* Roam SSID whitelist, ssids in this list are ok to  */
/* be considered as targets to join when considering a roam */

typedef struct wl_ssid_whitelist {

	uint16 version;
	uint16 flags;

	uint8 ssid_count;
	uint8 reserved[3];
	wlc_ssid_t ssids[];
} wl_ssid_whitelist_t;

#define ROAM_EXP_EVENT_VERSION       1

typedef struct wl_roam_exp_event {

	uint16 version;
	uint16 flags;
	wlc_ssid_t cur_ssid;
} wl_roam_exp_event_t;

/** scan params for extended join */
typedef struct wl_join_scan_params {
	uint8 scan_type;		/**< 0 use default, active or passive scan */
	uint8 PAD[3];
	int32 nprobes;			/**< -1 use default, number of probes per channel */
	int32 active_time;		/**< -1 use default, dwell time per channel for
					 * active scanning
					 */
	int32 passive_time;		/**< -1 use default, dwell time per channel
					 * for passive scanning
					 */
	int32 home_time;		/**< -1 use default, dwell time for the home channel
					 * between channel scans
					 */
} wl_join_scan_params_t;

/** extended join params */
typedef struct wl_extjoin_params {
	wlc_ssid_t ssid;		/**< {0, ""}: wildcard scan */
	wl_join_scan_params_t scan;
	wl_join_assoc_params_t assoc;	/**< optional field, but it must include the fixed portion
					 * of the wl_join_assoc_params_t struct when it does
					 * present.
					 */
} wl_extjoin_params_t;
#define WL_EXTJOIN_PARAMS_FIXED_SIZE 	(OFFSETOF(wl_extjoin_params_t, assoc) + \
					 WL_JOIN_ASSOC_PARAMS_FIXED_SIZE)

#define ANT_SELCFG_MAX		4	/**< max number of antenna configurations */
#define MAX_STREAMS_SUPPORTED	4	/**< max number of streams supported */
typedef struct {
	uint8 ant_config[ANT_SELCFG_MAX];	/**< antenna configuration */
	uint8 num_antcfg;			/**< number of available antenna configurations */
} wlc_antselcfg_t;

typedef struct {
	uint32 duration;		/**< millisecs spent sampling this channel */
	union {
		uint32 congest_ibss;	/**< millisecs in our bss (presumably this traffic will */
					/**<  move if cur bss moves channels) */
		uint32 congest_me;	/**< millisecs in my own traffic */
	};
	union {
		uint32 congest_obss;	/**< traffic not in our bss */
		uint32 congest_notme;	/**< traffic not from/to me (including bc/mc) */
	};
	uint32 interference;		/**< millisecs detecting a non 802.11 interferer. */
	uint32 timestamp;		/**< second timestamp */
} cca_congest_t;

typedef struct {
	chanspec_t chanspec;		/**< Which channel? */
	uint16 num_secs;		/**< How many secs worth of data */
	cca_congest_t  secs[1];		/**< Data */
} cca_congest_channel_req_t;

typedef struct {
	uint32 timestamp;		/**< second timestamp */

	/* Base structure of cca_congest_t: CCA statistics all inclusive */
	uint32 duration;		/**< millisecs spent sampling this channel */
	uint32 congest_meonly;		/**< millisecs in my own traffic (TX + RX) */
	uint32 congest_ibss;		/**< millisecs in our bss (presumably this traffic will */
					/**<  move if cur bss moves channels) */
	uint32 congest_obss;		/**< traffic not in our bss */
	uint32 interference;		/**< millisecs detecting a non 802.11 interferer. */

	/* CCA statistics for non PM only */
	uint32 duration_nopm;		/**< millisecs spent sampling this channel */
	uint32 congest_meonly_nopm;	/**< millisecs in my own traffic (TX + RX) */
	uint32 congest_ibss_nopm;	/**< millisecs in our bss (presumably this traffic will */
					/**<  move if cur bss moves channels) */
	uint32 congest_obss_nopm;	/**< traffic not in our bss */
	uint32 interference_nopm;	/**< millisecs detecting a non 802.11 interferer. */

	/* CCA statistics for during PM only */
	uint32 duration_pm;		/**< millisecs spent sampling this channel */
	uint32 congest_meonly_pm;	/**< millisecs in my own traffic (TX + RX) */
	uint32 congest_ibss_pm;		/**< millisecs in our bss (presumably this traffic will */
					/**<  move if cur bss moves channels) */
	uint32 congest_obss_pm;		/**< traffic not in our bss */
	uint32 interference_pm;		/**< millisecs detecting a non 802.11 interferer. */
} cca_congest_ext_t;

#define WL_CCA_EXT_REQ_VER		0
typedef struct {
	uint16 ver;			/**< version of this struct */
	uint16 len;			/**< len of this structure */
	chanspec_t chanspec;		/**< Which channel? */
	uint16 num_secs;		/**< How many secs worth of data */
	cca_congest_ext_t secs[1];	/**< Data - 3 sets for ALL - non-PM - PM */
} cca_congest_ext_channel_req_t;

typedef struct {
	uint32 duration;	/**< millisecs spent sampling this channel */
	uint32 congest;		/**< millisecs detecting busy CCA */
	uint32 timestamp;	/**< second timestamp */
} cca_congest_simple_t;

/* The following two structure must have same first 4 fields.
 * The cca_chan_qual_event_t is used to report CCA in older formats and NF.
 * The cca_only_chan_qual_event_t is used to report CCA only with newer format.
 */
typedef struct {
	uint16 status;
	uint16 id;
	chanspec_t chanspec;				/**< Which channel? */
	uint16 len;
	union {
		cca_congest_simple_t  cca_busy;		/**< CCA busy */
		cca_congest_t cca_busy_ext;		/**< Extended CCA report */
		int32 noise;				/**< noise floor */
	};
} cca_chan_qual_event_t;

typedef struct {
	uint16 status;
	uint16 id;
	chanspec_t chanspec;				/**< Which channel? */
	uint16 len;
	union {
		cca_congest_simple_t  cca_busy;		/**< CCA busy */
		struct {
			cca_congest_t cca_busy_ext;	/**< Extended CCA report */
			cca_congest_t cca_busy_nopm;	/**< Extedned CCA report (PM awake time) */
			cca_congest_t cca_busy_pm;	/**< Extedned CCA report (PM sleep time) */
		};
	};
} cca_only_chan_qual_event_t;

typedef struct {
	uint32 msrmnt_time;	/**< Time for Measurement (msec) */
	uint32 msrmnt_done;	/**< flag set when measurement complete */
	char buf[];
} cca_stats_n_flags;

typedef struct {
	uint32 msrmnt_query;    /* host to driver query for measurement done */
	uint32 time_req;        /* time required for measurement */
	uint8 report_opt;       /* option to print different stats in report */
	uint8 PAD[3];
} cca_msrmnt_query;

/* interference sources */
enum interference_source {
	ITFR_NONE = 0,			/**< interference */
	ITFR_PHONE,			/**< wireless phone */
	ITFR_VIDEO_CAMERA,		/**< wireless video camera */
	ITFR_MICROWAVE_OVEN,		/**< microwave oven */
	ITFR_BABY_MONITOR,		/**< wireless baby monitor */
	ITFR_BLUETOOTH,			/**< bluetooth */
	ITFR_VIDEO_CAMERA_OR_BABY_MONITOR,	/**< wireless camera or baby monitor */
	ITFR_BLUETOOTH_OR_BABY_MONITOR,	/**< bluetooth or baby monitor */
	ITFR_VIDEO_CAMERA_OR_PHONE,	/**< video camera or phone */
	ITFR_UNIDENTIFIED		/**< interference from unidentified source */
};

/** structure for interference source report */
typedef struct {
	uint32 flags;		/**< flags.  bit definitions below */
	uint32 source;		/**< last detected interference source */
	uint32 timestamp;	/**< second timestamp on interferenced flag change */
} interference_source_rep_t;

#define WLC_CNTRY_BUF_SZ	4		/**< Country string is 3 bytes + NUL */

typedef struct wl_country {
	char country_abbrev[WLC_CNTRY_BUF_SZ];	/**< nul-terminated country code used in
						 * the Country IE
						 */
	int32 rev;				/**< revision specifier for ccode
						 * on set, -1 indicates unspecified.
						 * on get, rev >= 0
						 */
	char ccode[WLC_CNTRY_BUF_SZ];		/**< nul-terminated built-in country code.
						 * variable length, but fixed size in
						 * struct allows simple allocation for
						 * expected country strings <= 3 chars.
						 */
} wl_country_t;

#define CCODE_INFO_VERSION 1

typedef enum wl_ccode_role {
	WLC_CCODE_ROLE_ACTIVE = 0,
	WLC_CCODE_ROLE_HOST,
	WLC_CCODE_ROLE_80211D_ASSOC,
	WLC_CCODE_ROLE_80211D_SCAN,
	WLC_CCODE_ROLE_DEFAULT,
	WLC_CCODE_ROLE_DEFAULT_SROM_BKUP,
	WLC_CCODE_LAST
} wl_ccode_role_t;
#define WLC_NUM_CCODE_INFO WLC_CCODE_LAST

typedef struct wl_ccode_entry {
	uint16 reserved;
	uint8 band;
	uint8 role;
	char	ccode[WLC_CNTRY_BUF_SZ];
} wl_ccode_entry_t;

typedef struct wl_ccode_info {
	uint16 version;
	uint16 count;   /**< Number of ccodes entries in the set */
	wl_ccode_entry_t ccodelist[1];
} wl_ccode_info_t;
#define WL_CCODE_INFO_FIXED_LEN	OFFSETOF(wl_ccode_info_t, ccodelist)
typedef struct wl_channels_in_country {
	uint32 buflen;
	uint32 band;
	char country_abbrev[WLC_CNTRY_BUF_SZ];
	uint32 count;
	uint32 channel[1];
} wl_channels_in_country_t;

typedef struct wl_country_list {
	uint32 buflen;
	uint32 band_set;
	uint32 band;
	uint32 count;
	char country_abbrev[1];
} wl_country_list_t;

typedef struct wl_rm_req_elt {
	int8	type;
	int8	flags;
	chanspec_t	chanspec;
	uint32	token;		/**< token for this measurement */
	uint32	tsf_h;		/**< TSF high 32-bits of Measurement start time */
	uint32	tsf_l;		/**< TSF low 32-bits */
	uint32	dur;		/**< TUs */
} wl_rm_req_elt_t;

typedef struct wl_rm_req {
	uint32	token;		/**< overall measurement set token */
	uint32	count;		/**< number of measurement requests */
	void	*cb;		/**< completion callback function: may be NULL */
	void	*cb_arg;	/**< arg to completion callback function */
	wl_rm_req_elt_t	req[1];	/**< variable length block of requests */
} wl_rm_req_t;
#define WL_RM_REQ_FIXED_LEN	OFFSETOF(wl_rm_req_t, req)

typedef struct wl_rm_rep_elt {
	int8	type;
	int8	flags;
	chanspec_t	chanspec;
	uint32	token;		/**< token for this measurement */
	uint32	tsf_h;		/**< TSF high 32-bits of Measurement start time */
	uint32	tsf_l;		/**< TSF low 32-bits */
	uint32	dur;		/**< TUs */
	uint32	len;		/**< byte length of data block */
	uint8	data[1];	/**< variable length data block */
} wl_rm_rep_elt_t;
#define WL_RM_REP_ELT_FIXED_LEN	24	/**< length excluding data block */

#define WL_RPI_REP_BIN_NUM 8
typedef struct wl_rm_rpi_rep {
	uint8	rpi[WL_RPI_REP_BIN_NUM];
	int8	rpi_max[WL_RPI_REP_BIN_NUM];
} wl_rm_rpi_rep_t;

typedef struct wl_rm_rep {
	uint32	token;		/**< overall measurement set token */
	uint32	len;		/**< length of measurement report block */
	wl_rm_rep_elt_t	rep[1];	/**< variable length block of reports */
} wl_rm_rep_t;
#define WL_RM_REP_FIXED_LEN	8
#ifdef BCMCCX

#define LEAP_USER_MAX		32
#define LEAP_DOMAIN_MAX		32
#define LEAP_PASSWORD_MAX	32

typedef struct wl_leap_info {
	wlc_ssid_t ssid;
	uint8 user_len;
	uint8 user[LEAP_USER_MAX];
	uint8 password_len;
	uint8 password[LEAP_PASSWORD_MAX];
	uint8 domain_len;
	uint8 domain[LEAP_DOMAIN_MAX];
	uint8 PAD;
} wl_leap_info_t;

typedef struct wl_leap_list {
	uint32 buflen;
	uint32 version;
	uint32 count;
	wl_leap_info_t leap_info[1];
} wl_leap_list_t;
#endif	/* BCMCCX */

typedef enum sup_auth_status {
	/* Basic supplicant authentication states */
	WLC_SUP_DISCONNECTED = 0,
	WLC_SUP_CONNECTING,
	WLC_SUP_IDREQUIRED,
	WLC_SUP_AUTHENTICATING,
	WLC_SUP_AUTHENTICATED,
	WLC_SUP_KEYXCHANGE,
	WLC_SUP_KEYED,
	WLC_SUP_TIMEOUT,
	WLC_SUP_LAST_BASIC_STATE,

	/* Extended supplicant authentication states */
	/** Waiting to receive handshake msg M1 */
	WLC_SUP_KEYXCHANGE_WAIT_M1 = WLC_SUP_AUTHENTICATED,
	/** Preparing to send handshake msg M2 */
	WLC_SUP_KEYXCHANGE_PREP_M2 = WLC_SUP_KEYXCHANGE,
	/* Waiting to receive handshake msg M3 */
	WLC_SUP_KEYXCHANGE_WAIT_M3 = WLC_SUP_LAST_BASIC_STATE,
	WLC_SUP_KEYXCHANGE_PREP_M4,	/**< Preparing to send handshake msg M4 */
	WLC_SUP_KEYXCHANGE_WAIT_G1,	/**< Waiting to receive handshake msg G1 */
	WLC_SUP_KEYXCHANGE_PREP_G2	/**< Preparing to send handshake msg G2 */
} sup_auth_status_t;

typedef struct wl_wsec_key {
	uint32		index;		/**< key index */
	uint32		len;		/**< key length */
	uint8		data[DOT11_MAX_KEY_SIZE];	/**< key data */
	uint32		pad_1[18];
	uint32		algo;		/**< CRYPTO_ALGO_AES_CCM, CRYPTO_ALGO_WEP128, etc */
	uint32		flags;		/**< misc flags */
	uint32		pad_2[2];
	int32		pad_3;
	int32		iv_initialized;	/**< has IV been initialized already? */
	int32		pad_4;
	/* Rx IV */
	struct {
		uint32	hi;		/**< upper 32 bits of IV */
		uint16	lo;		/**< lower 16 bits of IV */
		uint16	PAD;
	} rxiv;
	uint32		pad_5[2];
	struct ether_addr ea;		/**< per station */
	uint16	    PAD;
} wl_wsec_key_t;

/* Min length for PSK passphrase */
#define WSEC_MIN_PSK_LEN	8
/* Max length of supported passphrases for PSK */
#define WSEC_MAX_PSK_LEN	64
/* Max length of supported passphrases for SAE */
#define WSEC_MAX_PASSPHRASE_LEN 256u

/* Flag for key material needing passhash'ing */
#define WSEC_PASSPHRASE		1u
/* Flag indicating an SAE passphrase */
#define WSEC_SAE_PASSPHRASE 2u

/**receptacle for WLC_SET_WSEC_PMK parameter */
typedef struct wsec_pmk {
	ushort	key_len;		/* octets in key material */
	ushort	flags;			/* key handling qualification */
	uint8	key[WSEC_MAX_PASSPHRASE_LEN];	/* PMK material */
} wsec_pmk_t;

#define WL_AUTH_EVENT_DATA_V1		0x1

/* tlv ids for auth event */
#define WL_AUTH_PMK_TLV_ID	1
#define WL_AUTH_PMKID_TLV_ID	2
/* AUTH event data
* pmk and pmkid in case of SAE auth
* xtlvs will be 32 bit alligned
*/
typedef struct wl_auth_event {
	uint16 version;
	uint16 length;
	uint8 xtlvs[];
} wl_auth_event_t;

#define WL_AUTH_EVENT_FIXED_LEN_V1	OFFSETOF(wl_auth_event_t, xtlvs)

#define WL_PMKSA_EVENT_DATA_V1	1u

/* tlv ids for PMKSA event */
#define WL_PMK_TLV_ID		1u
#define WL_PMKID_TLV_ID		2u
#define WL_PEER_ADDR_TLV_ID	3u

/* PMKSA event data structure */
typedef struct wl_pmksa_event {
	uint16 version;
	uint16 length;
	uint8 xtlvs[];
} wl_pmksa_event_t;

#define WL_PMKSA_EVENT_FIXED_LEN_V1	OFFSETOF(wl_pmksa_event_t, xtlvs)

#define FILS_CACHE_ID_LEN	2u
#define PMK_LEN_MAX		48u

typedef struct _pmkid_v1 {
	struct ether_addr	BSSID;
	uint8				PMKID[WPA2_PMKID_LEN];
} pmkid_v1_t;

#define PMKID_ELEM_V2_LENGTH (sizeof(struct ether_addr) + WPA2_PMKID_LEN + PMK_LEN_MAX + \
	sizeof(ssid_info_t) + FILS_CACHE_ID_LEN)

typedef struct _pmkid_v2 {
	uint16				length; /* Should match PMKID_ELEM_VX_LENGTH */
	struct ether_addr	BSSID;
	uint8				PMKID[WPA2_PMKID_LEN];
	uint8				pmk[PMK_LEN_MAX]; /* for FILS key deriviation */
	uint16				pmk_len;
	ssid_info_t			ssid;
	uint8				fils_cache_id[FILS_CACHE_ID_LEN];
} pmkid_v2_t;

#define PMKID_LIST_VER_2	2

typedef struct _pmkid_v3 {
	struct ether_addr	bssid;
	uint8			pmkid[WPA2_PMKID_LEN];
	uint8			pmkid_len;
	uint8			pmk[PMK_LEN_MAX];
	uint8			pmk_len;
	uint16			fils_cache_id; /* 2-byte length */
	uint8			pad;
	uint8			ssid_len;
	uint8			ssid[DOT11_MAX_SSID_LEN]; /* For FILS, to save ESSID */
							  /* one pmkid used in whole ESS */
	uint32			time_left; /* remaining time until expirary in sec. */
					   /* 0 means expired, all 0xFF means never expire */
} pmkid_v3_t;

#define PMKID_LIST_VER_3	3
typedef struct _pmkid_list_v1 {
	uint32	npmkid;
	pmkid_v1_t	pmkid[1];
} pmkid_list_v1_t;

typedef struct _pmkid_list_v2 {
	uint16  version;
	uint16	length;
	pmkid_v2_t	pmkid[1];
} pmkid_list_v2_t;

typedef struct _pmkid_list_v3 {
	uint16		version;
	uint16		length;
	uint16		count;
	uint16          pad;
	pmkid_v3_t	pmkid[];
} pmkid_list_v3_t;

#ifndef PMKID_VERSION_ENABLED
/* pmkid structure before versioning. legacy. DONOT update anymore here */
typedef pmkid_v1_t pmkid_t;
typedef pmkid_list_v1_t pmkid_list_t;
#endif /* PMKID_VERSION_ENABLED */

typedef struct _pmkid_cand {
	struct ether_addr	BSSID;
	uint8			preauth;
} pmkid_cand_t;

typedef struct _pmkid_cand_list {
	uint32	npmkid_cand;
	pmkid_cand_t	pmkid_cand[1];
} pmkid_cand_list_t;

#define WL_STA_ANT_MAX		4	/**< max possible rx antennas */

typedef struct wl_assoc_info {
	uint32		req_len;
	uint32		resp_len;
	uint32		flags;
	struct dot11_assoc_req req;
	struct ether_addr reassoc_bssid; /**< used in reassoc's */
	struct dot11_assoc_resp resp;
	uint32		state;
} wl_assoc_info_t;

typedef struct wl_led_info {
	uint32      index;      /**< led index */
	uint32      behavior;
	uint8       activehi;
	uint8       PAD[3];
} wl_led_info_t;

/** srom read/write struct passed through ioctl */
typedef struct {
	uint32	byteoff;	/**< byte offset */
	uint32	nbytes;		/**< number of bytes */
	uint16	buf[];
} srom_rw_t;

#define CISH_FLAG_PCIECIS	(1 << 15)	/**< write CIS format bit for PCIe CIS */

/** similar cis (srom or otp) struct [iovar: may not be aligned] */
typedef struct {
	uint16	source;		/**< cis source */
	uint16	flags;		/**< flags */
	uint32	byteoff;	/**< byte offset */
	uint32	nbytes;		/**< number of bytes */
	/* data follows here */
} cis_rw_t;

/** R_REG and W_REG struct passed through ioctl */
typedef struct {
	uint32	byteoff;	/**< byte offset of the field in d11regs_t */
	uint32	val;		/**< read/write value of the field */
	uint32	size;		/**< sizeof the field */
	uint32	band;		/**< band (optional) */
} rw_reg_t;

/**
 * Structure used by GET/SET_ATTEN ioctls - it controls power in b/g-band
 * PCL - Power Control Loop
 */
typedef struct {
	uint16	auto_ctrl;	/**< WL_ATTEN_XX */
	uint16	bb;		/**< Baseband attenuation */
	uint16	radio;		/**< Radio attenuation */
	uint16	txctl1;		/**< Radio TX_CTL1 value */
} atten_t;

/** Per-AC retry parameters */
struct wme_tx_params_s {
	uint8  short_retry;
	uint8  short_fallback;
	uint8  long_retry;
	uint8  long_fallback;
	uint16 max_rate;  /**< In units of 512 Kbps */
};

typedef struct wme_tx_params_s wme_tx_params_t;

#define WL_WME_TX_PARAMS_IO_BYTES (sizeof(wme_tx_params_t) * AC_COUNT)

/**Used to get specific link/ac parameters */
typedef struct {
	int32 ac;
	uint8 val;
	struct ether_addr ea;
	uint8 PAD;
} link_val_t;

#define WL_PM_MUTE_TX_VER 1

typedef struct wl_pm_mute_tx {
	uint16 version;		/**< version */
	uint16 len;		/**< length */
	uint16 deadline;	/**< deadline timer (in milliseconds) */
	uint8  enable;		/**< set to 1 to enable mode; set to 0 to disable it */
	uint8 PAD;
} wl_pm_mute_tx_t;

/*
 * Please update the following when modifying this structure:
 *   StaInfo Twiki page flags section - description of the sta_info_t struct
 *    src/wl/exe/wlu.c - print of sta_info_t
 * Pay attention to version if structure changes.
 */

/* sta_info_t version 4 */
typedef struct {
	uint16			ver;		/**< version of this struct */
	uint16			len;		/**< length in bytes of this structure */
	uint16			cap;		/**< sta's advertised capabilities */
	uint16			PAD;
	uint32			flags;		/**< flags defined below */
	uint32			idle;		/**< time since data pkt rx'd from sta */
	struct ether_addr	ea;		/**< Station address */
	uint16			PAD;
	wl_rateset_t	rateset;	/**< rateset in use */
	uint32			in;		/**< seconds elapsed since associated */
	uint32			listen_interval_inms; /**< Min Listen interval in ms for this STA */
	uint32			tx_pkts;	/**< # of user packets transmitted (unicast) */
	uint32			tx_failures;	/**< # of user packets failed */
	uint32			rx_ucast_pkts;	/**< # of unicast packets received */
	uint32			rx_mcast_pkts;	/**< # of multicast packets received */
	uint32			tx_rate;	/**< Rate used by last tx frame */
	uint32			rx_rate;	/**< Rate of last successful rx frame */
	uint32			rx_decrypt_succeeds;	/**< # of packet decrypted successfully */
	uint32			rx_decrypt_failures;	/**< # of packet decrypted unsuccessfully */
	uint32			tx_tot_pkts;	/**< # of user tx pkts (ucast + mcast) */
	uint32			rx_tot_pkts;	/**< # of data packets recvd (uni + mcast) */
	uint32			tx_mcast_pkts;	/**< # of mcast pkts txed */
	uint64			tx_tot_bytes;	/**< data bytes txed (ucast + mcast) */
	uint64			rx_tot_bytes;	/**< data bytes recvd (ucast + mcast) */
	uint64			tx_ucast_bytes;	/**< data bytes txed (ucast) */
	uint64			tx_mcast_bytes;	/**< # data bytes txed (mcast) */
	uint64			rx_ucast_bytes;	/**< data bytes recvd (ucast) */
	uint64			rx_mcast_bytes;	/**< data bytes recvd (mcast) */
	int8			rssi[WL_STA_ANT_MAX]; /**< average rssi per antenna
							* of data frames
							*/
	int8			nf[WL_STA_ANT_MAX];	/**< per antenna noise floor */
	uint16			aid;			/**< association ID */
	uint16			ht_capabilities;	/**< advertised ht caps */
	uint16			vht_flags;		/**< converted vht flags */
	uint16			PAD;
	uint32			tx_pkts_retried;	/**< # of frames where a retry was
							 * necessary
							 */
	uint32			tx_pkts_retry_exhausted; /**< # of user frames where a retry
							  * was exhausted
							  */
	int8			rx_lastpkt_rssi[WL_STA_ANT_MAX]; /**< Per antenna RSSI of last
								  * received data frame.
								  */
	/* TX WLAN retry/failure statistics:
	 * Separated for host requested frames and WLAN locally generated frames.
	 * Include unicast frame only where the retries/failures can be counted.
	 */
	uint32			tx_pkts_total;		/**< # user frames sent successfully */
	uint32			tx_pkts_retries;	/**< # user frames retries */
	uint32			tx_pkts_fw_total;	/**< # FW generated sent successfully */
	uint32			tx_pkts_fw_retries;	/**< # retries for FW generated frames */
	uint32			tx_pkts_fw_retry_exhausted;	/**< # FW generated where a retry
								 * was exhausted
								 */
	uint32			rx_pkts_retried;	/**< # rx with retry bit set */
	uint32			tx_rate_fallback;	/**< lowest fallback TX rate */
	/* Fields above this line are common to sta_info_t versions 4 and 5 */

	uint32			rx_dur_total;	/* total user RX duration (estimated) */

	chanspec_t		chanspec;       /** chanspec this sta is on */
	uint16			PAD;
	wl_rateset_args_v1_t	rateset_adv;	/* rateset along with mcs index bitmap */
	uint32			PAD;
} sta_info_v4_t;

/* Note: Version 4 is the latest version of sta_info_t.  Version 5 is abandoned.
 * Please add new fields to version 4, not version 5.
 */
/* sta_info_t version 5 */
typedef struct {
	uint16			ver;		/**< version of this struct */
	uint16			len;		/**< length in bytes of this structure */
	uint16			cap;		/**< sta's advertised capabilities */
	uint16			PAD;
	uint32			flags;		/**< flags defined below */
	uint32			idle;		/**< time since data pkt rx'd from sta */
	struct ether_addr	ea;		/**< Station address */
	uint16			PAD;
	wl_rateset_t		rateset;	/**< rateset in use */
	uint32			in;		/**< seconds elapsed since associated */
	uint32			listen_interval_inms; /**< Min Listen interval in ms for this STA */
	uint32			tx_pkts;	/**< # of user packets transmitted (unicast) */
	uint32			tx_failures;	/**< # of user packets failed */
	uint32			rx_ucast_pkts;	/**< # of unicast packets received */
	uint32			rx_mcast_pkts;	/**< # of multicast packets received */
	uint32			tx_rate;	/**< Rate used by last tx frame */
	uint32			rx_rate;	/**< Rate of last successful rx frame */
	uint32			rx_decrypt_succeeds;	/**< # of packet decrypted successfully */
	uint32			rx_decrypt_failures;	/**< # of packet decrypted unsuccessfully */
	uint32			tx_tot_pkts;	/**< # of user tx pkts (ucast + mcast) */
	uint32			rx_tot_pkts;	/**< # of data packets recvd (uni + mcast) */
	uint32			tx_mcast_pkts;	/**< # of mcast pkts txed */
	uint64			tx_tot_bytes;	/**< data bytes txed (ucast + mcast) */
	uint64			rx_tot_bytes;	/**< data bytes recvd (ucast + mcast) */
	uint64			tx_ucast_bytes;	/**< data bytes txed (ucast) */
	uint64			tx_mcast_bytes;	/**< # data bytes txed (mcast) */
	uint64			rx_ucast_bytes;	/**< data bytes recvd (ucast) */
	uint64			rx_mcast_bytes;	/**< data bytes recvd (mcast) */
	int8			rssi[WL_STA_ANT_MAX]; /**< average rssi per antenna
							* of data frames
							*/
	int8			nf[WL_STA_ANT_MAX];	/**< per antenna noise floor */
	uint16			aid;			/**< association ID */
	uint16			ht_capabilities;	/**< advertised ht caps */
	uint16			vht_flags;		/**< converted vht flags */
	uint16			PAD;
	uint32			tx_pkts_retried;	/**< # of frames where a retry was
							 * necessary
							 */
	uint32			tx_pkts_retry_exhausted; /**< # of user frames where a retry
							  * was exhausted
							  */
	int8			rx_lastpkt_rssi[WL_STA_ANT_MAX]; /**< Per antenna RSSI of last
								  * received data frame.
								  */
	/* TX WLAN retry/failure statistics:
	 * Separated for host requested frames and WLAN locally generated frames.
	 * Include unicast frame only where the retries/failures can be counted.
	 */
	uint32			tx_pkts_total;		/**< # user frames sent successfully */
	uint32			tx_pkts_retries;	/**< # user frames retries */
	uint32			tx_pkts_fw_total;	/**< # FW generated sent successfully */
	uint32			tx_pkts_fw_retries;	/**< # retries for FW generated frames */
	uint32			tx_pkts_fw_retry_exhausted;	/**< # FW generated where a retry
								 * was exhausted
								 */
	uint32			rx_pkts_retried;	/**< # rx with retry bit set */
	uint32			tx_rate_fallback;	/**< lowest fallback TX rate */
	/* Fields above this line are common to sta_info_t versions 4 and 5 */

	chanspec_t		chanspec;       /** chanspec this sta is on */
	uint16			PAD;
	wl_rateset_args_v1_t	rateset_adv;	/* rateset along with mcs index bitmap */
} sta_info_v5_t;

/*
 * Please update the following when modifying this structure:
 *   StaInfo Twiki page flags section - description of the sta_info_t struct
 *    src/wl/exe/wlu.c - print of sta_info_t
 * Pay attention to version if structure changes.
 */

/* sta_info_t version 6
	changes to wl_rateset_args_t is leading to update this struct version as well.
 */
typedef struct {
	uint16			ver;		/**< version of this struct */
	uint16			len;		/**< length in bytes of this structure */
	uint16			cap;		/**< sta's advertised capabilities */
	uint16			PAD;
	uint32			flags;		/**< flags defined below */
	uint32			idle;		/**< time since data pkt rx'd from sta */
	struct ether_addr	ea;		/**< Station address */
	uint16			PAD;
	wl_rateset_t	rateset;	/**< rateset in use */
	uint32			in;		/**< seconds elapsed since associated */
	uint32			listen_interval_inms; /**< Min Listen interval in ms for this STA */
	uint32			tx_pkts;	/**< # of user packets transmitted (unicast) */
	uint32			tx_failures;	/**< # of user packets failed */
	uint32			rx_ucast_pkts;	/**< # of unicast packets received */
	uint32			rx_mcast_pkts;	/**< # of multicast packets received */
	uint32			tx_rate;	/**< Rate used by last tx frame */
	uint32			rx_rate;	/**< Rate of last successful rx frame */
	uint32			rx_decrypt_succeeds;	/**< # of packet decrypted successfully */
	uint32			rx_decrypt_failures;	/**< # of packet decrypted unsuccessfully */
	uint32			tx_tot_pkts;	/**< # of user tx pkts (ucast + mcast) */
	uint32			rx_tot_pkts;	/**< # of data packets recvd (uni + mcast) */
	uint32			tx_mcast_pkts;	/**< # of mcast pkts txed */
	uint64			tx_tot_bytes;	/**< data bytes txed (ucast + mcast) */
	uint64			rx_tot_bytes;	/**< data bytes recvd (ucast + mcast) */
	uint64			tx_ucast_bytes;	/**< data bytes txed (ucast) */
	uint64			tx_mcast_bytes;	/**< # data bytes txed (mcast) */
	uint64			rx_ucast_bytes;	/**< data bytes recvd (ucast) */
	uint64			rx_mcast_bytes;	/**< data bytes recvd (mcast) */
	int8			rssi[WL_STA_ANT_MAX]; /**< average rssi per antenna
							* of data frames
							*/
	int8			nf[WL_STA_ANT_MAX];	/**< per antenna noise floor */
	uint16			aid;			/**< association ID */
	uint16			ht_capabilities;	/**< advertised ht caps */
	uint16			vht_flags;		/**< converted vht flags */
	uint16			PAD;
	uint32			tx_pkts_retried;	/**< # of frames where a retry was
							 * necessary
							 */
	uint32			tx_pkts_retry_exhausted; /**< # of user frames where a retry
							  * was exhausted
							  */
	int8			rx_lastpkt_rssi[WL_STA_ANT_MAX]; /**< Per antenna RSSI of last
								  * received data frame.
								  */
	/* TX WLAN retry/failure statistics:
	 * Separated for host requested frames and WLAN locally generated frames.
	 * Include unicast frame only where the retries/failures can be counted.
	 */
	uint32			tx_pkts_total;		/**< # user frames sent successfully */
	uint32			tx_pkts_retries;	/**< # user frames retries */
	uint32			tx_pkts_fw_total;	/**< # FW generated sent successfully */
	uint32			tx_pkts_fw_retries;	/**< # retries for FW generated frames */
	uint32			tx_pkts_fw_retry_exhausted;	/**< # FW generated where a retry
								 * was exhausted
								 */
	uint32			rx_pkts_retried;	/**< # rx with retry bit set */
	uint32			tx_rate_fallback;	/**< lowest fallback TX rate */
	/* Fields above this line are common to sta_info_t versions 4 and 5 */

	uint32			rx_dur_total;	/* total user RX duration (estimated) */

	chanspec_t		chanspec;       /** chanspec this sta is on */
	uint16			PAD;
	wl_rateset_args_v2_t	rateset_adv;	/* rateset along with mcs index bitmap */
} sta_info_v6_t;

/* define to help support one version older sta_info_t from user level
 * applications.
 */
#define WL_OLD_STAINFO_SIZE	OFFSETOF(sta_info_t, tx_tot_pkts)

#define WL_STA_VER_4		4
#define WL_STA_VER_5		5
#define WL_STA_VER		WL_STA_VER_4

#define SWDIV_STATS_VERSION_2 2
#define SWDIV_STATS_CURRENT_VERSION SWDIV_STATS_VERSION_2

struct wlc_swdiv_stats_v1 {
	uint32 auto_en;
	uint32 active_ant;
	uint32 rxcount;
	int32 avg_snr_per_ant0;
	int32 avg_snr_per_ant1;
	int32 avg_snr_per_ant2;
	uint32 swap_ge_rxcount0;
	uint32 swap_ge_rxcount1;
	uint32 swap_ge_snrthresh0;
	uint32 swap_ge_snrthresh1;
	uint32 swap_txfail0;
	uint32 swap_txfail1;
	uint32 swap_timer0;
	uint32 swap_timer1;
	uint32 swap_alivecheck0;
	uint32 swap_alivecheck1;
	uint32 rxcount_per_ant0;
	uint32 rxcount_per_ant1;
	uint32 acc_rxcount;
	uint32 acc_rxcount_per_ant0;
	uint32 acc_rxcount_per_ant1;
	uint32 tx_auto_en;
	uint32 tx_active_ant;
	uint32 rx_policy;
	uint32 tx_policy;
	uint32 cell_policy;
	uint32 swap_snrdrop0;
	uint32 swap_snrdrop1;
	uint32 mws_antsel_ovr_tx;
	uint32 mws_antsel_ovr_rx;
	uint8 swap_trig_event_id;
};

struct wlc_swdiv_stats_v2 {
	uint16	version;	/* version of the structure
						* as defined by SWDIV_STATS_CURRENT_VERSION
						*/
	uint16	length;		/* length of the entire structure */
	uint32 auto_en;
	uint32 active_ant;
	uint32 rxcount;
	int32 avg_snr_per_ant0;
	int32 avg_snr_per_ant1;
	int32 avg_snr_per_ant2;
	uint32 swap_ge_rxcount0;
	uint32 swap_ge_rxcount1;
	uint32 swap_ge_snrthresh0;
	uint32 swap_ge_snrthresh1;
	uint32 swap_txfail0;
	uint32 swap_txfail1;
	uint32 swap_timer0;
	uint32 swap_timer1;
	uint32 swap_alivecheck0;
	uint32 swap_alivecheck1;
	uint32 rxcount_per_ant0;
	uint32 rxcount_per_ant1;
	uint32 acc_rxcount;
	uint32 acc_rxcount_per_ant0;
	uint32 acc_rxcount_per_ant1;
	uint32 tx_auto_en;
	uint32 tx_active_ant;
	uint32 rx_policy;
	uint32 tx_policy;
	uint32 cell_policy;
	uint32 swap_snrdrop0;
	uint32 swap_snrdrop1;
	uint32 mws_antsel_ovr_tx;
	uint32 mws_antsel_ovr_rx;
	uint32 swap_trig_event_id;
};

#define	WLC_NUMRATES	16	/**< max # of rates in a rateset */

/**Used to get specific STA parameters */
typedef struct {
	uint32	val;
	struct ether_addr ea;
	uint16	PAD;
} scb_val_t;

/**Used by iovar versions of some ioctls, i.e. WLC_SCB_AUTHORIZE et al */
typedef struct {
	uint32 code;
	scb_val_t ioctl_args;
} authops_t;

/** channel encoding */
typedef struct channel_info {
	int32 hw_channel;
	int32 target_channel;
	int32 scan_channel;
} channel_info_t;

/** For ioctls that take a list of MAC addresses */
typedef struct maclist {
	uint32 count;			/**< number of MAC addresses */
	struct ether_addr ea[1];	/**< variable length array of MAC addresses */
} maclist_t;

typedef struct wds_client_info {
	char	ifname[INTF_NAME_SIZ];	/* WDS ifname */
	struct	ether_addr ea;		/* WDS client MAC address */
} wds_client_info_t;

#define WDS_MACLIST_MAGIC	0xFFFFFFFF
#define WDS_MACLIST_VERSION	1

/* For wds MAC list ioctls */
typedef struct wds_maclist {
	uint32 count;				/* Number of WDS clients */
	uint32 magic;				/* Magic number */
	uint32 version;				/* Version number */
	struct wds_client_info  client_list[1]; /* Variable length array of WDS clients */
} wds_maclist_t;

/**get pkt count struct passed through ioctl */
typedef struct get_pktcnt {
	uint32 rx_good_pkt;
	uint32 rx_bad_pkt;
	uint32 tx_good_pkt;
	uint32 tx_bad_pkt;
	uint32 rx_ocast_good_pkt; /**< unicast packets destined for others */
} get_pktcnt_t;

/* NINTENDO2 */
#define LQ_IDX_MIN              0
#define LQ_IDX_MAX              1
#define LQ_IDX_AVG              2
#define LQ_IDX_SUM              2
#define LQ_IDX_LAST             3
#define LQ_STOP_MONITOR         0
#define LQ_START_MONITOR        1

/** Get averages RSSI, Rx PHY rate and SNR values */
/* Link Quality */
typedef struct {
	int32 rssi[LQ_IDX_LAST];  /**< Array to keep min, max, avg rssi */
	int32 snr[LQ_IDX_LAST];   /**< Array to keep min, max, avg snr */
	int32 isvalid;            /**< Flag indicating whether above data is valid */
} wl_lq_t;

typedef enum wl_wakeup_reason_type {
	LCD_ON = 1,
	LCD_OFF,
	DRC1_WAKE,
	DRC2_WAKE,
	REASON_LAST
} wl_wr_type_t;

typedef struct {
	/** Unique filter id */
	uint32	id;
	/** stores the reason for the last wake up */
	uint8	reason;
	uint8	PAD[3];
} wl_wr_t;

/** Get MAC specific rate histogram command */
typedef struct {
	struct	ether_addr ea;	/**< MAC Address */
	uint8	ac_cat;	/**< Access Category */
	uint8	num_pkts;	/**< Number of packet entries to be averaged */
} wl_mac_ratehisto_cmd_t;
/** Get MAC rate histogram response */
/* deprecated after JAGUAR branch */
typedef struct {
	uint32	rate[DOT11_RATE_MAX + 1];	/**< Rates */
	uint32	mcs[WL_RATESET_SZ_HT_IOCTL * WL_TX_CHAINS_MAX];	/**< MCS counts */
	uint32	vht[WL_RATESET_SZ_VHT_MCS][WL_TX_CHAINS_MAX];	/**< VHT counts */
	uint32	tsf_timer[2][2];	/**< Start and End time for 8bytes value */
	uint32	prop11n_mcs[WLC_11N_LAST_PROP_MCS - WLC_11N_FIRST_PROP_MCS + 1]; /** MCS counts */
} wl_mac_ratehisto_res_t;

/* sta_info ecounters */
typedef struct {
	struct ether_addr   ea;				/* Station MAC addr */
	struct ether_addr   BSSID;			/* BSSID of the BSS */
	uint32              tx_pkts_fw_total;		/* # FW generated sent successfully */
	uint32              tx_pkts_fw_retries;		/* # retries for FW generated frames */
	uint32              tx_pkts_fw_retry_exhausted;	/* # FW generated which
							 * failed after retry
							 */
} sta_info_ecounters_t;

#define STAMON_MODULE_VER		1

/**Linux network driver ioctl encoding */
typedef struct wl_ioctl {
	uint32 cmd;	/**< common ioctl definition */
	void *buf;	/**< pointer to user buffer */
	uint32 len;	/**< length of user buffer */
	uint8 set;		/**< 1=set IOCTL; 0=query IOCTL */
	uint32 used;	/**< bytes read or written (optional) */
	uint32 needed;	/**< bytes needed (optional) */
} wl_ioctl_t;

#ifdef CONFIG_COMPAT
typedef struct compat_wl_ioctl {
	uint32 cmd;	/**< common ioctl definition */
	uint32 buf;	/**< pointer to user buffer */
	uint32 len;	/**< length of user buffer */
	uint8 set;		/**< 1=set IOCTL; 0=query IOCTL */
	uint32 used;	/**< bytes read or written (optional) */
	uint32 needed;	/**< bytes needed (optional) */
} compat_wl_ioctl_t;
#endif /* CONFIG_COMPAT */

#define WL_NUM_RATES_CCK		4 /**< 1, 2, 5.5, 11 Mbps */
#define WL_NUM_RATES_OFDM		8 /**< 6, 9, 12, 18, 24, 36, 48, 54 Mbps SISO/CDD */
#define WL_NUM_RATES_MCS_1STREAM	8 /**< MCS 0-7 1-stream rates - SISO/CDD/STBC/MCS */
#define WL_NUM_RATES_EXTRA_VHT		2 /**< Additional VHT 11AC rates */
#define WL_NUM_RATES_VHT		10
#define WL_NUM_RATES_VHT_ALL		(WL_NUM_RATES_VHT + WL_NUM_RATES_EXTRA_VHT)
#define WL_NUM_RATES_HE			12
#define WL_NUM_RATES_MCS32		1
#define UC_PATH_LEN			128u /**< uCode path length */

/*
 * Structure for passing hardware and software
 * revision info up from the driver.
 */
typedef struct wlc_rev_info {
	uint32		vendorid;	/**< PCI vendor id */
	uint32		deviceid;	/**< device id of chip */
	uint32		radiorev;	/**< radio revision */
	uint32		chiprev;	/**< chip revision */
	uint32		corerev;	/**< core revision */
	uint32		boardid;	/**< board identifier (usu. PCI sub-device id) */
	uint32		boardvendor;	/**< board vendor (usu. PCI sub-vendor id) */
	uint32		boardrev;	/**< board revision */
	uint32		driverrev;	/**< driver version */
	uint32		ucoderev;	/**< uCode version */
	uint32		bus;		/**< bus type */
	uint32		chipnum;	/**< chip number */
	uint32		phytype;	/**< phy type */
	uint32		phyrev;		/**< phy revision */
	uint32		anarev;		/**< anacore rev */
	uint32		chippkg;	/**< chip package info */
	uint32		nvramrev;	/**< nvram revision number */
	uint32		phyminorrev;	/**< phy minor rev */
	uint32		coreminorrev;	/**< core minor rev */
	uint32		drvrev_major;	/**< driver version: major */
	uint32		drvrev_minor;	/**< driver version: minor */
	uint32		drvrev_rc;	/**< driver version: rc */
	uint32		drvrev_rc_inc;	/**< driver version: rc incremental */
	uint16		ucodeprebuilt;	/**< uCode prebuilt flag */
	uint16		ucodediffct;	/**< uCode diff count */
	uchar		ucodeurl[128u]; /* obsolete, kept for ROM compatiblity */
	uchar		ucodepath[UC_PATH_LEN]; /**< uCode URL or path */
} wlc_rev_info_t;

#define WL_REV_INFO_LEGACY_LENGTH	48

#define WL_BRAND_MAX 10
typedef struct wl_instance_info {
	uint32 instance;
	int8 brand[WL_BRAND_MAX];
	int8 PAD[4-(WL_BRAND_MAX%4)];
} wl_instance_info_t;

/** structure to change size of tx fifo */
typedef struct wl_txfifo_sz {
	uint16	magic;
	uint16	fifo;
	uint16	size;
} wl_txfifo_sz_t;

/* Transfer info about an IOVar from the driver */
/**Max supported IOV name size in bytes, + 1 for nul termination */
#define WLC_IOV_NAME_LEN	(32 + 1)

typedef struct wlc_iov_trx_s {
	uint8 module;
	uint8 type;
	char name[WLC_IOV_NAME_LEN];
} wlc_iov_trx_t;

/** bump this number if you change the ioctl interface */
#define WLC_IOCTL_VERSION	2
#define WLC_IOCTL_VERSION_LEGACY_IOTYPES	1
/* ifdef EXT_STA */
typedef struct _wl_assoc_result {
	ulong associated;
	ulong NDIS_auth;
	ulong NDIS_infra;
} wl_assoc_result_t;
/* EXT_STA */

#define WL_PHY_PAVARS_LEN	32	/**< Phytype, Bandrange, chain, a[0], b[0], c[0], d[0] .. */

#define WL_PHY_PAVAR_VER	1	/**< pavars version */
#define WL_PHY_PAVARS2_NUM	3	/**< a1, b0, b1 */
typedef struct wl_pavars2 {
	uint16 ver;		/**< version of this struct */
	uint16 len;		/**< len of this structure */
	uint16 inuse;		/**< driver return 1 for a1,b0,b1 in current band range */
	uint16 phy_type;	/**< phy type */
	uint16 bandrange;
	uint16 chain;
	uint16 inpa[WL_PHY_PAVARS2_NUM];	/**< phy pavars for one band range */
} wl_pavars2_t;

typedef struct wl_po {
	uint16	phy_type;	/**< Phy type */
	uint16	band;
	uint16	cckpo;
	uint16	PAD;
	uint32	ofdmpo;
	uint16	mcspo[8];
} wl_po_t;

#define WL_NUM_RPCALVARS 5	/**< number of rpcal vars */

typedef struct wl_rpcal {
	uint16 value;
	uint16 update;
} wl_rpcal_t;

#define WL_NUM_RPCALPHASEVARS 5	/* number of rpcal phase vars */

typedef struct wl_rpcal_phase {
	uint16 value;
	uint16 update;
} wl_rpcal_phase_t;

typedef struct wl_aci_args {
	int32 enter_aci_thresh; /* Trigger level to start detecting ACI */
	int32 exit_aci_thresh; /* Trigger level to exit ACI mode */
	int32 usec_spin; /* microsecs to delay between rssi samples */
	int32 glitch_delay; /* interval between ACI scans when glitch count is consistently high */
	uint16 nphy_adcpwr_enter_thresh;	/**< ADC power to enter ACI mitigation mode */
	uint16 nphy_adcpwr_exit_thresh;	/**< ADC power to exit ACI mitigation mode */
	uint16 nphy_repeat_ctr;		/**< Number of tries per channel to compute power */
	uint16 nphy_num_samples;	/**< Number of samples to compute power on one channel */
	uint16 nphy_undetect_window_sz;	/**< num of undetects to exit ACI Mitigation mode */
	uint16 nphy_b_energy_lo_aci;	/**< low ACI power energy threshold for bphy */
	uint16 nphy_b_energy_md_aci;	/**< mid ACI power energy threshold for bphy */
	uint16 nphy_b_energy_hi_aci;	/**< high ACI power energy threshold for bphy */
	uint16 nphy_noise_noassoc_glitch_th_up; /**< wl interference 4 */
	uint16 nphy_noise_noassoc_glitch_th_dn;
	uint16 nphy_noise_assoc_glitch_th_up;
	uint16 nphy_noise_assoc_glitch_th_dn;
	uint16 nphy_noise_assoc_aci_glitch_th_up;
	uint16 nphy_noise_assoc_aci_glitch_th_dn;
	uint16 nphy_noise_assoc_enter_th;
	uint16 nphy_noise_noassoc_enter_th;
	uint16 nphy_noise_assoc_rx_glitch_badplcp_enter_th;
	uint16 nphy_noise_noassoc_crsidx_incr;
	uint16 nphy_noise_assoc_crsidx_incr;
	uint16 nphy_noise_crsidx_decr;
} wl_aci_args_t;

#define WL_ACI_ARGS_LEGACY_LENGTH	16	/**< bytes of pre NPHY aci args */

#define	WL_MACFIFO_PLAY_ARGS_T_VERSION	1u	/* version of wl_macfifo_play_args_t struct */

enum wl_macfifo_play_flags {
	WL_MACFIFO_PLAY_STOP =		0x00u,	/* stop playing samples */
	WL_MACFIFO_PLAY_START =		0x01u,	/* start playing samples */
	WL_MACFIFO_PLAY_LOAD =		0x02u,	/* for set: load samples
						   for get: samples are loaded
						 */
	WL_MACFIFO_PLAY_GET_MAX_SIZE =	0x10u,	/* get the macfifo buffer size */
	WL_MACFIFO_PLAY_GET_STATUS =	0x20u,	/* get macfifo play status */
};

typedef struct wl_macfifo_play_args {
	uint16 version;		/* structure version */
	uint16 len;		/* size of structure */
	uint16 flags;
	uint8 PAD[2];
	uint32 data_len;	/* data length */
} wl_macfifo_play_args_t;

#define	WL_MACFIFO_PLAY_DATA_T_VERSION	1u	/* version of wl_macfifo_play_data_t struct */

typedef struct wl_macfifo_play_data {
	uint16 version;		/* structure version */
	uint16 len;		/* size of structure */
	uint32 data_len;	/* data length */
} wl_macfifo_play_data_t;

#define	WL_SAMPLECOLLECT_T_VERSION	2	/**< version of wl_samplecollect_args_t struct */
typedef struct wl_samplecollect_args {
	/* version 0 fields */
	uint8 coll_us;
	uint8 PAD[3];
	int32 cores;
	/* add'l version 1 fields */
	uint16 version;     /**< see definition of WL_SAMPLECOLLECT_T_VERSION */
	uint16 length;      /**< length of entire structure */
	int8 trigger;
	uint8 PAD;
	uint16 timeout;
	uint16 mode;
	uint16 PAD;
	uint32 pre_dur;
	uint32 post_dur;
	uint8 gpio_sel;
	uint8 downsamp;
	uint8 be_deaf;
	uint8 agc;		/**< loop from init gain and going down */
	uint8 filter;		/**< override high pass corners to lowest */
	/* add'l version 2 fields */
	uint8 trigger_state;
	uint8 module_sel1;
	uint8 module_sel2;
	uint16 nsamps;
	uint16 PAD;
	int32 bitStart;
	uint32 gpioCapMask;
	uint8 gpio_collection;
	uint8 PAD[3];
} wl_samplecollect_args_t;

#define	WL_SAMPLEDATA_T_VERSION		1	/**< version of wl_samplecollect_args_t struct */
/* version for unpacked sample data, int16 {(I,Q),Core(0..N)} */
#define	WL_SAMPLEDATA_T_VERSION_SPEC_AN 2

typedef struct wl_sampledata {
	uint16 version;	/**< structure version */
	uint16 size;	/**< size of structure */
	uint16 tag;	/**< Header/Data */
	uint16 length;	/**< data length */
	uint32 flag;	/**< bit def */
} wl_sampledata_t;

/* WL_OTA START */
/* OTA Test Status */
enum {
	WL_OTA_TEST_IDLE = 0,		/**< Default Idle state */
	WL_OTA_TEST_ACTIVE = 1,		/**< Test Running */
	WL_OTA_TEST_SUCCESS = 2,	/**< Successfully Finished Test */
	WL_OTA_TEST_FAIL = 3		/**< Test Failed in the Middle */
};

/* OTA SYNC Status */
enum {
	WL_OTA_SYNC_IDLE = 0,	/**< Idle state */
	WL_OTA_SYNC_ACTIVE = 1,	/**< Waiting for Sync */
	WL_OTA_SYNC_FAIL = 2	/**< Sync pkt not recieved */
};

/* Various error states dut can get stuck during test */
enum {
	WL_OTA_SKIP_TEST_CAL_FAIL = 1,		/**< Phy calibration failed */
	WL_OTA_SKIP_TEST_SYNCH_FAIL = 2,	/**< Sync Packet not recieved */
	WL_OTA_SKIP_TEST_FILE_DWNLD_FAIL = 3,	/**< Cmd flow file download failed */
	WL_OTA_SKIP_TEST_NO_TEST_FOUND = 4,	/**< No test found in Flow file */
	WL_OTA_SKIP_TEST_WL_NOT_UP = 5,		/**< WL UP failed */
	WL_OTA_SKIP_TEST_UNKNOWN_CALL		/**< Unintentional scheduling on ota test */
};

/* Differentiator for ota_tx and ota_rx */
enum {
	WL_OTA_TEST_TX = 0,		/**< ota_tx */
	WL_OTA_TEST_RX = 1,		/**< ota_rx */
};

/* Catch 3 modes of operation: 20Mhz, 40Mhz, 20 in 40 Mhz */
enum {
	WL_OTA_TEST_BW_20_IN_40MHZ = 0,		/**< 20 in 40 operation */
	WL_OTA_TEST_BW_20MHZ = 1,		/**< 20 Mhz operation */
	WL_OTA_TEST_BW_40MHZ = 2,		/**< full 40Mhz operation */
	WL_OTA_TEST_BW_80MHZ = 3		/* full 80Mhz operation */
};
#define HT_MCS_INUSE	0x00000080	/* HT MCS in use,indicates b0-6 holds an mcs */
#define VHT_MCS_INUSE	0x00000100	/* VHT MCS in use,indicates b0-6 holds an mcs */
#define OTA_RATE_MASK 0x0000007f	/* rate/mcs value */
#define OTA_STF_SISO	0
#define OTA_STF_CDD		1
#define OTA_STF_STBC	2
#define OTA_STF_SDM		3

typedef struct ota_rate_info {
	uint8 rate_cnt;					/**< Total number of rates */
	uint8 PAD;
	uint16 rate_val_mbps[WL_OTA_TEST_MAX_NUM_RATE];	/**< array of rates from 1mbps to 130mbps */
							/**< for legacy rates : ratein mbps * 2 */
							/**< for HT rates : mcs index */
} ota_rate_info_t;

typedef struct ota_power_info {
	int8 pwr_ctrl_on;	/**< power control on/off */
	int8 start_pwr;		/**< starting power/index */
	int8 delta_pwr;		/**< delta power/index */
	int8 end_pwr;		/**< end power/index */
} ota_power_info_t;

typedef struct ota_packetengine {
	uint16 delay;           /**< Inter-packet delay */
				/**< for ota_tx, delay is tx ifs in micro seconds */
				/* for ota_rx, delay is wait time in milliseconds */
	uint16 nframes;         /**< Number of frames */
	uint16 length;          /**< Packet length */
} ota_packetengine_t;

/*
 * OTA txant/rxant parameter
 *    bit7-4: 4 bits swdiv_tx/rx_policy bitmask, specify antenna-policy for SW diversity
 *    bit3-0: 4 bits TxCore bitmask, specify cores used for transmit frames
 *            (maximum spatial expansion)
 */
#define WL_OTA_TEST_ANT_MASK	0xF0
#define WL_OTA_TEST_CORE_MASK	0x0F

/* OTA txant/rxant 'ant_mask' field; map to Tx/Rx antenna policy for SW diversity */
enum {
	WL_OTA_TEST_FORCE_ANT0 = 0x10,	/* force antenna to Ant 0 */
	WL_OTA_TEST_FORCE_ANT1 = 0x20,	/* force antenna to Ant 1 */
};

/* antenna/core fields access */
#define WL_OTA_TEST_GET_ANT(_txant) ((_txant) & WL_OTA_TEST_ANT_MASK)
#define WL_OTA_TEST_GET_CORE(_txant) ((_txant) & WL_OTA_TEST_CORE_MASK)

/** Test info vector */
typedef struct wl_ota_test_args {
	uint8 cur_test;			/**< test phase */
	uint8 chan;			/**< channel */
	uint8 bw;			/**< bandwidth */
	uint8 control_band;		/**< control band */
	uint8 stf_mode;			/**< stf mode */
	uint8 PAD;
	ota_rate_info_t rt_info;	/**< Rate info */
	ota_packetengine_t pkteng;	/**< packeteng info */
	uint8 txant;			/**< tx antenna */
	uint8 rxant;			/**< rx antenna */
	ota_power_info_t pwr_info;	/**< power sweep info */
	uint8 wait_for_sync;		/**< wait for sync or not */
	uint8 ldpc;
	uint8 sgi;
	uint8 PAD;
	/* Update WL_OTA_TESTVEC_T_VERSION for adding new members to this structure */
} wl_ota_test_args_t;

#define WL_OTA_TESTVEC_T_VERSION		1	/* version of wl_ota_test_vector_t struct */
typedef struct wl_ota_test_vector {
	uint16 version;
	wl_ota_test_args_t test_arg[WL_OTA_TEST_MAX_NUM_SEQ];	/**< Test argument struct */
	uint16 test_cnt;					/**< Total no of test */
	uint8 file_dwnld_valid;					/**< File successfully downloaded */
	uint8 sync_timeout;					/**< sync packet timeout */
	int8 sync_fail_action;					/**< sync fail action */
	struct ether_addr sync_mac;				/**< macaddress for sync pkt */
	struct ether_addr tx_mac;				/**< macaddress for tx */
	struct ether_addr rx_mac;				/**< macaddress for rx */
	int8 loop_test;					/**< dbg feature to loop the test */
	uint16 test_rxcnt;
	/* Update WL_OTA_TESTVEC_T_VERSION for adding new members to this structure */
} wl_ota_test_vector_t;

/** struct copied back form dongle to host to query the status */
typedef struct wl_ota_test_status {
	int16 cur_test_cnt;		/**< test phase */
	int8 skip_test_reason;		/**< skip test reasoin */
	uint8 PAD;
	wl_ota_test_args_t test_arg;	/**< cur test arg details */
	uint16 test_cnt;		/**< total no of test downloaded */
	uint8 file_dwnld_valid;		/**< file successfully downloaded ? */
	uint8 sync_timeout;		/**< sync timeout */
	int8 sync_fail_action;		/**< sync fail action */
	struct ether_addr sync_mac;	/**< macaddress for sync pkt */
	struct ether_addr tx_mac;	/**< tx mac address */
	struct ether_addr rx_mac;	/**< rx mac address */
	uint8  test_stage;		/**< check the test status */
	int8 loop_test;			/**< Debug feature to puts test enfine in a loop */
	uint8 sync_status;		/**< sync status */
} wl_ota_test_status_t;

/* FOR ioctl that take the sta monitor information */
typedef struct stamon_data {
	struct ether_addr  ea;
	uint8 PAD[2];
	int32 rssi;
} stamon_data_t;

typedef struct stamon_info {
	int32 version;
	uint32 count;
	stamon_data_t sta_data[1];
} stamon_info_t;

typedef struct wl_ota_rx_rssi {
	uint16	pktcnt;		/* Pkt count used for this rx test */
	chanspec_t chanspec;	/* Channel info on which the packets are received */
	int16	rssi;		/* Average RSSI of the first 50% packets received */
} wl_ota_rx_rssi_t;

#define	WL_OTARSSI_T_VERSION		1	/* version of wl_ota_test_rssi_t struct */
#define WL_OTA_TEST_RSSI_FIXED_SIZE	OFFSETOF(wl_ota_test_rssi_t, rx_rssi)

typedef struct wl_ota_test_rssi {
	uint8 version;
	uint8	testcnt;		/* total measured RSSI values, valid on output only */
	wl_ota_rx_rssi_t rx_rssi[1]; /* Variable length array of wl_ota_rx_rssi_t */
} wl_ota_test_rssi_t;

/* WL_OTA END */

/**wl_radar_args_t */
typedef struct {
	int32 npulses;	/**< required number of pulses at n * t_int */
	int32 ncontig;	/**< required number of pulses at t_int */
	int32 min_pw;	/**< minimum pulse width (20 MHz clocks) */
	int32 max_pw;	/**< maximum pulse width (20 MHz clocks) */
	uint16 thresh0;	/**< Radar detection, thresh 0 */
	uint16 thresh1;	/**< Radar detection, thresh 1 */
	uint16 blank;	/**< Radar detection, blank control */
	uint16 fmdemodcfg;	/**< Radar detection, fmdemod config */
	int32 npulses_lp;  /**< Radar detection, minimum long pulses */
	int32 min_pw_lp; /**< Minimum pulsewidth for long pulses */
	int32 max_pw_lp; /**< Maximum pulsewidth for long pulses */
	int32 min_fm_lp; /**< Minimum fm for long pulses */
	int32 max_span_lp;  /**< Maximum deltat for long pulses */
	int32 min_deltat; /**< Minimum spacing between pulses */
	int32 max_deltat; /**< Maximum spacing between pulses */
	uint16 autocorr;	/**< Radar detection, autocorr on or off */
	uint16 st_level_time;	/**< Radar detection, start_timing level */
	uint16 t2_min; /**< minimum clocks needed to remain in state 2 */
	uint8 PAD[2];
	uint32 version; /**< version */
	uint32 fra_pulse_err;	/**< sample error margin for detecting French radar pulsed */
	int32 npulses_fra;  /**< Radar detection, minimum French pulses set */
	int32 npulses_stg2;  /**< Radar detection, minimum staggered-2 pulses set */
	int32 npulses_stg3;  /**< Radar detection, minimum staggered-3 pulses set */
	uint16 percal_mask;	/**< defines which period cal is masked from radar detection */
	uint8 PAD[2];
	int32 quant;	/**< quantization resolution to pulse positions */
	uint32 min_burst_intv_lp;	/**< minimum burst to burst interval for bin3 radar */
	uint32 max_burst_intv_lp;	/**< maximum burst to burst interval for bin3 radar */
	int32 nskip_rst_lp;	/**< number of skipped pulses before resetting lp buffer */
	int32 max_pw_tol; /* maximum tolerance allowd in detected pulse width for radar detection */
	uint16 feature_mask; /**< 16-bit mask to specify enabled features */
	uint16 thresh0_sc;	/**< Radar detection, thresh 0 */
	uint16 thresh1_sc;	/**< Radar detection, thresh 1 */
	uint8 PAD[2];
} wl_radar_args_t;

#define WL_RADAR_ARGS_VERSION 2

typedef struct {
	uint32 version; /**< version */
	uint16 thresh0_20_lo;	/**< Radar detection, thresh 0 (range 5250-5350MHz) for BW 20MHz */
	uint16 thresh1_20_lo;	/**< Radar detection, thresh 1 (range 5250-5350MHz) for BW 20MHz */
	uint16 thresh0_40_lo;	/**< Radar detection, thresh 0 (range 5250-5350MHz) for BW 40MHz */
	uint16 thresh1_40_lo;	/**< Radar detection, thresh 1 (range 5250-5350MHz) for BW 40MHz */
	uint16 thresh0_80_lo;	/**< Radar detection, thresh 0 (range 5250-5350MHz) for BW 80MHz */
	uint16 thresh1_80_lo;	/**< Radar detection, thresh 1 (range 5250-5350MHz) for BW 80MHz */
	uint16 thresh0_20_hi;	/**< Radar detection, thresh 0 (range 5470-5725MHz) for BW 20MHz */
	uint16 thresh1_20_hi;	/**< Radar detection, thresh 1 (range 5470-5725MHz) for BW 20MHz */
	uint16 thresh0_40_hi;	/**< Radar detection, thresh 0 (range 5470-5725MHz) for BW 40MHz */
	uint16 thresh1_40_hi;	/**< Radar detection, thresh 1 (range 5470-5725MHz) for BW 40MHz */
	uint16 thresh0_80_hi;	/**< Radar detection, thresh 0 (range 5470-5725MHz) for BW 80MHz */
	uint16 thresh1_80_hi;	/**< Radar detection, thresh 1 (range 5470-5725MHz) for BW 80MHz */
	uint16 thresh0_160_lo;	/**< Radar detection, thresh 0 (range 5250-5350MHz) for BW 160MHz */
	uint16 thresh1_160_lo;	/**< Radar detection, thresh 1 (range 5250-5350MHz) for BW 160MHz */
	uint16 thresh0_160_hi;	/**< Radar detection, thresh 0 (range 5470-5725MHz) for BW 160MHz */
	uint16 thresh1_160_hi;	/**< Radar detection, thresh 1 (range 5470-5725MHz) for BW 160MHz */
} wl_radar_thr_t;

typedef struct {
	uint32 version; /* version */
	uint16 thresh0_sc_20_lo;
	uint16 thresh1_sc_20_lo;
	uint16 thresh0_sc_40_lo;
	uint16 thresh1_sc_40_lo;
	uint16 thresh0_sc_80_lo;
	uint16 thresh1_sc_80_lo;
	uint16 thresh0_sc_20_hi;
	uint16 thresh1_sc_20_hi;
	uint16 thresh0_sc_40_hi;
	uint16 thresh1_sc_40_hi;
	uint16 thresh0_sc_80_hi;
	uint16 thresh1_sc_80_hi;
	uint16 fc_varth_sb;
	uint16 fc_varth_bin5_sb;
	uint16 notradar_enb;
	uint16 max_notradar_lp;
	uint16 max_notradar;
	uint16 max_notradar_lp_sc;
	uint16 max_notradar_sc;
	uint16 highpow_war_enb;
	uint16 highpow_sp_ratio;	//unit is 0.5
} wl_radar_thr2_t;

#define WL_RADAR_THR_VERSION	2

typedef struct {
	uint32 ver;
	uint32 len;
	int32  rssi_th[3];
	uint8  rssi_gain_80[4];
	uint8  rssi_gain_160[4];
} wl_dyn_switch_th_t;

#define WL_PHY_DYN_SWITCH_TH_VERSION	1

/** RSSI per antenna */
typedef struct {
	uint32	version;		/**< version field */
	uint32	count;			/**< number of valid antenna rssi */
	int8 rssi_ant[WL_RSSI_ANT_MAX];	/**< rssi per antenna */
	int8 rssi_sum;			/**< summed rssi across all antennas */
	int8 PAD[3];
} wl_rssi_ant_t;

/* SNR per antenna */
typedef struct {
	uint32  version;				/* version field */
	uint32  count;					/* number of valid antenna snr */
	int8 snr_ant[WL_RSSI_ANT_MAX];	/* snr per antenna */
} wl_snr_ant_t;

/* Weighted average support */
#define	WL_WA_VER		0	/* Initial version - Basic WA algorithm only */

#define WL_WA_ALGO_BASIC	0	/* Basic weighted average algorithm (all 4 metrics) */
#define WL_WA_TYPE_RSSI		0
#define WL_WA_TYPE_SNR		1
#define WL_WA_TYPE_TXRATE	2
#define WL_WA_TYPE_RXRATE	3
#define WL_WA_TYPE_MAX		4

typedef struct {			/* payload of subcmd in xtlv */
	uint8 id;
	uint8 n_total;			/* Total number of samples (n_total >= n_recent) */
	uint8 n_recent;			/* Number of samples denoted as recent */
	uint8 w_recent;			/* Total weight for the recent samples (as percentage) */
} wl_wa_basic_params_t;

typedef struct {
	uint16 ver;
	uint16 len;
	uint8 subcmd[];			/* sub-cmd in bcm_xtlv_t */
} wl_wa_cmd_t;

/** data structure used in 'dfs_status' wl interface, which is used to query dfs status */
typedef struct {
	uint32 state;		/**< noted by WL_DFS_CACSTATE_XX. */
	uint32 duration;		/**< time spent in ms in state. */
	/**
	 * as dfs enters ISM state, it removes the operational channel from quiet channel
	 * list and notes the channel in channel_cleared. set to 0 if no channel is cleared
	 */
	chanspec_t chanspec_cleared;
	/** chanspec cleared used to be a uint32, add another to uint16 to maintain size */
	uint16 pad;
} wl_dfs_status_t;

typedef struct {
	uint32 state;		/* noted by WL_DFS_CACSTATE_XX */
	uint32 duration;		/* time spent in ms in state */
	chanspec_t chanspec;	/* chanspec of this core */
	chanspec_t chanspec_last_cleared; /* chanspec last cleared for operation by scanning */
	uint16 sub_type;	/* currently just the index of the core or the respective PLL */
	uint16 pad;
} wl_dfs_sub_status_t;

#define WL_DFS_STATUS_ALL_VERSION	(1)
typedef struct {
	uint16 version;		/* version field; current max version 1 */
	uint16 num_sub_status;
	wl_dfs_sub_status_t  dfs_sub_status[1]; /* struct array of length num_sub_status */
} wl_dfs_status_all_t;

#define WL_DFS_AP_MOVE_VERSION	(1)

struct wl_dfs_ap_move_status_v1 {
	int16 dfs_status;	/* DFS scan status */
	chanspec_t chanspec;	/* New AP Chanspec */
	wl_dfs_status_t cac_status;	/* CAC status */
};

typedef struct wl_dfs_ap_move_status_v2 {
	int8 version;            /* version field; current max version 1 */
	int8 move_status;        /* DFS move status */
	chanspec_t chanspec;     /* New AP Chanspec */
	wl_dfs_status_all_t scan_status; /* status; see dfs_status_all for wl_dfs_status_all_t */
} wl_dfs_ap_move_status_v2_t;

#define WL_DFS_AP_MOVE_ABORT -1		/* Abort any dfs_ap_move in progress immediately */
#define WL_DFS_AP_MOVE_STUNT -2		/* Stunt move but continue background CSA if in progress */

/** data structure used in 'radar_status' wl interface, which is use to query radar det status */
typedef struct {
	uint8 detected;
	uint8 PAD[3];
	int32 count;
	uint8 pretended;
	uint8 PAD[3];
	uint32 radartype;
	uint32 timenow;
	uint32 timefromL;
	int32  lp_csect_single;
	int32  detected_pulse_index;
	int32  nconsecq_pulses;
	chanspec_t ch;
	uint8 PAD[2];
	int32  pw[10];
	int32  intv[10];
	int32  fm[10];
} wl_radar_status_t;

#define NUM_PWRCTRL_RATES 12

typedef struct {
	uint8 txpwr_band_max[NUM_PWRCTRL_RATES];	/**< User set target */
	uint8 txpwr_limit[NUM_PWRCTRL_RATES];		/**< reg and local power limit */
	uint8 txpwr_local_max;				/**< local max according to the AP */
	uint8 txpwr_local_constraint;			/**< local constraint according to the AP */
	uint8 txpwr_chan_reg_max;			/**< Regulatory max for this channel */
	uint8 txpwr_target[2][NUM_PWRCTRL_RATES];	/**< Latest target for 2.4 and 5 Ghz */
	uint8 txpwr_est_Pout[2];			/**< Latest estimate for 2.4 and 5 Ghz */
	uint8 txpwr_opo[NUM_PWRCTRL_RATES];		/**< On G phy, OFDM power offset */
	uint8 txpwr_bphy_cck_max[NUM_PWRCTRL_RATES];	/**< Max CCK power for this band (SROM) */
	uint8 txpwr_bphy_ofdm_max;			/**< Max OFDM power for this band (SROM) */
	uint8 txpwr_aphy_max[NUM_PWRCTRL_RATES];	/**< Max power for A band (SROM) */
	int8  txpwr_antgain[2];				/**< Ant gain for each band - from SROM */
	uint8 txpwr_est_Pout_gofdm;			/**< Pwr estimate for 2.4 OFDM */
} tx_power_legacy_t;

#define WL_TX_POWER_RATES_LEGACY    45
#define WL_TX_POWER_MCS20_FIRST         12
#define WL_TX_POWER_MCS20_NUM           16
#define WL_TX_POWER_MCS40_FIRST         28
#define WL_TX_POWER_MCS40_NUM           17

typedef struct {
	uint32 flags;
	chanspec_t chanspec;                 /**< txpwr report for this channel */
	chanspec_t local_chanspec;           /**< channel on which we are associated */
	uint8 local_max;                 /**< local max according to the AP */
	uint8 local_constraint;              /**< local constraint according to the AP */
	int8  antgain[2];                /**< Ant gain for each band - from SROM */
	uint8 rf_cores;                  /**< count of RF Cores being reported */
	uint8 est_Pout[4];                           /**< Latest tx power out estimate per RF
							  * chain without adjustment
							  */
	uint8 est_Pout_cck;                          /**< Latest CCK tx power out estimate */
	uint8 user_limit[WL_TX_POWER_RATES_LEGACY];  /**< User limit */
	uint8 reg_limit[WL_TX_POWER_RATES_LEGACY];   /**< Regulatory power limit */
	uint8 board_limit[WL_TX_POWER_RATES_LEGACY]; /**< Max power board can support (SROM) */
	uint8 target[WL_TX_POWER_RATES_LEGACY];      /**< Latest target power */
	uint8 PAD[2];
} tx_power_legacy2_t;

#define WL_NUM_2x2_ELEMENTS		4
#define WL_NUM_3x3_ELEMENTS		6
#define WL_NUM_4x4_ELEMENTS		10

typedef struct {
	uint16 ver;				/**< version of this struct */
	uint16 len;				/**< length in bytes of this structure */
	uint32 flags;
	chanspec_t chanspec;			/**< txpwr report for this channel */
	chanspec_t local_chanspec;		/**< channel on which we are associated */
	uint32     buflen;			/**< ppr buffer length */
	uint8      pprbuf[1];			/**< Latest target power buffer */
} wl_txppr_t;

#define WL_TXPPR_VERSION	1
#define WL_TXPPR_LENGTH	(sizeof(wl_txppr_t))
#define TX_POWER_T_VERSION	45

/* curpower ppr types */
enum {
	PPRTYPE_TARGETPOWER	=	1,
	PPRTYPE_BOARDLIMITS	=	2,
	PPRTYPE_REGLIMITS	=	3,
	PPRTYPE_RU_REGLIMITS    =       4,
	PPRTYPE_RU_BOARDLIMITS  =       5,
	PPRTYPE_RU_TARGETPOWER  =       6,
	PPRTYPE_LAST
};

/** number of ppr serialization buffers, it should be reg, board and target */
#define WL_TXPPR_SER_BUF_NUM	(PPRTYPE_LAST - 1)

typedef struct chanspec_txpwr_max {
	chanspec_t chanspec;   /**< chanspec */
	uint8 txpwr_max;       /**< max txpwr in all the rates */
	uint8 padding;
} chanspec_txpwr_max_t;

typedef struct  wl_chanspec_txpwr_max {
	uint16 ver;			/**< version of this struct */
	uint16 len;			/**< length in bytes of this structure */
	uint32 count;		/**< number of elements of (chanspec, txpwr_max) pair */
	chanspec_txpwr_max_t txpwr[1];	/**< array of (chanspec, max_txpwr) pair */
} wl_chanspec_txpwr_max_t;

#define WL_CHANSPEC_TXPWR_MAX_VER	1
#define WL_CHANSPEC_TXPWR_MAX_LEN	(sizeof(wl_chanspec_txpwr_max_t))

typedef struct tx_inst_power {
	uint8 txpwr_est_Pout[2];			/**< Latest estimate for 2.4 and 5 Ghz */
	uint8 txpwr_est_Pout_gofdm;			/**< Pwr estimate for 2.4 OFDM */
} tx_inst_power_t;

#define WL_NUM_TXCHAIN_MAX	4
typedef struct wl_txchain_pwr_offsets {
	int8 offset[WL_NUM_TXCHAIN_MAX];	/**< quarter dBm signed offset for each chain */
} wl_txchain_pwr_offsets_t;

/** maximum channels returned by the get valid channels iovar */
#define WL_NUMCHANNELS		64
#define WL_NUMCHANNELS_MANY_CHAN 10
#define WL_ITER_LIMIT_MANY_CHAN 5

#define WL_MIMO_PS_CFG_VERSION_1 1

typedef struct wl_mimops_cfg {
	uint8 version;
	/* active_chains: 0 for all, 1 for 1 chain. */
	uint8 active_chains;
	/* static (0) or dynamic (1).or disabled (3) Mode applies only when active_chains = 0. */
	uint8 mode;
	/* bandwidth = Full (0), 20M (1), 40M (2), 80M (3). */
	uint8 bandwidth;
	uint8 applychangesafterlearning;
	uint8 pad[3];
} wl_mimops_cfg_t;

/* This event is for tracing MIMO PS metrics snapshot calls.
 * It is helpful to debug out-of-sync issue between
 * ucode SHM values and FW snapshot calculation.
 * It is part of the EVENT_LOG_TAG_MIMO_PS_TRACE.
 */
#define WL_MIMO_PS_METRICS_SNAPSHOT_TRACE_TYPE	0
typedef struct wl_mimo_ps_metrics_snapshot_trace {
	/* type field for this TLV: */
	uint16  type;
	/* length field for this TLV */
	uint16  len;
	uint32  idle_slotcnt_mimo;	/* MIMO idle slotcnt raw SHM value */
	uint32  last_idle_slotcnt_mimo;	/* stored value snapshot */
	uint32  idle_slotcnt_siso;	/* SISO idle slotcnt raw SHM value */
	uint32  last_idle_slotcnt_siso;	/* stored value snapshot */
	uint32	rx_time_mimo;		/* Rx MIMO raw SHM value */
	uint32	last_rx_time_mimo;	/* stored value snapshot */
	uint32	rx_time_siso;		/* RX SISO raw SHM value */
	uint32	last_rx_time_siso;	/* stored value snapshot */
	uint32  tx_time_1chain;		/* Tx 1-chain raw SHM value */
	uint32  last_tx_time_1chain;	/* stored value snapshot */
	uint32	tx_time_2chain;		/* Tx 2-chain raw SHM value */
	uint32	last_tx_time_2chain;	/* stored value snapshot */
	uint32  tx_time_3chain;		/* Tx 3-chain raw SHM value */
	uint32  last_tx_time_3chain;	/* stored value snapshot */
	uint16	reason;			/* reason for snapshot call, see below */
	/* Does the call reset last values after delta calculation */
	uint16	reset_last;
} wl_mimo_ps_metrics_snapshot_trace_t;
/* reason codes for mimo ps metrics snapshot function calls */
#define WL_MIMOPS_METRICS_SNAPSHOT_REPORT		1
#define WL_MIMOPS_METRICS_SNAPSHOT_RXCHAIN_SET		2
#define WL_MIMOPS_METRICS_SNAPSHOT_ARBI			3
#define WL_MIMOPS_METRICS_SNAPSHOT_SLOTUPD		4
#define WL_MIMOPS_METRICS_SNAPSHOT_PMBCNRX		5
#define WL_MIMOPS_METRICS_SNAPSHOT_BMACINIT		6
#define WL_MIMOPS_METRICS_SNAPSHOT_HT_COMPLETE		7
#define WL_MIMOPS_METRICS_SNAPSHOT_OCL                  8

#define WL_MIMO_PS_STATUS_VERSION_2	2
typedef struct wl_mimo_ps_status {
	uint8 version;
	uint8 ap_cap;			/* The associated AP's capability (BW, MIMO/SISO). */
	uint8 association_status;	/* How we are associated to the AP (MIMO/SISO). */
	uint8 mimo_ps_state;		/* mimo_ps_cfg states: [0-5]. See below for values */
	uint8 mrc_state;		/* MRC state: NONE (0), ACTIVE(1) */
	uint8 bss_rxchain;		/* bss rxchain bitmask */
	uint8 bss_txchain;		/* bss txchain bitmask */
	uint8 bss_bw;			/* bandwidth: Full (0), 20M (1), 40M (2), 80M (3), etc */
	uint16 hw_state;		/* bitmask of hw state. See below for values */
	uint8 hw_rxchain;		/* actual HW rxchain bitmask */
	uint8 hw_txchain;		/* actual HW txchain bitmask */
	uint8 hw_bw;			/* bandwidth: Full (0), 20M (1), 40M (2), 80M (3), etc */
	uint8 pm_bcnrx_state;		/* actual state of ucode flag */
	uint8 basic_rates_present;	/* internal flag to trigger siso bcmc rx */
	uint8 siso_bcmc_rx_state;	/* actual state of ucode flag */
} wl_mimo_ps_status_t;

#define WL_MIMO_PS_STATUS_VERSION_1	1
typedef struct wl_mimo_ps_status_v1 {
	uint8 version;
	uint8 ap_cap;			/* The associated AP's capability (BW, MIMO/SISO). */
	uint8 association_status;	/* How we are associated to the AP (MIMO/SISO). */
	uint8 mimo_ps_state;		/* mimo_ps_cfg states: [0-5]. See below for values */
	uint8 mrc_state;		/* MRC state: NONE (0), ACTIVE(1) */
	uint8 bss_rxchain;		/* bss rxchain bitmask */
	uint8 bss_txchain;		/* bss txchain bitmask */
	uint8 bss_bw;			/* bandwidth: Full (0), 20M (1), 40M (2), 80M (3), etc */
	uint16 hw_state;		/* bitmask of hw state. See below for values */
	uint8 hw_rxchain;		/* actual HW rxchain bitmask */
	uint8 hw_txchain;		/* actual HW txchain bitmask */
	uint8 hw_bw;			/* bandwidth: Full (0), 20M (1), 40M (2), 80M (3), etc */
	uint8 pad[3];
} wl_mimo_ps_status_v1_t;

#define WL_MIMO_PS_STATUS_AP_CAP(ap_cap)	(ap_cap & 0x0F)
#define WL_MIMO_PS_STATUS_AP_CAP_BW(ap_cap)	(ap_cap >> 4)
#define WL_MIMO_PS_STATUS_ASSOC_BW_SHIFT 4

/* version 3: assoc status: low nibble is status enum, high other flags */
#define WL_MIMO_PS_STATUS_VERSION_3			3
#define WL_MIMO_PS_STATUS_ASSOC_STATUS_MASK		0x0F
#define WL_MIMO_PS_STATUS_ASSOC_STATUS_VHT_WITHOUT_OMN	0x80

/* mimo_ps_status: ap_cap/association status */
enum {
	WL_MIMO_PS_STATUS_ASSOC_NONE	= 0,
	WL_MIMO_PS_STATUS_ASSOC_SISO	= 1,
	WL_MIMO_PS_STATUS_ASSOC_MIMO	= 2,
	WL_MIMO_PS_STATUS_ASSOC_LEGACY	= 3
};

/* mimo_ps_status: mimo_ps_cfg states */
enum {
	WL_MIMO_PS_CFG_STATE_NONE			= 0,
	WL_MIMO_PS_CFG_STATE_INFORM_AP_INPROGRESS	= 1,
	WL_MIMO_PS_CFG_STATE_INFORM_AP_DONE		= 2,
	WL_MIMO_PS_CFG_STATE_LEARNING			= 3,
	WL_MIMO_PS_CFG_STATE_HW_CONFIGURE		= 4,
	WL_MIMO_PS_CFG_STATE_INFORM_AP_PENDING		= 5
};

/* mimo_ps_status: hw_state values */
#define WL_MIMO_PS_STATUS_HW_STATE_NONE			0
#define WL_MIMO_PS_STATUS_HW_STATE_LTECOEX		(0x1 << 0)
#define WL_MIMO_PS_STATUS_HW_STATE_MIMOPS_BSS		(0x1 << 1)
#define WL_MIMO_PS_STATUS_HW_STATE_SCAN			(0x1 << 3)
#define WL_MIMO_PS_STATUS_HW_STATE_TXPPR		(0x1 << 4)
#define WL_MIMO_PS_STATUS_HW_STATE_PWRTHOTTLE		(0x1 << 5)
#define WL_MIMO_PS_STATUS_HW_STATE_TMPSENSE		(0x1 << 6)
#define WL_MIMO_PS_STATUS_HW_STATE_IOVAR		(0x1 << 7)
#define WL_MIMO_PS_STATUS_HW_STATE_AP_BSS		(0x1 << 8)

/* mimo_ps_status: mrc states */
#define WL_MIMO_PS_STATUS_MRC_NONE	0
#define WL_MIMO_PS_STATUS_MRC_ACTIVE	1

/* mimo_ps_status: core flag states for single-core beacon and siso-bcmc rx */
#define WL_MIMO_PS_STATUS_MHF_FLAG_NONE		0
#define WL_MIMO_PS_STATUS_MHF_FLAG_ACTIVE	1
#define WL_MIMO_PS_STATUS_MHF_FLAG_COREDOWN	2
#define WL_MIMO_PS_STATUS_MHF_FLAG_INVALID	3

/* Type values for the REASON */
#define WL_MIMO_PS_PS_LEARNING_ABORTED          (1 << 0)
#define WL_MIMO_PS_PS_LEARNING_COMPLETED        (1 << 1)
#define WL_MIMO_PS_PS_LEARNING_ONGOING          (1 << 2)

typedef struct wl_mimo_ps_learning_event_data {
	uint32 startTimeStamp;
	uint32 endTimeStamp;
	uint16 reason;
	struct ether_addr BSSID;
	uint32 totalSISO_below_rssi_threshold;
	uint32 totalMIMO_below_rssi_threshold;
	uint32 totalSISO_above_rssi_threshold;
	uint32 totalMIMO_above_rssi_threshold;
} wl_mimo_ps_learning_event_data_t;

#define WL_MIMO_PS_PS_LEARNING_CFG_ABORT	(1 << 0)
#define WL_MIMO_PS_PS_LEARNING_CFG_STATUS	(1 << 1)
#define WL_MIMO_PS_PS_LEARNING_CFG_CONFIG	(1 << 2)
#define WL_MIMO_PS_PS_LEARNING_CFG_MASK		(0x7)

#define WL_MIMO_PS_PS_LEARNING_CFG_V1 1

typedef struct wl_mimops_learning_cfg {
	/* flag:  bit 0 for abort */
	/* flag:  bit 1 for status */
	/* flag:  bit 2 for configuring no of packets and rssi */
	uint8                  flag;
	/* mimo ps learning version, compatible version is 0 */
	uint8                  version;
	/* if version is 0 or rssi is 0, ignored */
	int8                   learning_rssi_threshold;
	uint8                  reserved;
	uint32                 no_of_packets_for_learning;
	wl_mimo_ps_learning_event_data_t mimops_learning_data;
} wl_mimops_learning_cfg_t;

#define WL_OCL_STATUS_VERSION 1
typedef struct ocl_status_info {
	uint8  version;
	uint8  len;
	uint16 fw_status;     /* Bits representing FW disable reasons */
	uint8  hw_status;     /* Bits for actual HW config and SISO/MIMO coremask */
	uint8  coremask;      /* The ocl core mask (indicating listening core) */
} ocl_status_info_t;

/* MWS OCL map */
#define WL_MWS_OCL_OVERRIDE_VERSION 1
typedef struct wl_mws_ocl_override {
	uint16  version;    /* Structure version */
	uint16	bitmap_2g; /* bitmap for 2.4G channels bits 1-13 */
	uint16	bitmap_5g_lo;  /* bitmap for 5G low channels by 2:
				*34-48, 52-56, 60-64, 100-102
				*/
	uint16	bitmap_5g_mid; /* bitmap for 5G mid channels by 2:
				* 104, 108-112, 116-120, 124-128,
				* 132-136, 140, 149-151
				*/
	uint16	bitmap_5g_high; /* bitmap for 5G high channels by 2
				* 153, 157-161, 165
				*/
} wl_mws_ocl_override_t;

/* Bits for fw_status */
#define OCL_DISABLED_HOST		0x01   /* Host has disabled through ocl_enable */
#define OCL_DISABLED_RSSI		0x02   /* Disabled because of ocl_rssi_threshold */
#define OCL_DISABLED_LTEC		0x04   /* Disabled due to LTE Coex activity */
#define OCL_DISABLED_SISO		0x08   /* Disabled while in SISO mode */
#define OCL_DISABLED_CAL		0x10   /* Disabled during active calibration */
#define OCL_DISABLED_CHANSWITCH		0x20   /* Disabled during active channel switch */
#define OCL_DISABLED_ASPEND     0x40   /* Disabled due to assoc pending */

/* Bits for hw_status */
#define OCL_HWCFG			0x01   /* State of OCL config bit in phy HW */
#define OCL_HWMIMO			0x02   /* Set if current coremask is > 1 bit */
#define OCL_COREDOWN			0x80   /* Set if core is currently down */

#define WL_OPS_CFG_VERSION_1  1
/* Common IOVAR struct */
typedef struct wl_ops_cfg_v1 {
	uint16  version;
	uint16 len;		/* total length includes fixed fields and variable data[] */
	uint16 subcmd_id;	/* subcommand id */
	uint16 padding;		/* reserved / padding for 4 byte align */
	uint8 data[];		/* subcommand data; could be empty */
} wl_ops_cfg_v1_t;

/* subcommands ids */
enum {
	WL_OPS_CFG_SUBCMD_ENABLE =		0,	/* OPS enable/disable mybss and obss
				                         * for nav and plcp options
				                         */
	WL_OPS_CFG_SUBCMD_MAX_SLEEP_DUR =	1,	/* Max sleep duration used for OPS */
	WL_OPS_CFG_SUBCMD_RESET_STATS =		2	/* Reset stats part of ops_status
			                                 * on both slices
				                         */
};

#define WL_OPS_CFG_MASK				0xffff
#define WL_OPS_CFG_CAP_MASK			0xffff0000
#define WL_OPS_CFG_CAP_SHIFT			16	/* Shift bits to locate the OPS CAP */
#define WL_OPS_MAX_SLEEP_DUR			12500	/* max ops duration in us */
#define WL_OPS_MINOF_MAX_SLEEP_DUR		512	/* minof max ops duration in us */
#define WL_OPS_SUPPORTED_CFG			(WL_OPS_MYBSS_PLCP_DUR | WL_OPS_MYBSS_NAV_DUR \
		                                 | WL_OPS_OBSS_PLCP_DUR | WL_OPS_OBSS_NAV_DUR)
#define WL_OPS_DEFAULT_CFG		        WL_OPS_SUPPORTED_CFG

/* WL_OPS_CFG_SUBCMD_ENABLE */
typedef struct wl_ops_cfg_enable {
	uint32   bits;   /* selectively enable ops for mybss and obss */
} wl_ops_cfg_enable_t;
/* Bits for WL_OPS_CFG_SUBCMD_ENABLE Parameter */
#define WL_OPS_MYBSS_PLCP_DUR		0x1	/* OPS based on mybss 11b & 11n mixed HT frames
		                                 * PLCP header duration
		                                 */
#define WL_OPS_MYBSS_NAV_DUR		0x2	/* OPS based on mybss RTS-CTS duration */
#define WL_OPS_OBSS_PLCP_DUR		0x4	/* OPS based on obss 11b & 11n mixed HT frames
		                                 * PLCP header duration
		                                 */
#define WL_OPS_OBSS_NAV_DUR		0x8	/* OPS based on obss RTS-CTS duration */

/* WL_OPS_CFG_SUBCMD_MAX_SLEEP_DUR */
typedef struct wl_ops_cfg_max_sleep_dur {
	uint32  val;   /* maximum sleep duration (us) used for OPS */
} wl_ops_cfg_max_sleep_dur_t;

/* WL_OPS_CFG_SUBCMD_RESET_STATS */
typedef struct wl_ops_cfg_reset_stats {
	uint32  val;   /* bitmap of slices, 0 means all slices */
} wl_ops_cfg_reset_stats_t;

#define WL_OPS_STATUS_VERSION_1 1
#define OPS_DUR_HIST_BINS	5	/* number of bins used, 0-1, 1-2, 2-4, 4-8, >8 msec */
typedef struct wl_ops_status_v1 {
	uint16  version;
	uint16  len;			/* Total length including all fixed fields */
	uint8	slice_index;		/* Slice for which status is reported */
	uint8   disable_obss;		/* indicate if obss cfg is disabled */
	uint8   pad[2];			/* 4-byte alignment */
	uint32  disable_reasons;	/* FW disable reasons */
	uint32  disable_duration;	/* ops disable time(ms) due to disable reasons */
	uint32  applied_ops_config;	/* currently applied ops config */
	uint32  partial_ops_dur;	/* Total time (in usec) of partial ops duration */
	uint32  full_ops_dur;		/* Total time (in usec) of full ops duration */
	uint32  count_dur_hist[OPS_DUR_HIST_BINS];	/* ops occurrence histogram */
	uint32  nav_cnt;		/* number of times ops triggered based NAV duration */
	uint32  plcp_cnt;		/* number of times ops triggered based PLCP duration */
	uint32  mybss_cnt;		/* number of times mybss ops trigger */
	uint32  obss_cnt;		/* number of times obss ops trigger */
	uint32  miss_dur_cnt;		/* number of times ops couldn't happen
		                         * due to insufficient duration
		                         */
	uint32  miss_premt_cnt;		/* number of times ops couldn't happen due
		                         * to not meeting Phy preemption thresh
		                         */
	uint32  max_dur_cnt;		/* number of times ops did not trigger due to
		                         * frames exceeding max sleep duration
		                         */
	uint32	wake_cnt;		/* number of ops miss due to wake reason */
	uint32	bcn_wait_cnt;		/* number of ops miss due to waiting for bcn */
} wl_ops_status_v1_t;
/* Bits for disable_reasons */
#define OPS_DISABLED_HOST	0x01	/* Host has disabled through ops_cfg */
#define OPS_DISABLED_UNASSOC	0x02	/* Disabled because the slice is in unassociated state */
#define OPS_DISABLED_SCAN	0x04	/* Disabled because the slice is in scan state */
#define OPS_DISABLED_BCN_MISS	0x08	/* Disabled because beacon missed for a duration */

#define WL_PSBW_CFG_VERSION_1  1
/* Common IOVAR struct */
typedef struct wl_psbw_cfg_v1 {
	uint16  version;
	uint16 len;			/* total length includes fixed fields and variable data[] */
	uint16 subcmd_id;		/* subcommand id */
	uint16 pad;			/* reserved / padding for 4 byte align */
	uint8 data[];			/* subcommand data */
} wl_psbw_cfg_v1_t;

/* subcommands ids */
enum {
	/* PSBW enable/disable */
	WL_PSBW_CFG_SUBCMD_ENABLE =                0,
	/* override psbw disable requests */
	WL_PSBW_CFG_SUBCMD_OVERRIDE_DISABLE_MASK = 1,
	/* Reset stats part of psbw status */
	WL_PSBW_CFG_SUBCMD_RESET_STATS =           2
};

#define WL_PSBW_OVERRIDE_DISA_CFG_MASK			0x0000ffff
#define WL_PSBW_OVERRIDE_DISA_CAP_MASK			0xffff0000
#define WL_PSBW_OVERRIDE_DISA_CAP_SHIFT			16  /* shift bits for cap */

/* WL_PSBW_CFG_SUBCMD_ENABLE */
typedef struct wl_psbw_cfg_enable {
	bool   enable;		/* enable or disable */
} wl_psbw_cfg_enable_t;

/* WL_PSBW_CFG_SUBCMD_OVERRIDE_DISABLE_MASK */
typedef struct wl_psbw_cfg_override_disable_mask {
	uint32  mask;		/* disable requests to override, cap and current cfg */
} wl_psbw_cfg_override_disable_mask_t;

/* WL_PSBW_CFG_SUBCMD_RESET_STATS */
typedef struct wl_psbw_cfg_reset_stats {
	uint32  val;		/* infra interface index, 0 */
} wl_psbw_cfg_reset_stats_t;

#define WL_PSBW_STATUS_VERSION_1 1
typedef struct wl_psbw_status_v1 {
	uint16  version;
	uint16  len;			/* total length including all fixed fields */
	uint8   curr_slice_index;	/* current slice index of the interface */
	uint8   associated;		/* interface associatd */
	chanspec_t   chspec;		/* radio chspec */
	uint32	state;			/* psbw state */
	uint32  disable_reasons;	/* FW disable reasons */
	uint32  slice_enable_dur;	/* time(ms) psbw remains enabled on this slice */
	uint32  total_enable_dur;	/* time(ms) psbw remains enabled total */
	uint32  enter_cnt;		/* total cnt entering PSBW active */
	uint32  exit_cnt;		/* total cnt exiting PSBW active */
	uint32  exit_imd_cnt;		/* total cnt imd exit when waited N tbtts */
	uint32  enter_skip_cnt;		/* total cnt entering PSBW active skipped */
} wl_psbw_status_v1_t;

/* Bit for state */
#define PSBW_ACTIVE				0x1 /* active 20MHz */
#define PSBW_TTTT_PEND				0x2 /* waiting for TTTT intr */
#define PSBW_WAIT_ENTER				0x4 /* in wait period before entering */
#define PSBW_CAL_DONE				0x8 /* 20M channel cal done */

/* Bits for disable_reasons */
#define WL_PSBW_DISA_HOST			0x00000001 /* Host has disabled through psbw_cfg */
#define WL_PSBW_DISA_AP20M			0x00000002 /* AP is operating on 20 MHz */
#define WL_PSBW_DISA_SLOTTED_BSS		0x00000004 /* slot_bss active */
#define WL_PSBW_DISA_NOT_PMFAST			0x00000008 /* Not PM_FAST */
#define WL_PSBW_DISA_BASICRATESET		0x00000010 /* BasicRateSet is empty */
#define WL_PSBW_DISA_NOT_D3			0x00000020 /* PCIe not in D3 */
#define WL_PSBW_DISA_CSA			0x00000040 /* CSA IE is present */
#define WL_PSBW_DISA_ASSOC			0x00000080 /* assoc state is active/or unassoc */
#define WL_PSBW_DISA_SCAN			0x00000100 /* scan state is active */
#define WL_PSBW_DISA_CAL			0x00000200 /* cal pending or active */
/* following are not part of disable reasons */
#define WL_PSBW_EXIT_PM				0x00001000 /* Out of PM */
#define WL_PSBW_EXIT_TIM			0x00002000 /* unicast TIM bit present */
#define WL_PSBW_EXIT_DATA			0x00004000 /* Data for transmission */
#define WL_PSBW_EXIT_MGMTDATA			0x00008000 /* management frame for transmission */
#define WL_PSBW_EXIT_BW_UPD			0x00010000 /* BW being updated */
#define WL_PSBW_DISA_NONE			0x80000000 /* reserved for internal use only */

/*
 * Join preference iovar value is an array of tuples. Each tuple has a one-byte type,
 * a one-byte length, and a variable length value.  RSSI type tuple must be present
 * in the array.
 *
 * Types are defined in "join preference types" section.
 *
 * Length is the value size in octets. It is reserved for WL_JOIN_PREF_WPA type tuple
 * and must be set to zero.
 *
 * Values are defined below.
 *
 * 1. RSSI - 2 octets
 * offset 0: reserved
 * offset 1: reserved
 *
 * 2. WPA - 2 + 12 * n octets (n is # tuples defined below)
 * offset 0: reserved
 * offset 1: # of tuples
 * offset 2: tuple 1
 * offset 14: tuple 2
 * ...
 * offset 2 + 12 * (n - 1) octets: tuple n
 *
 * struct wpa_cfg_tuple {
 *   uint8 akm[DOT11_OUI_LEN+1];     akm suite
 *   uint8 ucipher[DOT11_OUI_LEN+1]; unicast cipher suite
 *   uint8 mcipher[DOT11_OUI_LEN+1]; multicast cipher suite
 * };
 *
 * multicast cipher suite can be specified as a specific cipher suite or WL_WPA_ACP_MCS_ANY.
 *
 * 3. BAND - 2 octets
 * offset 0: reserved
 * offset 1: see "band preference" and "band types"
 *
 * 4. BAND RSSI - 2 octets
 * offset 0: band types
 * offset 1: +ve RSSI boost value in dB
 */

struct tsinfo_arg {
	uint8 octets[3];
};

#define RATE_CCK_1MBPS 0
#define RATE_CCK_2MBPS 1
#define RATE_CCK_5_5MBPS 2
#define RATE_CCK_11MBPS 3

#define RATE_LEGACY_OFDM_6MBPS 0
#define RATE_LEGACY_OFDM_9MBPS 1
#define RATE_LEGACY_OFDM_12MBPS 2
#define RATE_LEGACY_OFDM_18MBPS 3
#define RATE_LEGACY_OFDM_24MBPS 4
#define RATE_LEGACY_OFDM_36MBPS 5
#define RATE_LEGACY_OFDM_48MBPS 6
#define RATE_LEGACY_OFDM_54MBPS 7

#define WL_BSSTRANS_RSSI_RATE_MAP_VERSION 1
#define WL_BSSTRANS_RSSI_RATE_MAP_VERSION_V1 1
#define WL_BSSTRANS_RSSI_RATE_MAP_VERSION_V2 2

typedef struct wl_bsstrans_rssi {
	int8 rssi_2g;	/**< RSSI in dbm for 2.4 G */
	int8 rssi_5g;	/**< RSSI in dbm for 5G, unused for cck */
} wl_bsstrans_rssi_t;

#define RSSI_RATE_MAP_MAX_STREAMS 4	/**< max streams supported */

/** RSSI to rate mapping, all 20Mhz, no SGI */
typedef struct wl_bsstrans_rssi_rate_map_v2 {
	uint16 ver;
	uint16 len; /**< length of entire structure */
	wl_bsstrans_rssi_t cck[WL_NUM_RATES_CCK]; /**< 2.4G only */
	wl_bsstrans_rssi_t ofdm[WL_NUM_RATES_OFDM]; /**< 6 to 54mbps */
	wl_bsstrans_rssi_t phy_n[RSSI_RATE_MAP_MAX_STREAMS][WL_NUM_RATES_MCS_1STREAM]; /* MCS0-7 */
	wl_bsstrans_rssi_t phy_ac[RSSI_RATE_MAP_MAX_STREAMS][WL_NUM_RATES_VHT_ALL]; /**< MCS0-11 */
	wl_bsstrans_rssi_t phy_ax[RSSI_RATE_MAP_MAX_STREAMS][WL_NUM_RATES_HE]; /**< MCS0-11 */
} wl_bsstrans_rssi_rate_map_v2_t;

/** RSSI to rate mapping, all 20Mhz, no SGI */
typedef struct wl_bsstrans_rssi_rate_map_v1 {
	uint16 ver;
	uint16 len; /**< length of entire structure */
	wl_bsstrans_rssi_t cck[WL_NUM_RATES_CCK]; /**< 2.4G only */
	wl_bsstrans_rssi_t ofdm[WL_NUM_RATES_OFDM]; /**< 6 to 54mbps */
	wl_bsstrans_rssi_t phy_n[RSSI_RATE_MAP_MAX_STREAMS][WL_NUM_RATES_MCS_1STREAM]; /* MCS0-7 */
	wl_bsstrans_rssi_t phy_ac[RSSI_RATE_MAP_MAX_STREAMS][WL_NUM_RATES_VHT]; /**< MCS0-9 */
} wl_bsstrans_rssi_rate_map_v1_t;

/** RSSI to rate mapping, all 20Mhz, no SGI */
typedef struct wl_bsstrans_rssi_rate_map {
	uint16 ver;
	uint16 len; /**< length of entire structure */
	wl_bsstrans_rssi_t cck[WL_NUM_RATES_CCK]; /**< 2.4G only */
	wl_bsstrans_rssi_t ofdm[WL_NUM_RATES_OFDM]; /**< 6 to 54mbps */
	wl_bsstrans_rssi_t phy_n[RSSI_RATE_MAP_MAX_STREAMS][WL_NUM_RATES_MCS_1STREAM]; /* MCS0-7 */
	wl_bsstrans_rssi_t phy_ac[RSSI_RATE_MAP_MAX_STREAMS][WL_NUM_RATES_VHT]; /**< MCS0-9 */
} wl_bsstrans_rssi_rate_map_t;

#define WL_BSSTRANS_ROAMTHROTTLE_VERSION 1

/** Configure number of scans allowed per throttle period */
typedef struct wl_bsstrans_roamthrottle {
	uint16 ver;
	uint16 period;
	uint16 scans_allowed;
} wl_bsstrans_roamthrottle_t;

#define	NFIFO			6	/**< # tx/rx fifopairs */

#ifndef NFIFO_EXT
#if defined(BCM_AQM_DMA_DESC) && !defined(BCM_AQM_DMA_DESC_DISABLED)
#define NFIFO_EXT		10		/* 4EDCA + 4 TWT + 1 Mcast/Bcast + 1 Spare */
#elif defined(WL11AX_TRIGGERQ) && !defined(WL11AX_TRIGGERQ_DISABLED)
#define NFIFO_EXT		10
#else
#define NFIFO_EXT		NFIFO
#endif /* BCM_AQM_DMA_DESC && !BCM_AQM_DMA_DESC_DISABLED */
#endif /* NFIFO_EXT */

/* When new reason codes are added to list, Please update wl_reinit_names also */
/* Reinit reason codes */
enum {
	WL_REINIT_RC_NONE             = 0,
	WL_REINIT_RC_PS_SYNC          = 1,
	WL_REINIT_RC_PSM_WD           = 2,
	WL_REINIT_RC_MAC_WAKE         = 3,
	WL_REINIT_RC_MAC_SUSPEND      = 4,
	WL_REINIT_RC_MAC_SPIN_WAIT    = 5,
	WL_REINIT_RC_AXI_BUS_ERROR    = 6,
	WL_REINIT_RC_DEVICE_REMOVED   = 7,
	WL_REINIT_RC_PCIE_FATAL_ERROR = 8,
	WL_REINIT_RC_OL_FW_TRAP       = 9,
	WL_REINIT_RC_FIFO_ERR         = 10,
	WL_REINIT_RC_INV_TX_STATUS    = 11,
	WL_REINIT_RC_MQ_ERROR         = 12,
	WL_REINIT_RC_PHYTXERR_THRESH  = 13,
	WL_REINIT_RC_USER_FORCED      = 14,
	WL_REINIT_RC_FULL_RESET       = 15,
	WL_REINIT_RC_AP_BEACON        = 16,
	WL_REINIT_RC_PM_EXCESSED      = 17,
	WL_REINIT_RC_NO_CLK           = 18,
	WL_REINIT_RC_SW_ASSERT        = 19,
	WL_REINIT_RC_PSM_JMP0         = 20,
	WL_REINIT_RC_PSM_RUN          = 21,
	WL_REINIT_RC_ENABLE_MAC       = 22,
	WL_REINIT_RC_SCAN_TIMEOUT     = 23,
	WL_REINIT_RC_JOIN_TIMEOUT     = 24,
	/* Below error codes are generated during D3 exit validation */
	WL_REINIT_RC_LINK_NOT_ACTIVE  = 25,
	WL_REINIT_RC_PCI_CFG_RD_FAIL  = 26,
	WL_REINIT_RC_INV_VEN_ID       = 27,
	WL_REINIT_RC_INV_DEV_ID       = 28,
	WL_REINIT_RC_INV_BAR0         = 29,
	WL_REINIT_RC_INV_BAR2         = 30,
	WL_REINIT_RC_AER_UC_FATAL     = 31,
	WL_REINIT_RC_AER_UC_NON_FATAL = 32,
	WL_REINIT_RC_AER_CORR         = 33,
	WL_REINIT_RC_AER_DEV_STS      = 34,
	WL_REINIT_RC_PCIe_STS         = 35,
	WL_REINIT_RC_MMIO_RD_FAIL     = 36,
	WL_REINIT_RC_MMIO_RD_INVAL    = 37,
	WL_REINIT_RC_MMIO_ARM_MEM_RD_FAIL = 38,
	WL_REINIT_RC_MMIO_ARM_MEM_INVAL   = 39,
	WL_REINIT_RC_SROM_LOAD_FAILED     = 40,
	WL_REINIT_RC_PHY_CRASH            = 41,
	WL_REINIT_TX_STALL                = 42,
	WL_REINIT_RC_TX_FLOW_CONTROL_BLOCKED	= 43,
	WL_REINIT_RC_RX_HC_FAIL           = 44,
	WL_REINIT_RC_RX_DMA_STALL         = 45,
	WL_REINIT_UTRACE_BUF_OVERLAP_SR	  = 46,
	WL_REINIT_UTRACE_TPL_OUT_BOUNDS   = 47,
	WL_REINIT_UTRACE_TPL_OSET_STRT0   = 48,
	WL_REINIT_RC_PHYTXERR             = 49,
	WL_REINIT_RC_PSM_FATAL_SUSP       = 50,
	WL_REINIT_RC_TX_FIFO_SUSP         = 51,
	WL_REINIT_RC_MAC_ENABLE           = 52,
	WL_REINIT_RC_SCAN_STALLED         = 53,
	WL_REINIT_RC_PHY_HC		  = 54,
	WL_REINIT_RC_LAST	/* This must be the last entry */
};

#define WL_REINIT_RC_INVALID	255

#define NREINITREASONCOUNT	8
/* NREINITREASONCOUNT is 8 in other branches.
 * Any change to this will break wl tool compatibility with other branches
 * #define NREINITREASONCOUNT	WL_REINIT_RC_LAST
 */

#define REINITRSNIDX(_x)	(((_x) < WL_REINIT_RC_LAST) ? (_x) : 0)

#define	WL_CNT_T_VERSION	30	/**< current version of wl_cnt_t struct */
#define WL_CNT_VERSION_6	6
#define WL_CNT_VERSION_7	7
#define WL_CNT_VERSION_11	11
#define WL_CNT_VERSION_XTLV	30

#define WL_COUNTERS_IOV_VERSION_1	1
#define WL_SUBCNTR_IOV_VER		WL_COUNTERS_IOV_VERSION_1
/* First two uint16 are version and lenght fields. So offset of the first counter will be 4 */
#define FIRST_COUNTER_OFFSET		0x04

/* need for now due to src/wl/ndis automerged to other branches. e.g. BISON */
#define WLC_WITH_XTLV_CNT

/* Number of xtlv info as required to calculate subcounter offsets */
#define WL_CNT_XTLV_ID_NUM	12
#define WL_TLV_IOV_VER		1

/**
 * tlv IDs uniquely identifies counter component
 * packed into wl_cmd_t container
 */
enum wl_cnt_xtlv_id {
	WL_CNT_XTLV_SLICE_IDX = 0x1,		/**< Slice index */
	WL_CNT_XTLV_WLC = 0x100,		/**< WLC layer counters */
	WL_CNT_XTLV_WLC_RINIT_RSN = 0x101,	/**< WLC layer reinitreason extension */
	WL_CNT_XTLV_WLC_HE = 0x102,		/* he counters */
	WL_CNT_XTLV_WLC_SECVLN = 0x103,		/* security vulnerabilities counters */
	WL_CNT_XTLV_WLC_HE_OMI = 0x104,		/* he omi counters */
	WL_CNT_XTLV_CNTV_LE10_UCODE = 0x200,	/**< wl counter ver < 11 UCODE MACSTAT */
	WL_CNT_XTLV_LT40_UCODE_V1 = 0x300,	/**< corerev < 40 UCODE MACSTAT */
	WL_CNT_XTLV_GE40_UCODE_V1 = 0x400,	/**< corerev >= 40 UCODE MACSTAT */
	WL_CNT_XTLV_GE64_UCODEX_V1 = 0x800,	/* corerev >= 64 UCODEX MACSTAT */
	WL_CNT_XTLV_GE80_UCODE_V1 = 0x900,	/* corerev >= 80 UCODEX MACSTAT */
	WL_CNT_XTLV_GE80_TXFUNFL_UCODE_V1 = 0x1000	/* corerev >= 80 UCODEX MACSTAT */
};

/* tlv IDs uniquely identifies periodic state component */
enum wl_periodic_slice_state_xtlv_id {
	WL_STATE_COMPACT_COUNTERS = 0x1,
	WL_STATE_TXBF_COUNTERS = 0x2,
	WL_STATE_COMPACT_HE_COUNTERS = 0x3
};

/* Sub tlvs for chan_counters */
enum wl_periodic_chan_xtlv_id {
	WL_CHAN_GENERIC_COUNTERS = 0x1,
	WL_CHAN_PERIODIC_COUNTERS = 0x2
};

#ifdef WLC_CHAN_ECNTR_TEST
#define WL_CHAN_PERIODIC_CNTRS_VER_1 1
typedef struct wlc_chan_periodic_cntr
{
	uint16 version;
	uint16 pad;
	uint32	rxstrt;
} wlc_chan_periodic_cntr_t;
#endif /* WLC_CHAN_ECNTR_TEST */

#define WL_CHANCNTR_HDR_VER_1 1
typedef struct wlc_chan_cntr_hdr_v1
{
	uint16 version;
	uint16 pad;
	chanspec_t chanspec;	/* Dont add any fields above this */
	uint16 pad1;
	uint32 total_time;
	uint32 chan_entry_cnt;
} wlc_chan_cntr_hdr_v1_t;

/* tlv IDs uniquely identifies periodic state component */
enum wl_periodic_if_state_xtlv_id {
	WL_STATE_IF_COMPACT_STATE = 0x1,
	WL_STATE_IF_ADPS_STATE = 0x02
};

enum wl_periodic_tdls_if_state_xtlv_id {
	WL_STATE_IF_TDLS_STATE = 0x1
};

#define TDMTX_CNT_VERSION_V1      1
#define TDMTX_CNT_VERSION_V2      2

/* structure holding tdm counters that interface to iovar */
typedef struct tdmtx_cnt_v1 {
	uint16 ver;
	uint16 length; /* length of this structure */
	uint16 wlc_idx; /* index for wlc */
	uint16 enabled; /* tdmtx is enabled on slice */
	uint32 tdmtx_txa_on; /* TXA on requests */
	uint32 tdmtx_txa_tmcnt; /* Total number of TXA timeout */
	uint32 tdmtx_por_on; /* TXA POR requests */
	uint32 tdmtx_txpuen; /* Path enable requests */
	uint32 tdmtx_txpudis; /* Total number of times Tx path is muted on the slice */
	uint32 tdmtx_txpri_on; /* Total number of times Tx priority was obtained by the slice */
	uint32 tdmtx_txdefer; /* Total number of times Tx was deferred on the slice */
	uint32 tdmtx_txmute; /* Total number of times active Tx muted on the slice */
	uint32 tdmtx_actpwrboff; /* Total number of times TX power is backed off by the slice */
	uint32 tdmtx_txa_dur; /* Total time txa on */
	uint32 tdmtx_txpri_dur; /* Total time TXPri */
	uint32 tdmtx_txdefer_dur; /* Total time txdefer */
	/* TDMTX input fields */
	uint32 tdmtx_txpri;
	uint32 tdmtx_defer;
	uint32 tdmtx_threshold;
	uint32 tdmtx_rssi_threshold;
	uint32 tdmtx_txpwrboff;
	uint32 tdmtx_txpwrboff_dt;
} tdmtx_cnt_v1_t;

typedef struct {
	uint16  ver;
	uint16	length; /* length of the data portion */
	uint16  cnt;
	uint16	pad; /* pad to align to 32 bit */
	uint8   data[]; /* array of tdmtx_cnt_v1_t */
} tdmtx_status_t;

/* structure holding counters that match exactly shm field sizes */
typedef struct tdmtx_cnt_shm_v1 {
	uint16 tdmtx_txa_on; /* TXA on requests */
	uint16 tdmtx_tmcnt; /* TXA on requests */
	uint16 tdmtx_por_on; /* TXA POR requests */
	uint16 tdmtx_txpuen; /* Path enable requests */
	uint16 tdmtx_txpudis; /* Total number of times Tx path is muted on the slice */
	uint16 tdmtx_txpri_on; /* Total number of times Tx priority was obtained by the slice */
	uint16 tdmtx_txdefer; /* Total number of times Tx was defered by the slice */
	uint16 tdmtx_txmute; /* Total number of times active Tx muted on the slice */
	uint16 tdmtx_actpwrboff; /* Total number of times TX power is backed off by the slice */
	uint16 tdmtx_txa_dur_l; /* Total time (low 16 bits) txa on */
	uint16 tdmtx_txa_dur_h; /* Total time (low 16 bits) txa on */
	uint16 tdmtx_txpri_dur_l; /* Total time (low 16 bits) TXPri */
	uint16 tdmtx_txpri_dur_h; /* Total time (high 16 bits) TXPri */
	uint16 tdmtx_txdefer_dur_l; /* Total time (low 16 bits)  txdefer */
	uint16 tdmtx_txdefer_dur_h; /* Total time (high 16 bits) txdefer */
} tdmtx_cnt_shm_v1_t;

/* structure holding tdm counters that interface to iovar for version 2 */
typedef struct tdmtx_cnt_v2 {
	uint16 ver;
	uint16 length; /* length of this structure */
	uint16 wlc_idx; /* index for wlc */
	uint16 enabled; /* tdmtx is enabled on slice */
	uint32 tdmtx_txa_on; /* TXA on requests */
	uint32 tdmtx_txa_tmcnt; /* Total number of TXA timeout */
	uint32 tdmtx_porhi_on; /* TXA PORHI requests */
	uint32 tdmtx_porlo_on; /* TXA PORLO requests */
	uint32 tdmtx_txpuen; /* Path enable requests */
	uint32 tdmtx_txpudis; /* Total number of times Tx path is muted on the slice */
	uint32 tdmtx_txpri_on; /* Total number of times Tx priority was obtained by the slice */
	uint32 tdmtx_txdefer; /* Total number of times Tx was deferred on the slice */
	uint32 tdmtx_txmute; /* Total number of times active Tx muted on the slice */
	uint32 tdmtx_actpwrboff; /* Total number of times TX power is backed off by the slice */
	uint32 tdmtx_txa_dur; /* Total time txa on */
	uint32 tdmtx_txpri_dur; /* Total time TXPri */
	uint32 tdmtx_txdefer_dur; /* Total time txdefer */
	/* TDMTX input fields */
	uint32 tdmtx_txpri;
	uint32 tdmtx_defer;
	uint32 tdmtx_threshold;
	uint32 tdmtx_rssi_threshold;
	uint32 tdmtx_txpwrboff;
	uint32 tdmtx_txpwrboff_dt;
} tdmtx_cnt_v2_t;

/* structure holding counters that match exactly shm field sizes */
typedef struct tdmtx_cnt_shm_v2 {
	uint16 tdmtx_txa_on; /* TXA on requests */
	uint16 tdmtx_tmcnt; /* TXA on requests */
	uint16 tdmtx_porhi_on; /* TXA PORHI requests */
	uint16 tdmtx_porlo_on; /* TXA PORLO requests */
	uint16 tdmtx_txpuen; /* Path enable requests */
	uint16 tdmtx_txpudis; /* Total number of times Tx path is muted on the slice */
	uint16 tdmtx_txpri_on; /* Total number of times Tx priority was obtained by the slice */
	uint16 tdmtx_txdefer; /* Total number of times Tx was defered by the slice */
	uint16 tdmtx_txmute; /* Total number of times active Tx muted on the slice */
	uint16 tdmtx_actpwrboff; /* Total number of times TX power is backed off by the slice */
	uint16 tdmtx_txa_dur_l; /* Total time (low 16 bits) txa on */
	uint16 tdmtx_txa_dur_h; /* Total time (low 16 bits) txa on */
	uint16 tdmtx_txpri_dur_l; /* Total time (low 16 bits) TXPri */
	uint16 tdmtx_txpri_dur_h; /* Total time (high 16 bits) TXPri */
	uint16 tdmtx_txdefer_dur_l; /* Total time (low 16 bits)  txdefer */
	uint16 tdmtx_txdefer_dur_h; /* Total time (high 16 bits) txdefer */
} tdmtx_cnt_shm_v2_t;

typedef struct wl_tdmtx_ioc {
	uint16 id;	/* ID of the sub-command */
	uint16 len;	/* total length of all data[] */
	uint8  data[];	/* var len payload */
} wl_tdmtx_ioc_t;

/*
 * iovar subcommand ids
 */
enum {
	IOV_TDMTX_ENB = 1,
	IOV_TDMTX_STATUS = 2,
	IOV_TDMTX_TXPRI = 3,
	IOV_TDMTX_DEFER = 4,
	IOV_TDMTX_TXA = 5,
	IOV_TDMTX_CFG = 6,
	IOV_TDMTX_LAST
};

/**
 * The number of variables in wl macstat cnt struct.
 * (wl_cnt_ge40mcst_v1_t, wl_cnt_lt40mcst_v1_t, wl_cnt_v_le10_mcst_t)
 */
#define WL_CNT_MCST_VAR_NUM 64
/* sizeof(wl_cnt_ge40mcst_v1_t), sizeof(wl_cnt_lt40mcst_v1_t), and sizeof(wl_cnt_v_le10_mcst_t) */
#define WL_CNT_MCST_STRUCT_SZ ((uint32)sizeof(uint32) * WL_CNT_MCST_VAR_NUM)
#define WL_CNT_REV80_MCST_STRUCT_SZ ((uint32)sizeof(wl_cnt_ge80mcst_v1_t))
#define WL_CNT_REV80_MCST_TXFUNFlW_STRUCT_FIXED_SZ \
	((uint32)OFFSETOF(wl_cnt_ge80_txfunfl_v1_t, txfunfl))
#define WL_CNT_REV80_MCST_TXFUNFl_STRUCT_SZ(fcnt) \
	(WL_CNT_REV80_MCST_TXFUNFlW_STRUCT_FIXED_SZ + (fcnt * sizeof(uint32)))
#define WL_CNT_REV80_MCST_TXFUNFlW_STRUCT_SZ (WL_CNT_REV80_MCST_TXFUNFl_STRUCT_SZ(NFIFO_EXT))

#define WL_CNT_MCXST_STRUCT_SZ ((uint32)sizeof(wl_cnt_ge64mcxst_v1_t))

#define WL_CNT_HE_STRUCT_SZ ((uint32)sizeof(wl_he_cnt_wlc_t))

#define WL_CNT_SECVLN_STRUCT_SZ ((uint32)sizeof(wl_secvln_cnt_t))

#define WL_CNT_HE_OMI_STRUCT_SZ ((uint32)sizeof(wl_he_omi_cnt_wlc_v1_t))
#define INVALID_CNT_VAL (uint32)(-1)

#define WL_XTLV_CNTBUF_MAX_SIZE ((uint32)(OFFSETOF(wl_cnt_info_t, data)) +	\
		(uint32)BCM_XTLV_HDR_SIZE + (uint32)sizeof(wl_cnt_wlc_t) +		\
		(uint32)BCM_XTLV_HDR_SIZE + WL_CNT_MCST_STRUCT_SZ +              \
		(uint32)BCM_XTLV_HDR_SIZE + WL_CNT_MCXST_STRUCT_SZ)

#define WL_CNTBUF_MAX_SIZE MAX(WL_XTLV_CNTBUF_MAX_SIZE, (uint32)sizeof(wl_cnt_ver_11_t))

/* Please refer to the twiki for counters addition/deletion.
 * http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/WlCounters#Counter_Edition
 */

/** Top structure of counters IOVar buffer */
typedef struct {
	uint16	version;	/**< see definition of WL_CNT_T_VERSION */
	uint16	datalen;	/**< length of data including all paddings. */
	uint8   data [];	/**< variable length payload:
				 * 1 or more bcm_xtlv_t type of tuples.
				 * each tuple is padded to multiple of 4 bytes.
				 * 'datalen' field of this structure includes all paddings.
				 */
} wl_cnt_info_t;

/* Top structure of subcounters IOVar buffer
 * Whenever we make any change in this structure
 * WL_SUBCNTR_IOV_VER should be updated accordingly
 * The structure definition should remain consistant b/w
 * FW and wl/WLM app.
 */
typedef struct {
	uint16	version;	  /* Version of IOVAR structure. Used for backward
				   * compatibility in future. Whenever we make any
				   * changes to this structure then value of WL_SUBCNTR_IOV_VER
				   * needs to be updated properly.
				   */
	uint16	length;		  /* length in bytes of this structure */
	uint16	counters_version; /* see definition of WL_CNT_T_VERSION
				   * wl app will send the version of counters
				   * which is used to calculate the offset of counters.
				   * It must match the version of counters FW is using
				   * else FW will return error with his version of counters
				   * set in this field.
				   */
	uint16	num_subcounters;  /* Number of counter offset passed by wl app to FW. */
	uint32	data[1];	  /* variable length payload:
				   * Offsets to the counters will be passed to FW
				   * throught this data field. FW will return the value of counters
				   * at the offsets passed by wl app in this fiels itself.
				   */
} wl_subcnt_info_t;

/* Top structure of counters TLV version IOVar buffer
 * The structure definition should remain consistant b/w
 * FW and wl/WLM app.
 */
typedef struct {
	uint16   version;	/* Version of IOVAR structure. Added for backward
			* compatibility feature. If any changes are done,
			* WL_TLV_IOV_VER need to be updated.
			*/
	uint16   length;	/* total len in bytes of this structure + payload */
	uint16   counters_version;	/* See definition of WL_CNT_VERSION_XTLV
			* wl app will update counter tlv version to be used
			* so to calculate offset of supported TLVs.
			* If there is a mismatch in the version, FW will update an error
			*/
	uint16  num_tlv;	/* Max number of TLV info passed by FW to WL app.
			* and vice-versa
			*/
	uint32   data[];	/* variable length payload:
			* This stores the tlv as supported by F/W to the wl app.
			* This table is required to compute subcounter offsets at WLapp end.
			*/
} wl_cntr_tlv_info_t;

/** wlc layer counters */
typedef struct {
	/* transmit stat counters */
	uint32	txframe;	/**< tx data frames */
	uint32	txbyte;		/**< tx data bytes */
	uint32	txretrans;	/**< tx mac retransmits */
	uint32	txerror;	/**< tx data errors (derived: sum of others) */
	uint32	txctl;		/**< tx management frames */
	uint32	txprshort;	/**< tx short preamble frames */
	uint32	txserr;		/**< tx status errors */
	uint32	txnobuf;	/**< tx out of buffers errors */
	uint32	txnoassoc;	/**< tx discard because we're not associated */
	uint32	txrunt;		/**< tx runt frames */
	uint32	txchit;		/**< tx header cache hit (fastpath) */
	uint32	txcmiss;	/**< tx header cache miss (slowpath) */

	/* transmit chip error counters */
	uint32	txuflo;		/**< tx fifo underflows */
	uint32	txphyerr;	/**< tx phy errors (indicated in tx status) */
	uint32	txphycrs;

	/* receive stat counters */
	uint32	rxframe;	/**< rx data frames */
	uint32	rxbyte;		/**< rx data bytes */
	uint32	rxerror;	/**< rx data errors (derived: sum of others) */
	uint32	rxctl;		/**< rx management frames */
	uint32	rxnobuf;	/**< rx out of buffers errors */
	uint32	rxnondata;	/**< rx non data frames in the data channel errors */
	uint32	rxbadds;	/**< rx bad DS errors */
	uint32	rxbadcm;	/**< rx bad control or management frames */
	uint32	rxfragerr;	/**< rx fragmentation errors */
	uint32	rxrunt;		/**< rx runt frames */
	uint32	rxgiant;	/**< rx giant frames */
	uint32	rxnoscb;	/**< rx no scb error */
	uint32	rxbadproto;	/**< rx invalid frames */
	uint32	rxbadsrcmac;	/**< rx frames with Invalid Src Mac */
	uint32	rxbadda;	/**< rx frames tossed for invalid da */
	uint32	rxfilter;	/**< rx frames filtered out */

	/* receive chip error counters */
	uint32	rxoflo;		/**< rx fifo overflow errors */
	uint32	rxuflo[NFIFO];	/**< rx dma descriptor underflow errors */

	uint32	d11cnt_txrts_off;	/**< d11cnt txrts value when reset d11cnt */
	uint32	d11cnt_rxcrc_off;	/**< d11cnt rxcrc value when reset d11cnt */
	uint32	d11cnt_txnocts_off;	/**< d11cnt txnocts value when reset d11cnt */

	/* misc counters */
	uint32	dmade;		/**< tx/rx dma descriptor errors */
	uint32	dmada;		/**< tx/rx dma data errors */
	uint32	dmape;		/**< tx/rx dma descriptor protocol errors */
	uint32	reset;		/**< reset count */
	uint32	tbtt;		/**< cnts the TBTT int's */
	uint32	txdmawar;
	uint32	pkt_callback_reg_fail;	/**< callbacks register failure */

	/* 802.11 MIB counters, pp. 614 of 802.11 reaff doc. */
	uint32	txfrag;		/**< dot11TransmittedFragmentCount */
	uint32	txmulti;	/**< dot11MulticastTransmittedFrameCount */
	uint32	txfail;		/**< dot11FailedCount */
	uint32	txretry;	/**< dot11RetryCount */
	uint32	txretrie;	/**< dot11MultipleRetryCount */
	uint32	rxdup;		/**< dot11FrameduplicateCount */
	uint32	txrts;		/**< dot11RTSSuccessCount */
	uint32	txnocts;	/**< dot11RTSFailureCount */
	uint32	txnoack;	/**< dot11ACKFailureCount */
	uint32	rxfrag;		/**< dot11ReceivedFragmentCount */
	uint32	rxmulti;	/**< dot11MulticastReceivedFrameCount */
	uint32	rxcrc;		/**< dot11FCSErrorCount */
	uint32	txfrmsnt;	/**< dot11TransmittedFrameCount (bogus MIB?) */
	uint32	rxundec;	/**< dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32	tkipmicfaill;	/**< TKIPLocalMICFailures */
	uint32	tkipcntrmsr;	/**< TKIPCounterMeasuresInvoked */
	uint32	tkipreplay;	/**< TKIPReplays */
	uint32	ccmpfmterr;	/**< CCMPFormatErrors */
	uint32	ccmpreplay;	/**< CCMPReplays */
	uint32	ccmpundec;	/**< CCMPDecryptErrors */
	uint32	fourwayfail;	/**< FourWayHandshakeFailures */
	uint32	wepundec;	/**< dot11WEPUndecryptableCount */
	uint32	wepicverr;	/**< dot11WEPICVErrorCount */
	uint32	decsuccess;	/**< DecryptSuccessCount */
	uint32	tkipicverr;	/**< TKIPICVErrorCount */
	uint32	wepexcluded;	/**< dot11WEPExcludedCount */

	uint32	txchanrej;	/**< Tx frames suppressed due to channel rejection */
	uint32	psmwds;		/**< Count PSM watchdogs */
	uint32	phywatchdog;	/**< Count Phy watchdogs (triggered by ucode) */

	/* MBSS counters, AP only */
	uint32	prq_entries_handled;	/**< PRQ entries read in */
	uint32	prq_undirected_entries;	/**<    which were bcast bss & ssid */
	uint32	prq_bad_entries;	/**<    which could not be translated to info */
	uint32	atim_suppress_count;	/**< TX suppressions on ATIM fifo */
	uint32	bcn_template_not_ready;	/**< Template marked in use on send bcn ... */
	uint32	bcn_template_not_ready_done; /**< ...but "DMA done" interrupt rcvd */
	uint32	late_tbtt_dpc;	/**< TBTT DPC did not happen in time */

	/* per-rate receive stat counters */
	uint32  rx1mbps;	/**< packets rx at 1Mbps */
	uint32  rx2mbps;	/**< packets rx at 2Mbps */
	uint32  rx5mbps5;	/**< packets rx at 5.5Mbps */
	uint32  rx6mbps;	/**< packets rx at 6Mbps */
	uint32  rx9mbps;	/**< packets rx at 9Mbps */
	uint32  rx11mbps;	/**< packets rx at 11Mbps */
	uint32  rx12mbps;	/**< packets rx at 12Mbps */
	uint32  rx18mbps;	/**< packets rx at 18Mbps */
	uint32  rx24mbps;	/**< packets rx at 24Mbps */
	uint32  rx36mbps;	/**< packets rx at 36Mbps */
	uint32  rx48mbps;	/**< packets rx at 48Mbps */
	uint32  rx54mbps;	/**< packets rx at 54Mbps */
	uint32  rx108mbps;	/**< packets rx at 108mbps */
	uint32  rx162mbps;	/**< packets rx at 162mbps */
	uint32  rx216mbps;	/**< packets rx at 216 mbps */
	uint32  rx270mbps;	/**< packets rx at 270 mbps */
	uint32  rx324mbps;	/**< packets rx at 324 mbps */
	uint32  rx378mbps;	/**< packets rx at 378 mbps */
	uint32  rx432mbps;	/**< packets rx at 432 mbps */
	uint32  rx486mbps;	/**< packets rx at 486 mbps */
	uint32  rx540mbps;	/**< packets rx at 540 mbps */

	uint32	rfdisable;	/**< count of radio disables */

	uint32	txexptime;	/**< Tx frames suppressed due to timer expiration */

	uint32	txmpdu_sgi;	/**< count for sgi transmit */
	uint32	rxmpdu_sgi;	/**< count for sgi received */
	uint32	txmpdu_stbc;	/**< count for stbc transmit */
	uint32	rxmpdu_stbc;	/**< count for stbc received */

	uint32	rxundec_mcst;	/**< dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32	tkipmicfaill_mcst;	/**< TKIPLocalMICFailures */
	uint32	tkipcntrmsr_mcst;	/**< TKIPCounterMeasuresInvoked */
	uint32	tkipreplay_mcst;	/**< TKIPReplays */
	uint32	ccmpfmterr_mcst;	/**< CCMPFormatErrors */
	uint32	ccmpreplay_mcst;	/**< CCMPReplays */
	uint32	ccmpundec_mcst;	/**< CCMPDecryptErrors */
	uint32	fourwayfail_mcst;	/**< FourWayHandshakeFailures */
	uint32	wepundec_mcst;	/**< dot11WEPUndecryptableCount */
	uint32	wepicverr_mcst;	/**< dot11WEPICVErrorCount */
	uint32	decsuccess_mcst;	/**< DecryptSuccessCount */
	uint32	tkipicverr_mcst;	/**< TKIPICVErrorCount */
	uint32	wepexcluded_mcst;	/**< dot11WEPExcludedCount */

	uint32	dma_hang;	/**< count for dma hang */
	uint32	reinit;		/**< count for reinit */

	uint32  pstatxucast;	/**< count of ucast frames xmitted on all psta assoc */
	uint32  pstatxnoassoc;	/**< count of txnoassoc frames xmitted on all psta assoc */
	uint32  pstarxucast;	/**< count of ucast frames received on all psta assoc */
	uint32  pstarxbcmc;	/**< count of bcmc frames received on all psta */
	uint32  pstatxbcmc;	/**< count of bcmc frames transmitted on all psta */

	uint32  cso_passthrough; /**< hw cso required but passthrough */
	uint32	cso_normal;	/**< hw cso hdr for normal process */
	uint32	chained;	/**< number of frames chained */
	uint32	chainedsz1;	/**< number of chain size 1 frames */
	uint32	unchained;	/**< number of frames not chained */
	uint32	maxchainsz;	/**< max chain size so far */
	uint32	currchainsz;	/**< current chain size */
	uint32	pciereset;	/**< Secondary Bus Reset issued by driver */
	uint32	cfgrestore;	/**< configspace restore by driver */
	uint32	reinitreason[NREINITREASONCOUNT]; /**< reinitreason counters; 0: Unknown reason */
	uint32	rxrtry;
	uint32  rxmpdu_mu;      /**< Number of MU MPDUs received */

	/* detailed control/management frames */
	uint32	txbar;		/**< Number of TX BAR */
	uint32	rxbar;		/**< Number of RX BAR */
	uint32	txpspoll;	/**< Number of TX PS-poll */
	uint32	rxpspoll;	/**< Number of RX PS-poll */
	uint32	txnull;		/**< Number of TX NULL_DATA */
	uint32	rxnull;		/**< Number of RX NULL_DATA */
	uint32	txqosnull;	/**< Number of TX NULL_QoSDATA */
	uint32	rxqosnull;	/**< Number of RX NULL_QoSDATA */
	uint32	txassocreq;	/**< Number of TX ASSOC request */
	uint32	rxassocreq;	/**< Number of RX ASSOC request */
	uint32	txreassocreq;	/**< Number of TX REASSOC request */
	uint32	rxreassocreq;	/**< Number of RX REASSOC request */
	uint32	txdisassoc;	/**< Number of TX DISASSOC */
	uint32	rxdisassoc;	/**< Number of RX DISASSOC */
	uint32	txassocrsp;	/**< Number of TX ASSOC response */
	uint32	rxassocrsp;	/**< Number of RX ASSOC response */
	uint32	txreassocrsp;	/**< Number of TX REASSOC response */
	uint32	rxreassocrsp;	/**< Number of RX REASSOC response */
	uint32	txauth;		/**< Number of TX AUTH */
	uint32	rxauth;		/**< Number of RX AUTH */
	uint32	txdeauth;	/**< Number of TX DEAUTH */
	uint32	rxdeauth;	/**< Number of RX DEAUTH */
	uint32	txprobereq;	/**< Number of TX probe request */
	uint32	rxprobereq;	/**< Number of RX probe request */
	uint32	txprobersp;	/**< Number of TX probe response */
	uint32	rxprobersp;	/**< Number of RX probe response */
	uint32	txaction;	/**< Number of TX action frame */
	uint32	rxaction;	/**< Number of RX action frame */
	uint32  ampdu_wds;	/**< Number of AMPDU watchdogs */
	uint32  txlost;		/**< Number of lost packets reported in txs */
	uint32	txdatamcast;	/**< Number of TX multicast data packets */
	uint32	txdatabcast;	/**< Number of TX broadcast data packets */
	uint32	psmxwds;	/**< Number of PSMx watchdogs */
	uint32  rxback;
	uint32  txback;
	uint32  p2p_tbtt;	/**< Number of P2P TBTT Events */
	uint32  p2p_tbtt_miss;	/**< Number of P2P TBTT Events Miss */
	uint32	txqueue_start;
	uint32	txqueue_end;
	uint32  txbcast;        /* Broadcast TransmittedFrameCount */
	uint32  txdropped;      /* tx dropped pkts */
	uint32  rxbcast;        /* BroadcastReceivedFrameCount */
	uint32  rxdropped;      /* rx dropped pkts (derived: sum of others) */
	uint32	txq_end_assoccb; /* forced txqueue_end callback fired in assoc */
	uint32	tx_toss_cnt;	/* number of tx packets tossed */
	uint32	rx_toss_cnt;	/* number of rx packets tossed	*/
	uint32	last_tx_toss_rsn; /* reason because of which last tx pkt tossed */
	uint32	last_rx_toss_rsn; /* reason because of which last rx pkt tossed */
	uint32	pmk_badlen_cnt;	/* number of invalid pmk len */
	uint32	txbar_notx;	/* number of TX BAR not sent (maybe supressed or muted) */
	uint32	txbar_noack;	/* number of TX BAR sent, but not acknowledged by peer */
	uint32	rxfrag_agedout;	/**< # of aged out rx fragmentation */

	/* Do not remove or rename in the middle of this struct.
	 * All counter variables have to be of uint32.
	 * Please follow the instruction in
	 * http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/WlCounters#Counter_Edition
	 */
} wl_cnt_wlc_t;

/* he counters Version 1 */
#define HE_COUNTERS_V1		(1)
typedef struct wl_he_cnt_wlc_v1 {
	uint32 he_rxtrig_myaid;
	uint32 he_rxtrig_rand;
	uint32 he_colormiss_cnt;
	uint32 he_txmampdu;
	uint32 he_txmtid_back;
	uint32 he_rxmtid_back;
	uint32 he_rxmsta_back;
	uint32 he_txfrag;
	uint32 he_rxdefrag;
	uint32 he_txtrig;
	uint32 he_rxtrig_basic;
	uint32 he_rxtrig_murts;
	uint32 he_rxtrig_bsrp;
	uint32 he_rxdlmu;
	uint32 he_physu_rx;
	uint32 he_phyru_rx;
	uint32 he_txtbppdu;
} wl_he_cnt_wlc_v1_t;

/* he counters Version 2 */
#define HE_COUNTERS_V2		(2)
typedef struct wl_he_cnt_wlc_v2 {
	uint16 version;
	uint16 len;
	uint32 he_rxtrig_myaid; /**< rxed valid trigger frame with myaid */
	uint32 he_rxtrig_rand; /**< rxed valid trigger frame with random aid */
	uint32 he_colormiss_cnt; /**< for bss color mismatch cases */
	uint32 he_txmampdu; /**< for multi-TID AMPDU transmission */
	uint32 he_txmtid_back; /**< for multi-TID BACK transmission */
	uint32 he_rxmtid_back; /**< reception of multi-TID BACK */
	uint32 he_rxmsta_back; /**< reception of multi-STA BACK */
	uint32 he_txfrag; /**< transmission of Dynamic fragmented packets */
	uint32 he_rxdefrag; /**< reception of dynamic fragmented packets */
	uint32 he_txtrig; /**< transmission of trigger frames */
	uint32 he_rxtrig_basic; /**< reception of basic trigger frame */
	uint32 he_rxtrig_murts; /**< reception of MU-RTS trigger frame */
	uint32 he_rxtrig_bsrp; /**< reception of BSR poll trigger frame */
	uint32 he_rxdlmu; /**< reception of DL MU PPDU */
	uint32 he_physu_rx; /**< reception of SU frame */
	uint32 he_phyru_rx; /**< reception of RU frame */
	uint32 he_txtbppdu; /**< increments on transmission of every TB PPDU */
	uint32 he_null_tbppdu; /**< null TB PPDU's sent as a response to basic trigger frame */
} wl_he_cnt_wlc_v2_t;

/* he counters Version 3 */
#define WL_RU_TYPE_MAX			6
#define HE_COUNTERS_V3		(3)

typedef struct wl_he_cnt_wlc_v3 {
	uint16 version;
	uint16 len;
	uint32 he_rxtrig_myaid; /**< rxed valid trigger frame with myaid */
	uint32 he_rxtrig_rand; /**< rxed valid trigger frame with random aid */
	uint32 he_colormiss_cnt; /**< for bss color mismatch cases */
	uint32 he_txmampdu; /**< for multi-TID AMPDU transmission */
	uint32 he_txmtid_back; /**< for multi-TID BACK transmission */
	uint32 he_rxmtid_back; /**< reception of multi-TID BACK */
	uint32 he_rxmsta_back; /**< reception of multi-STA BACK */
	uint32 he_txfrag; /**< transmission of Dynamic fragmented packets */
	uint32 he_rxdefrag; /**< reception of dynamic fragmented packets */
	uint32 he_txtrig; /**< transmission of trigger frames */
	uint32 he_rxtrig_basic; /**< reception of basic trigger frame */
	uint32 he_rxtrig_murts; /**< reception of MU-RTS trigger frame */
	uint32 he_rxtrig_bsrp; /**< reception of BSR poll trigger frame */
	uint32 he_rxhemuppdu_cnt; /**< rxing HE MU PPDU */
	uint32 he_physu_rx; /**< reception of SU frame */
	uint32 he_phyru_rx; /**< reception of RU frame */
	uint32 he_txtbppdu; /**< increments on transmission of every TB PPDU */
	uint32 he_null_tbppdu; /**< null TB PPDU's sent as a response to basic trigger frame */
	uint32 he_rxhesuppdu_cnt; /**< rxing SU PPDU */
	uint32 he_rxhesureppdu_cnt; /**< rxing Range Extension(RE) SU PPDU */
	uint32 he_null_zero_agg; /**< null AMPDU's transmitted in response to basic trigger
				 * because of zero aggregation
				 */
	uint32 he_null_bsrp_rsp; /**< null AMPDU's txed in response to BSR poll */
	uint32 he_null_fifo_empty; /**< null AMPDU's in response to basic trigger
				 * because of no frames in fifo's
				 */
	uint32 he_myAID_cnt;
	uint32 he_rxtrig_bfm_cnt;
	uint32 he_rxtrig_mubar;
	uint32 rxheru[WL_RU_TYPE_MAX];		/**< HE of rx pkts */
	uint32 txheru[WL_RU_TYPE_MAX];
	uint32 he_mgmt_tbppdu;
	uint32 he_cs_req_tx_cancel;
	uint32 he_wrong_nss;
	uint32 he_trig_unsupp_rate;
	uint32 he_rxtrig_nfrp;
	uint32 he_rxtrig_bqrp;
	uint32 he_rxtrig_gcrmubar;
} wl_he_cnt_wlc_v3_t;

/* he counters Version 4 */
#define HE_COUNTERS_V4		(4)
typedef struct wl_he_cnt_wlc_v4 {
	uint16 version;
	uint16 len;
	uint32 he_rxtrig_myaid; /**< rxed valid trigger frame with myaid */
	uint32 he_rxtrig_rand; /**< rxed valid trigger frame with random aid */
	uint32 he_colormiss_cnt; /**< for bss color mismatch cases */
	uint32 he_txmampdu; /**< for multi-TID AMPDU transmission */
	uint32 he_txmtid_back; /**< for multi-TID BACK transmission */
	uint32 he_rxmtid_back; /**< reception of multi-TID BACK */
	uint32 he_rxmsta_back; /**< reception of multi-STA BACK */
	uint32 he_txfrag; /**< transmission of Dynamic fragmented packets */
	uint32 he_rxdefrag; /**< reception of dynamic fragmented packets */
	uint32 he_txtrig; /**< transmission of trigger frames */
	uint32 he_rxtrig_basic; /**< reception of basic trigger frame */
	uint32 he_rxtrig_murts; /**< reception of MU-RTS trigger frame */
	uint32 he_rxtrig_bsrp; /**< reception of BSR poll trigger frame */
	uint32 he_rxtsrt_hemuppdu_cnt; /**< rxing HE MU PPDU */
	uint32 he_physu_rx; /**< reception of SU frame */
	uint32 he_phyru_rx; /**< reception of RU frame */
	uint32 he_txtbppdu; /**< increments on transmission of every TB PPDU */
	uint32 he_null_tbppdu; /**< null TB PPDU's sent as a response to basic trigger frame */
	uint32 he_rxstrt_hesuppdu_cnt; /**< rxing SU PPDU */
	uint32 he_rxstrt_hesureppdu_cnt; /**< rxing Range Extension(RE) SU PPDU */
	uint32 he_null_zero_agg; /**< null AMPDU's transmitted in response to basic trigger
				 * because of zero aggregation
				 */
	uint32 he_null_bsrp_rsp; /**< null AMPDU's txed in response to BSR poll */
	uint32 he_null_fifo_empty; /**< null AMPDU's in response to basic trigger
				 * because of no frames in fifo's
				 */
	uint32 he_myAID_cnt;
	uint32 he_rxtrig_bfm_cnt;
	uint32 he_rxtrig_mubar;
	uint32 rxheru[WL_RU_TYPE_MAX];		/**< HE of rx pkts */
	uint32 txheru[WL_RU_TYPE_MAX];
	uint32 he_mgmt_tbppdu;
	uint32 he_cs_req_tx_cancel;
	uint32 he_wrong_nss;
	uint32 he_trig_unsupp_rate;
	uint32 he_rxtrig_nfrp;
	uint32 he_rxtrig_bqrp;
	uint32 he_rxtrig_gcrmubar;
	uint32 he_rxtrig_basic_htpack; /**< triggers received with HTP ack policy */
	uint32 he_rxtrig_ed_cncl;	/**< count of cancelled packets
					 * becasue of cs_req in trigger frame
					 */
	uint32 he_rxtrig_suppr_null_tbppdu; /**<  count of null frame sent becasue of
					 * suppression scenarios
					 */
	uint32 he_ulmu_disable;		/**< number of UL MU disable scenario's handled in ucode */
	uint32 he_ulmu_data_disable;	/**<number of UL MU data disable scenarios
					 * handled in ucode
					 */
} wl_he_cnt_wlc_v4_t;

#ifndef HE_COUNTERS_VERSION_ENABLED
#define HE_COUNTERS_VERSION	(HE_COUNTERS_V1)
typedef wl_he_cnt_wlc_v1_t wl_he_cnt_wlc_t;
#endif /* HE_COUNTERS_VERSION_ENABLED */

/* he omi counters Version 1 */
#define HE_OMI_COUNTERS_V1		(1)
typedef struct wl_he_omi_cnt_wlc_v1 {
	uint16 version;
	uint16 len;
	uint32 he_omitx_sched;          /* Count for total number of OMIs scheduled */
	uint32 he_omitx_success;        /* Count for OMI Tx success */
	uint32 he_omitx_retries;        /* Count for OMI retries as TxDone not set */
	uint32 he_omitx_dur;            /* Accumulated duration of OMI completion time */
	uint32 he_omitx_ulmucfg;        /* count for UL MU enable/disable change req */
	uint32 he_omitx_ulmucfg_ack;    /* count for UL MU enable/disable req txed successfully */
	uint32 he_omitx_txnsts;         /* count for Txnsts change req */
	uint32 he_omitx_txnsts_ack;     /* count for Txnsts change req txed successfully */
	uint32 he_omitx_rxnss;          /* count for Rxnss change req */
	uint32 he_omitx_rxnss_ack;      /* count for Rxnss change req txed successfully */
	uint32 he_omitx_bw;             /* count for BW change req */
	uint32 he_omitx_bw_ack;         /* count for BW change req txed successfully */
	uint32 he_omitx_ersudis;        /* count for ER SU enable/disable req */
	uint32 he_omitx_ersudis_ack;    /* count for ER SU enable/disable req txed successfully */
	uint32 he_omitx_dlmursdrec;	/* count for Resound recommendation change req */
	uint32 he_omitx_dlmursdrec_ack;	/* count for Resound recommendation req txed successfully */
} wl_he_omi_cnt_wlc_v1_t;

/* WL_IFSTATS_XTLV_WL_SLICE_TXBF */
/* beamforming counters version 1 */
#define TXBF_ECOUNTERS_V1	(1u)
#define WL_TXBF_CNT_ARRAY_SZ	(8u)
typedef struct wl_txbf_ecounters_v1 {
	uint16 version;
	uint16 len;
	/* transmit beamforming stats */
	uint16 txndpa;				/* null data packet announcements */
	uint16 txndp;				/* null data packets */
	uint16 txbfpoll;			/* beamforming report polls */
	uint16 txsf;				/* subframes */
	uint16 txcwrts;				/* contention window rts */
	uint16 txcwcts;				/* contention window cts */
	uint16 txbfm;
	/* receive beamforming stats */
	uint16 rxndpa_u;			/* unicast NDPAs */
	uint16 rxndpa_m;			/* multicast NDPAs */
	uint16 rxbfpoll;			/* unicast bf-polls */
	uint16 bferpt;				/* beamforming reports */
	uint16 rxsf;
	uint16 rxcwrts;
	uint16 rxcwcts;
	uint16 rxtrig_bfpoll;
	uint16 unused_uint16;			/* pad */
	/* sounding stats - interval capture */
	uint16 rxnontb_sound[WL_TXBF_CNT_ARRAY_SZ];	/* non-TB sounding for last 8 captures */
	uint16 rxtb_sound[WL_TXBF_CNT_ARRAY_SZ];	/* TB sounding count for last 8 captures */
	uint32 cap_dur_ms[WL_TXBF_CNT_ARRAY_SZ];	/* last 8 capture durations (in ms) */
	uint32 cap_last_ts;			/* timestamp of last sample capture */
} wl_txbf_ecounters_v1_t;

/* security vulnerabilities counters */
typedef struct {
	uint32	ie_unknown;		/* number of unknown IEs */
	uint32	ie_invalid_length;	/* number of IEs with invalid length */
	uint32	ie_invalid_data;	/* number of IEs with invalid data */
	uint32	ipv6_invalid_length;	/* number of IPv6 packets with invalid payload length */
} wl_secvln_cnt_t;

/* Reinit reasons - do not put anything else other than reinit reasons here */
typedef struct {
	uint32 rsn[WL_REINIT_RC_LAST];
} reinit_rsns_t;

/* MACXSTAT counters for ucodex (corerev >= 64) */
typedef struct {
	uint32 macxsusp;
	uint32 m2vmsg;
	uint32 v2mmsg;
	uint32 mboxout;
	uint32 musnd;
	uint32 sfb2v;
} wl_cnt_ge64mcxst_v1_t;

/** MACSTAT counters for ucode (corerev >= 40) */
typedef struct {
	/* MAC counters: 32-bit version of d11.h's macstat_t */
	uint32	txallfrm;	/**< total number of frames sent, incl. Data, ACK, RTS, CTS,
				 * Control Management (includes retransmissions)
				 */
	uint32	txrtsfrm;	/**< number of RTS sent out by the MAC */
	uint32	txctsfrm;	/**< number of CTS sent out by the MAC */
	uint32	txackfrm;	/**< number of ACK frames sent out */
	uint32	txdnlfrm;	/**< number of Null-Data transmission generated from template  */
	uint32	txbcnfrm;	/**< beacons transmitted */
	uint32	txfunfl[6];	/**< per-fifo tx underflows */
	uint32	txampdu;	/**< number of AMPDUs transmitted */
	uint32	txmpdu;		/**< number of MPDUs transmitted */
	uint32	txtplunfl;	/**< Template underflows (mac was too slow to transmit ACK/CTS
				 * or BCN)
				 */
	uint32	txphyerror;	/**< Transmit phy error, type of error is reported in tx-status for
				 * driver enqueued frames
				 */
	uint32  pktengrxducast; /**< unicast frames rxed by the pkteng code */
	uint32  pktengrxdmcast; /**< multicast frames rxed by the pkteng code */
	uint32	rxfrmtoolong;	/**< Received frame longer than legal limit (2346 bytes) */
	uint32	rxfrmtooshrt; /**< Received frame did not contain enough bytes for its frame type */
	uint32	rxanyerr;	/**< Any RX error that is not counted by other counters. */
	uint32	rxbadfcs;	/**< number of frames for which the CRC check failed in the MAC */
	uint32	rxbadplcp;	/**< parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/**< PHY was able to correlate the preamble but not the header */
	uint32	rxstrt;		/**< Number of received frames with a good PLCP
				 * (i.e. passing parity check)
				 */
	uint32	rxdtucastmbss; /**< number of received DATA frames with good FCS and matching RA */
	uint32	rxmgucastmbss; /**< number of received mgmt frames with good FCS and matching RA */
	uint32	rxctlucast; /**< number of received CNTRL frames with good FCS and matching RA */
	uint32	rxrtsucast;	/**< number of unicast RTS addressed to the MAC (good FCS) */
	uint32	rxctsucast;	/**< number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxackucast;	/**< number of ucast ACKS received (good FCS) */
	uint32	rxdtocast; /**< number of received DATA frames (good FCS and not matching RA) */
	uint32	rxmgocast; /**< number of received MGMT frames (good FCS and not matching RA) */
	uint32	rxctlocast; /**< number of received CNTRL frame (good FCS and not matching RA) */
	uint32	rxrtsocast;	/**< number of received RTS not addressed to the MAC */
	uint32	rxctsocast;	/**< number of received CTS not addressed to the MAC */
	uint32	rxdtmcast;	/**< number of RX Data multicast frames received by the MAC */
	uint32	rxmgmcast;	/**< number of RX Management multicast frames received by the MAC */
	uint32	rxctlmcast;	/**< number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32	rxbeaconmbss;	/**< beacons received from member of BSS */
	uint32	rxdtucastobss; /**< number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32	rxbeaconobss;	/**< beacons received from other BSS */
	uint32	rxrsptmout;	/**< number of response timeouts for transmitted frames
				 * expecting a response
				 */
	uint32	bcntxcancl;	/**< transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32	rxnodelim;	/**< number of no valid delimiter detected by ampdu parser */
	uint32	rxf0ovfl;	/**< number of receive fifo 0 overflows */
	uint32	rxf1ovfl;	/**< number of receive fifo 1 overflows */
	uint32	rxhlovfl;	/**< number of length / header fifo overflows */
	uint32	missbcn_dbg;	/**< number of beacon missed to receive */
	uint32	pmqovfl;	/**< number of PMQ overflows */
	uint32	rxcgprqfrm;	/**< number of received Probe requests that made it into
				 * the PRQ fifo
				 */
	uint32	rxcgprsqovfl;	/**< Rx Probe Request Que overflow in the AP */
	uint32	txcgprsfail;	/**< Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32	txcgprssuc;	/**< Tx Probe Response Success (ACK was received) */
	uint32	prs_timeout;	/**< number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32	txrtsfail;	/**< number of rts transmission failure that reach retry limit */
	uint32	txucast;	/**< number of unicast tx expecting response other than cts/cwcts */
	uint32  txinrtstxop;	/**< number of data frame transmissions during rts txop */
	uint32	rxback;		/**< blockack rxcnt */
	uint32	txback;		/**< blockack txcnt */
	uint32	bphy_rxcrsglitch;	/**< PHY count of bphy glitches */
	uint32	rxdrop20s;	/**< drop secondary cnt */
	uint32	rxtoolate;	/**< receive too late */
	uint32  bphy_badplcp;	/**< number of bad PLCP reception on BPHY rate */
	/* All counter variables have to be of uint32. */
} wl_cnt_ge40mcst_v1_t;

/** MACSTAT counters for ucode (corerev < 40) */
typedef struct {
	/* MAC counters: 32-bit version of d11.h's macstat_t */
	uint32	txallfrm;	/**< total number of frames sent, incl. Data, ACK, RTS, CTS,
				 * Control Management (includes retransmissions)
				 */
	uint32	txrtsfrm;	/**< number of RTS sent out by the MAC */
	uint32	txctsfrm;	/**< number of CTS sent out by the MAC */
	uint32	txackfrm;	/**< number of ACK frames sent out */
	uint32	txdnlfrm;	/**< number of Null-Data transmission generated from template  */
	uint32	txbcnfrm;	/**< beacons transmitted */
	uint32	txfunfl[6];	/**< per-fifo tx underflows */
	uint32	txampdu;	/**< number of AMPDUs transmitted */
	uint32	txmpdu;		/**< number of MPDUs transmitted */
	uint32	txtplunfl;	/**< Template underflows (mac was too slow to transmit ACK/CTS
				 * or BCN)
				 */
	uint32	txphyerror;	/**< Transmit phy error, type of error is reported in tx-status for
				 * driver enqueued frames
				 */
	uint32  pktengrxducast; /**< unicast frames rxed by the pkteng code */
	uint32  pktengrxdmcast; /**< multicast frames rxed by the pkteng code */
	uint32	rxfrmtoolong;	/**< Received frame longer than legal limit (2346 bytes) */
	uint32	rxfrmtooshrt; /**< Received frame did not contain enough bytes for its frame type */
	uint32	rxanyerr;	/**< Any RX error that is not counted by other counters. */
	uint32	rxbadfcs;	/**< number of frames for which the CRC check failed in the MAC */
	uint32	rxbadplcp;	/**< parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/**< PHY was able to correlate the preamble but not the header */
	uint32	rxstrt;		/**< Number of received frames with a good PLCP
				 * (i.e. passing parity check)
				 */
	uint32	rxdtucastmbss; /**< number of received DATA frames with good FCS and matching RA */
	uint32	rxmgucastmbss; /**< number of received mgmt frames with good FCS and matching RA */
	uint32	rxctlucast; /**< number of received CNTRL frames with good FCS and matching RA */
	uint32	rxrtsucast;	/**< number of unicast RTS addressed to the MAC (good FCS) */
	uint32	rxctsucast;	/**< number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxackucast;	/**< number of ucast ACKS received (good FCS) */
	uint32	rxdtocast;  /**< number of received DATA frames (good FCS and not matching RA) */
	uint32	rxmgocast;  /**< number of received MGMT frames (good FCS and not matching RA) */
	uint32	rxctlocast; /**< number of received CNTRL frame (good FCS and not matching RA) */
	uint32	rxrtsocast;	/**< number of received RTS not addressed to the MAC */
	uint32	rxctsocast;	/**< number of received CTS not addressed to the MAC */
	uint32	rxdtmcast;	/**< number of RX Data multicast frames received by the MAC */
	uint32	rxmgmcast;	/**< number of RX Management multicast frames received by the MAC */
	uint32	rxctlmcast;	/**< number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32	rxbeaconmbss;	/**< beacons received from member of BSS */
	uint32	rxdtucastobss; /**< number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32	rxbeaconobss;	/**< beacons received from other BSS */
	uint32	rxrsptmout;	/**< number of response timeouts for transmitted frames
				 * expecting a response
				 */
	uint32	bcntxcancl;	/**< transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32	rxnodelim;	/**< number of no valid delimiter detected by ampdu parser */
	uint32	rxf0ovfl;	/**< number of receive fifo 0 overflows */
	uint32	dbgoff46;
	uint32	dbgoff47;
	uint32	dbgoff48;	/**< Used for counting txstatus queue overflow (corerev <= 4)  */
	uint32	pmqovfl;	/**< number of PMQ overflows */
	uint32	rxcgprqfrm;	/**< number of received Probe requests that made it into
				 * the PRQ fifo
				 */
	uint32	rxcgprsqovfl;	/**< Rx Probe Request Que overflow in the AP */
	uint32	txcgprsfail;	/**< Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32	txcgprssuc;	/**< Tx Probe Response Success (ACK was received) */
	uint32	prs_timeout;	/**< number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32	txrtsfail;	/**< number of rts transmission failure that reach retry limit */
	uint32	txucast;	/**< number of unicast tx expecting response other than cts/cwcts */
	uint32  txinrtstxop;	/**< number of data frame transmissions during rts txop */
	uint32	rxback;		/**< blockack rxcnt */
	uint32	txback;		/**< blockack txcnt */
	uint32	bphy_rxcrsglitch;	/**< PHY count of bphy glitches */
	uint32	phywatch;
	uint32	rxtoolate;	/**< receive too late */
	uint32  bphy_badplcp;	/**< number of bad PLCP reception on BPHY rate */
	/* All counter variables have to be of uint32. */
} wl_cnt_lt40mcst_v1_t;

/** MACSTAT counters for ucode (corerev >= 80) */
typedef struct {
	/* MAC counters: 32-bit version of d11.h's macstat_t */
	/* Start of PSM2HOST stats(72) block */
	uint32	txallfrm;	/**< total number of frames sent, incl. Data, ACK, RTS, CTS,
				 * Control Management (includes retransmissions)
				 */
	uint32	txrtsfrm;	/**< number of RTS sent out by the MAC */
	uint32	txctsfrm;	/**< number of CTS sent out by the MAC */
	uint32	txackfrm;	/**< number of ACK frames sent out */
	uint32	txdnlfrm;	/**< number of Null-Data transmission generated from template  */
	uint32	txbcnfrm;	/**< beacons transmitted */
	uint32	txampdu;	/**< number of AMPDUs transmitted */
	uint32	txmpdu;		/**< number of MPDUs transmitted */
	uint32	txtplunfl;	/**< Template underflows (mac was too slow to transmit ACK/CTS
				 * or BCN)
				 */
	uint32	txphyerror;	/**< Transmit phy error, type of error is reported in tx-status for
				 * driver enqueued frames
				 */
	uint32  pktengrxducast; /**< unicast frames rxed by the pkteng code */
	uint32  pktengrxdmcast; /**< multicast frames rxed by the pkteng code */
	uint32	rxfrmtoolong;	/**< Received frame longer than legal limit (2346 bytes) */
	uint32	rxfrmtooshrt; /**< Received frame did not contain enough bytes for its frame type */
	uint32	rxanyerr;	/**< Any RX error that is not counted by other counters. */
	uint32	rxbadfcs;	/**< number of frames for which the CRC check failed in the MAC */
	uint32	rxbadplcp;	/**< parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/**< PHY was able to correlate the preamble but not the header */
	uint32	rxstrt;		/**< Number of received frames with a good PLCP
				 * (i.e. passing parity check)
				 */
	uint32	rxdtucastmbss; /**< number of received DATA frames with good FCS and matching RA */
	uint32	rxmgucastmbss; /**< number of received mgmt frames with good FCS and matching RA */
	uint32	rxctlucast; /**< number of received CNTRL frames with good FCS and matching RA */
	uint32	rxrtsucast;	/**< number of unicast RTS addressed to the MAC (good FCS) */
	uint32	rxctsucast;	/**< number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxackucast;	/**< number of ucast ACKS received (good FCS) */
	uint32	rxdtocast; /**< number of received DATA frames (good FCS and not matching RA) */
	uint32	rxmgocast; /**< number of received MGMT frames (good FCS and not matching RA) */
	uint32	rxctlocast; /**< number of received CNTRL frame (good FCS and not matching RA) */
	uint32	rxrtsocast;	/**< number of received RTS not addressed to the MAC */
	uint32	rxctsocast;	/**< number of received CTS not addressed to the MAC */
	uint32	rxdtmcast;	/**< number of RX Data multicast frames received by the MAC */
	uint32	rxmgmcast;	/**< number of RX Management multicast frames received by the MAC */
	uint32	rxctlmcast;	/**< number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32	rxbeaconmbss;	/**< beacons received from member of BSS */
	uint32	rxdtucastobss; /**< number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32	rxbeaconobss;	/**< beacons received from other BSS */
	uint32	rxrsptmout;	/**< number of response timeouts for transmitted frames
				 * expecting a response
				 */
	uint32	bcntxcancl;	/**< transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32	rxnodelim;	/**< number of no valid delimiter detected by ampdu parser */
	uint32	missbcn_dbg;	/**< number of beacon missed to receive */
	uint32	pmqovfl;	/**< number of PMQ overflows */
	uint32	rxcgprqfrm;	/**< number of received Probe requests that made it into
				 * the PRQ fifo
				 */
	uint32	rxcgprsqovfl;	/**< Rx Probe Request Que overflow in the AP */
	uint32	txcgprsfail;	/**< Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32	txcgprssuc;	/**< Tx Probe Response Success (ACK was received) */
	uint32	prs_timeout;	/**< number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32	txrtsfail;	/**< number of rts transmission failure that reach retry limit */
	uint32	txucast;	/**< number of unicast tx expecting response other than cts/cwcts */
	uint32  txinrtstxop;	/**< number of data frame transmissions during rts txop */
	uint32	rxback;		/**< blockack rxcnt */
	uint32	txback;		/**< blockack txcnt */
	uint32	bphy_rxcrsglitch;	/**< PHY count of bphy glitches */
	uint32	rxdrop20s;	/**< drop secondary cnt */
	uint32	rxtoolate;	/**< receive too late */
	uint32  bphy_badplcp;	/**< number of bad PLCP reception on BPHY rate */
	uint32	rxtrig_myaid;	/* New counters added in corerev 80 */
	uint32	rxtrig_rand;
	uint32	goodfcs;
	uint32	colormiss;
	uint32	txmampdu;
	uint32	rxmtidback;
	uint32	rxmstaback;
	uint32	txfrag;
	/* End of PSM2HOST stats block */
	/* start of rxerror overflow counter(24) block which are modified/added in corerev 80 */
	uint32	phyovfl;
	uint32	rxf0ovfl;	/**< number of receive fifo 0 overflows */
	uint32	rxf1ovfl;	/**< number of receive fifo 1 overflows */
	uint32	lenfovfl;
	uint32	weppeof;
	uint32	badplcp;
	uint32	msduthresh;
	uint32	strmeof;
	uint32	stsfifofull;
	uint32	stsfifoerr;
	uint32	PAD[6];
	uint32	rxerr_stat;
	uint32	ctx_fifo_full;
	uint32	PAD0[9];
	uint32	ctmode_ufc_cnt;
	uint32	PAD1[28]; /* PAD added for counter elements to be added soon */
} wl_cnt_ge80mcst_v1_t;

typedef struct {
	uint32 fifocount;
	uint32 txfunfl[];
} wl_cnt_ge80_txfunfl_v1_t;

/** MACSTAT counters for "wl counter" version <= 10 */
/*  With ucode before its macstat cnts cleaned up */
typedef struct {
	/* MAC counters: 32-bit version of d11.h's macstat_t */
	uint32	txallfrm;	/**< total number of frames sent, incl. Data, ACK, RTS, CTS,
				 * Control Management (includes retransmissions)
				 */
	uint32	txrtsfrm;	/**< number of RTS sent out by the MAC */
	uint32	txctsfrm;	/**< number of CTS sent out by the MAC */
	uint32	txackfrm;	/**< number of ACK frames sent out */
	uint32	txdnlfrm;	/**< number of Null-Data transmission generated from template  */
	uint32	txbcnfrm;	/**< beacons transmitted */
	uint32	txfunfl[6];	/**< per-fifo tx underflows */
	uint32	txfbw;		/**< transmit at fallback bw (dynamic bw) */
	uint32	PAD0;		/**< number of MPDUs transmitted */
	uint32	txtplunfl;	/**< Template underflows (mac was too slow to transmit ACK/CTS
				 * or BCN)
				 */
	uint32	txphyerror;	/**< Transmit phy error, type of error is reported in tx-status for
				 * driver enqueued frames
				 */
	uint32  pktengrxducast; /**< unicast frames rxed by the pkteng code */
	uint32  pktengrxdmcast; /**< multicast frames rxed by the pkteng code */
	uint32	rxfrmtoolong;	/**< Received frame longer than legal limit (2346 bytes) */
	uint32	rxfrmtooshrt; /**< Received frame did not contain enough bytes for its frame type */
	uint32	rxinvmachdr;	/**< Either the protocol version != 0 or frame type not
				 * data/control/management
				 */
	uint32	rxbadfcs;	/**< number of frames for which the CRC check failed in the MAC */
	uint32	rxbadplcp;	/**< parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/**< PHY was able to correlate the preamble but not the header */
	uint32	rxstrt;		/**< Number of received frames with a good PLCP
				 * (i.e. passing parity check)
				 */
	uint32	rxdfrmucastmbss; /* number of received DATA frames with good FCS and matching RA */
	uint32	rxmfrmucastmbss; /* number of received mgmt frames with good FCS and matching RA */
	uint32	rxcfrmucast; /**< number of received CNTRL frames with good FCS and matching RA */
	uint32	rxrtsucast;  /**< number of unicast RTS addressed to the MAC (good FCS) */
	uint32	rxctsucast;  /**< number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxackucast;	/**< number of ucast ACKS received (good FCS) */
	uint32	rxdfrmocast; /**< number of received DATA frames (good FCS and not matching RA) */
	uint32	rxmfrmocast; /**< number of received MGMT frames (good FCS and not matching RA) */
	uint32	rxcfrmocast; /**< number of received CNTRL frame (good FCS and not matching RA) */
	uint32	rxrtsocast;	/**< number of received RTS not addressed to the MAC */
	uint32	rxctsocast;	/**< number of received CTS not addressed to the MAC */
	uint32	rxdfrmmcast;	/**< number of RX Data multicast frames received by the MAC */
	uint32	rxmfrmmcast;	/**< number of RX Management multicast frames received by the MAC */
	uint32	rxcfrmmcast;	/**< number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32	rxbeaconmbss;	/**< beacons received from member of BSS */
	uint32	rxdfrmucastobss; /**< number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32	rxbeaconobss;	/**< beacons received from other BSS */
	uint32	rxrsptmout;	/**< number of response timeouts for transmitted frames
				 * expecting a response
				 */
	uint32	bcntxcancl;	/**< transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32	PAD1;
	uint32	rxf0ovfl;	/**< number of receive fifo 0 overflows */
	uint32	rxf1ovfl;	/**< Number of receive fifo 1 overflows (obsolete) */
	uint32	rxf2ovfl;	/**< Number of receive fifo 2 overflows (obsolete) */
	uint32	txsfovfl;	/**< Number of transmit status fifo overflows (obsolete) */
	uint32	pmqovfl;	/**< number of PMQ overflows */
	uint32	rxcgprqfrm;	/**< number of received Probe requests that made it into
				 * the PRQ fifo
				 */
	uint32	rxcgprsqovfl;	/**< Rx Probe Request Que overflow in the AP */
	uint32	txcgprsfail;	/**< Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32	txcgprssuc;	/**< Tx Probe Response Success (ACK was received) */
	uint32	prs_timeout;	/**< number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32	rxnack;		/**< obsolete */
	uint32	frmscons;	/**< obsolete */
	uint32  txnack;		/**< obsolete */
	uint32	rxback;		/**< blockack rxcnt */
	uint32	txback;		/**< blockack txcnt */
	uint32	bphy_rxcrsglitch;	/**< PHY count of bphy glitches */
	uint32	rxdrop20s;	/**< drop secondary cnt */
	uint32	rxtoolate;	/**< receive too late */
	uint32  bphy_badplcp;	/**< number of bad PLCP reception on BPHY rate */
	/* All counter variables have to be of uint32. */
} wl_cnt_v_le10_mcst_t;

#define MAX_RX_FIFO 3
#define WL_RXFIFO_CNT_VERSION  1   /* current version of wl_rxfifo_cnt_t */
typedef struct {
	/* Counters for frames received from rx fifos */
	uint16	version;
	uint16	length;		/* length of entire structure */
	uint32	rxf_data[MAX_RX_FIFO];		/* data frames from rx fifo */
	uint32	rxf_mgmtctl[MAX_RX_FIFO];	/* mgmt/ctl frames from rx fifo */
} wl_rxfifo_cnt_t;

typedef struct {
	uint16	version;	/**< see definition of WL_CNT_T_VERSION */
	uint16	length;		/**< length of entire structure */

	/* transmit stat counters */
	uint32	txframe;	/**< tx data frames */
	uint32	txbyte;		/**< tx data bytes */
	uint32	txretrans;	/**< tx mac retransmits */
	uint32	txerror;	/**< tx data errors (derived: sum of others) */
	uint32	txctl;		/**< tx management frames */
	uint32	txprshort;	/**< tx short preamble frames */
	uint32	txserr;		/**< tx status errors */
	uint32	txnobuf;	/**< tx out of buffers errors */
	uint32	txnoassoc;	/**< tx discard because we're not associated */
	uint32	txrunt;		/**< tx runt frames */
	uint32	txchit;		/**< tx header cache hit (fastpath) */
	uint32	txcmiss;	/**< tx header cache miss (slowpath) */

	/* transmit chip error counters */
	uint32	txuflo;		/**< tx fifo underflows */
	uint32	txphyerr;	/**< tx phy errors (indicated in tx status) */
	uint32	txphycrs;

	/* receive stat counters */
	uint32	rxframe;	/**< rx data frames */
	uint32	rxbyte;		/**< rx data bytes */
	uint32	rxerror;	/**< rx data errors (derived: sum of others) */
	uint32	rxctl;		/**< rx management frames */
	uint32	rxnobuf;	/**< rx out of buffers errors */
	uint32	rxnondata;	/**< rx non data frames in the data channel errors */
	uint32	rxbadds;	/**< rx bad DS errors */
	uint32	rxbadcm;	/**< rx bad control or management frames */
	uint32	rxfragerr;	/**< rx fragmentation errors */
	uint32	rxrunt;		/**< rx runt frames */
	uint32	rxgiant;	/**< rx giant frames */
	uint32	rxnoscb;	/**< rx no scb error */
	uint32	rxbadproto;	/**< rx invalid frames */
	uint32	rxbadsrcmac;	/**< rx frames with Invalid Src Mac */
	uint32	rxbadda;	/**< rx frames tossed for invalid da */
	uint32	rxfilter;	/**< rx frames filtered out */

	/* receive chip error counters */
	uint32	rxoflo;		/**< rx fifo overflow errors */
	uint32	rxuflo[NFIFO];	/**< rx dma descriptor underflow errors */

	uint32	d11cnt_txrts_off;	/**< d11cnt txrts value when reset d11cnt */
	uint32	d11cnt_rxcrc_off;	/**< d11cnt rxcrc value when reset d11cnt */
	uint32	d11cnt_txnocts_off;	/**< d11cnt txnocts value when reset d11cnt */

	/* misc counters */
	uint32	dmade;		/**< tx/rx dma descriptor errors */
	uint32	dmada;		/**< tx/rx dma data errors */
	uint32	dmape;		/**< tx/rx dma descriptor protocol errors */
	uint32	reset;		/**< reset count */
	uint32	tbtt;		/**< cnts the TBTT int's */
	uint32	txdmawar;
	uint32	pkt_callback_reg_fail;	/**< callbacks register failure */

	/* MAC counters: 32-bit version of d11.h's macstat_t */
	uint32	txallfrm;	/**< total number of frames sent, incl. Data, ACK, RTS, CTS,
				 * Control Management (includes retransmissions)
				 */
	uint32	txrtsfrm;	/**< number of RTS sent out by the MAC */
	uint32	txctsfrm;	/**< number of CTS sent out by the MAC */
	uint32	txackfrm;	/**< number of ACK frames sent out */
	uint32	txdnlfrm;	/**< Not used */
	uint32	txbcnfrm;	/**< beacons transmitted */
	uint32	txfunfl[6];	/**< per-fifo tx underflows */
	uint32	rxtoolate;	/**< receive too late */
	uint32  txfbw;		/**< transmit at fallback bw (dynamic bw) */
	uint32	txtplunfl;	/**< Template underflows (mac was too slow to transmit ACK/CTS
				 * or BCN)
				 */
	uint32	txphyerror;	/**< Transmit phy error, type of error is reported in tx-status for
				 * driver enqueued frames
				 */
	uint32	rxfrmtoolong;	/**< Received frame longer than legal limit (2346 bytes) */
	uint32	rxfrmtooshrt; /**< Received frame did not contain enough bytes for its frame type */
	uint32	rxinvmachdr;	/**< Either the protocol version != 0 or frame type not
				 * data/control/management
				 */
	uint32	rxbadfcs;	/**< number of frames for which the CRC check failed in the MAC */
	uint32	rxbadplcp;	/**< parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/**< PHY was able to correlate the preamble but not the header */
	uint32	rxstrt;		/**< Number of received frames with a good PLCP
				 * (i.e. passing parity check)
				 */
	uint32	rxdfrmucastmbss; /* Number of received DATA frames with good FCS and matching RA */
	uint32	rxmfrmucastmbss; /* number of received mgmt frames with good FCS and matching RA */
	uint32	rxcfrmucast; /**< number of received CNTRL frames with good FCS and matching RA */
	uint32	rxrtsucast;	/**< number of unicast RTS addressed to the MAC (good FCS) */
	uint32	rxctsucast;	/**< number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxackucast;	/**< number of ucast ACKS received (good FCS) */
	uint32	rxdfrmocast; /**< number of received DATA frames (good FCS and not matching RA) */
	uint32	rxmfrmocast; /**< number of received MGMT frames (good FCS and not matching RA) */
	uint32	rxcfrmocast; /**< number of received CNTRL frame (good FCS and not matching RA) */
	uint32	rxrtsocast;	/**< number of received RTS not addressed to the MAC */
	uint32	rxctsocast;	/**< number of received CTS not addressed to the MAC */
	uint32	rxdfrmmcast;	/**< number of RX Data multicast frames received by the MAC */
	uint32	rxmfrmmcast;	/**< number of RX Management multicast frames received by the MAC */
	uint32	rxcfrmmcast;	/**< number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32	rxbeaconmbss;	/**< beacons received from member of BSS */
	uint32	rxdfrmucastobss; /**< number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32	rxbeaconobss;	/**< beacons received from other BSS */
	uint32	rxrsptmout;	/**< Number of response timeouts for transmitted frames
				 * expecting a response
				 */
	uint32	bcntxcancl;	/**< transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32	rxf0ovfl;	/**< Number of receive fifo 0 overflows */
	uint32	rxf1ovfl;	/**< Number of receive fifo 1 overflows (obsolete) */
	uint32	rxf2ovfl;	/**< Number of receive fifo 2 overflows (obsolete) */
	uint32	txsfovfl;	/**< Number of transmit status fifo overflows (obsolete) */
	uint32	pmqovfl;	/**< Number of PMQ overflows */
	uint32	rxcgprqfrm;	/**< Number of received Probe requests that made it into
				 * the PRQ fifo
				 */
	uint32	rxcgprsqovfl;	/**< Rx Probe Request Que overflow in the AP */
	uint32	txcgprsfail;	/**< Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32	txcgprssuc;	/**< Tx Probe Response Success (ACK was received) */
	uint32	prs_timeout;	/**< Number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32	rxnack;		/**< obsolete */
	uint32	frmscons;	/**< obsolete */
	uint32  txnack;		/**< obsolete */
	uint32	rxback;		/**< blockack rxcnt */
	uint32	txback;		/**< blockack txcnt */

	/* 802.11 MIB counters, pp. 614 of 802.11 reaff doc. */
	uint32	txfrag;		/**< dot11TransmittedFragmentCount */
	uint32	txmulti;	/**< dot11MulticastTransmittedFrameCount */
	uint32	txfail;		/**< dot11FailedCount */
	uint32	txretry;	/**< dot11RetryCount */
	uint32	txretrie;	/**< dot11MultipleRetryCount */
	uint32	rxdup;		/**< dot11FrameduplicateCount */
	uint32	txrts;		/**< dot11RTSSuccessCount */
	uint32	txnocts;	/**< dot11RTSFailureCount */
	uint32	txnoack;	/**< dot11ACKFailureCount */
	uint32	rxfrag;		/**< dot11ReceivedFragmentCount */
	uint32	rxmulti;	/**< dot11MulticastReceivedFrameCount */
	uint32	rxcrc;		/**< dot11FCSErrorCount */
	uint32	txfrmsnt;	/**< dot11TransmittedFrameCount (bogus MIB?) */
	uint32	rxundec;	/**< dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32	tkipmicfaill;	/**< TKIPLocalMICFailures */
	uint32	tkipcntrmsr;	/**< TKIPCounterMeasuresInvoked */
	uint32	tkipreplay;	/**< TKIPReplays */
	uint32	ccmpfmterr;	/**< CCMPFormatErrors */
	uint32	ccmpreplay;	/**< CCMPReplays */
	uint32	ccmpundec;	/**< CCMPDecryptErrors */
	uint32	fourwayfail;	/**< FourWayHandshakeFailures */
	uint32	wepundec;	/**< dot11WEPUndecryptableCount */
	uint32	wepicverr;	/**< dot11WEPICVErrorCount */
	uint32	decsuccess;	/**< DecryptSuccessCount */
	uint32	tkipicverr;	/**< TKIPICVErrorCount */
	uint32	wepexcluded;	/**< dot11WEPExcludedCount */

	uint32	txchanrej;	/**< Tx frames suppressed due to channel rejection */
	uint32	psmwds;		/**< Count PSM watchdogs */
	uint32	phywatchdog;	/**< Count Phy watchdogs (triggered by ucode) */

	/* MBSS counters, AP only */
	uint32	prq_entries_handled;	/**< PRQ entries read in */
	uint32	prq_undirected_entries;	/**<    which were bcast bss & ssid */
	uint32	prq_bad_entries;	/**<    which could not be translated to info */
	uint32	atim_suppress_count;	/**< TX suppressions on ATIM fifo */
	uint32	bcn_template_not_ready;	/**< Template marked in use on send bcn ... */
	uint32	bcn_template_not_ready_done; /**< ...but "DMA done" interrupt rcvd */
	uint32	late_tbtt_dpc;	/**< TBTT DPC did not happen in time */

	/* per-rate receive stat counters */
	uint32  rx1mbps;	/**< packets rx at 1Mbps */
	uint32  rx2mbps;	/**< packets rx at 2Mbps */
	uint32  rx5mbps5;	/**< packets rx at 5.5Mbps */
	uint32  rx6mbps;	/**< packets rx at 6Mbps */
	uint32  rx9mbps;	/**< packets rx at 9Mbps */
	uint32  rx11mbps;	/**< packets rx at 11Mbps */
	uint32  rx12mbps;	/**< packets rx at 12Mbps */
	uint32  rx18mbps;	/**< packets rx at 18Mbps */
	uint32  rx24mbps;	/**< packets rx at 24Mbps */
	uint32  rx36mbps;	/**< packets rx at 36Mbps */
	uint32  rx48mbps;	/**< packets rx at 48Mbps */
	uint32  rx54mbps;	/**< packets rx at 54Mbps */
	uint32  rx108mbps;	/**< packets rx at 108mbps */
	uint32  rx162mbps;	/**< packets rx at 162mbps */
	uint32  rx216mbps;	/**< packets rx at 216 mbps */
	uint32  rx270mbps;	/**< packets rx at 270 mbps */
	uint32  rx324mbps;	/**< packets rx at 324 mbps */
	uint32  rx378mbps;	/**< packets rx at 378 mbps */
	uint32  rx432mbps;	/**< packets rx at 432 mbps */
	uint32  rx486mbps;	/**< packets rx at 486 mbps */
	uint32  rx540mbps;	/**< packets rx at 540 mbps */

	/* pkteng rx frame stats */
	uint32	pktengrxducast; /**< unicast frames rxed by the pkteng code */
	uint32	pktengrxdmcast; /**< multicast frames rxed by the pkteng code */

	uint32	rfdisable;	/**< count of radio disables */
	uint32	bphy_rxcrsglitch;	/**< PHY count of bphy glitches */
	uint32  bphy_badplcp;

	uint32	txexptime;	/**< Tx frames suppressed due to timer expiration */

	uint32	txmpdu_sgi;	/**< count for sgi transmit */
	uint32	rxmpdu_sgi;	/**< count for sgi received */
	uint32	txmpdu_stbc;	/**< count for stbc transmit */
	uint32	rxmpdu_stbc;	/**< count for stbc received */

	uint32	rxundec_mcst;	/**< dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32	tkipmicfaill_mcst;	/**< TKIPLocalMICFailures */
	uint32	tkipcntrmsr_mcst;	/**< TKIPCounterMeasuresInvoked */
	uint32	tkipreplay_mcst;	/**< TKIPReplays */
	uint32	ccmpfmterr_mcst;	/**< CCMPFormatErrors */
	uint32	ccmpreplay_mcst;	/**< CCMPReplays */
	uint32	ccmpundec_mcst;	/**< CCMPDecryptErrors */
	uint32	fourwayfail_mcst;	/**< FourWayHandshakeFailures */
	uint32	wepundec_mcst;	/**< dot11WEPUndecryptableCount */
	uint32	wepicverr_mcst;	/**< dot11WEPICVErrorCount */
	uint32	decsuccess_mcst;	/**< DecryptSuccessCount */
	uint32	tkipicverr_mcst;	/**< TKIPICVErrorCount */
	uint32	wepexcluded_mcst;	/**< dot11WEPExcludedCount */

	uint32	dma_hang;	/**< count for dma hang */
	uint32	reinit;		/**< count for reinit */

	uint32  pstatxucast;	/**< count of ucast frames xmitted on all psta assoc */
	uint32  pstatxnoassoc;	/**< count of txnoassoc frames xmitted on all psta assoc */
	uint32  pstarxucast;	/**< count of ucast frames received on all psta assoc */
	uint32  pstarxbcmc;	/**< count of bcmc frames received on all psta */
	uint32  pstatxbcmc;	/**< count of bcmc frames transmitted on all psta */

	uint32  cso_passthrough; /**< hw cso required but passthrough */
	uint32	cso_normal;	/**< hw cso hdr for normal process */
	uint32	chained;	/**< number of frames chained */
	uint32	chainedsz1;	/**< number of chain size 1 frames */
	uint32	unchained;	/**< number of frames not chained */
	uint32	maxchainsz;	/**< max chain size so far */
	uint32	currchainsz;	/**< current chain size */
	uint32	rxdrop20s;	/**< drop secondary cnt */
	uint32	pciereset;	/**< Secondary Bus Reset issued by driver */
	uint32	cfgrestore;	/**< configspace restore by driver */
	uint32	reinitreason[NREINITREASONCOUNT]; /**< reinitreason counters; 0: Unknown reason */
	uint32  rxrtry;		/**< num of received packets with retry bit on */
	uint32	txmpdu;		/**< macstat cnt only valid in ver 11. number of MPDUs txed.  */
	uint32	rxnodelim;	/**< macstat cnt only valid in ver 11.
				 * number of occasions that no valid delimiter is detected
				 * by ampdu parser.
				 */
	uint32  rxmpdu_mu;      /**< Number of MU MPDUs received */

	/* detailed control/management frames */
	uint32	txbar;		/**< Number of TX BAR */
	uint32	rxbar;		/**< Number of RX BAR */
	uint32	txpspoll;	/**< Number of TX PS-poll */
	uint32	rxpspoll;	/**< Number of RX PS-poll */
	uint32	txnull;		/**< Number of TX NULL_DATA */
	uint32	rxnull;		/**< Number of RX NULL_DATA */
	uint32	txqosnull;	/**< Number of TX NULL_QoSDATA */
	uint32	rxqosnull;	/**< Number of RX NULL_QoSDATA */
	uint32	txassocreq;	/**< Number of TX ASSOC request */
	uint32	rxassocreq;	/**< Number of RX ASSOC request */
	uint32	txreassocreq;	/**< Number of TX REASSOC request */
	uint32	rxreassocreq;	/**< Number of RX REASSOC request */
	uint32	txdisassoc;	/**< Number of TX DISASSOC */
	uint32	rxdisassoc;	/**< Number of RX DISASSOC */
	uint32	txassocrsp;	/**< Number of TX ASSOC response */
	uint32	rxassocrsp;	/**< Number of RX ASSOC response */
	uint32	txreassocrsp;	/**< Number of TX REASSOC response */
	uint32	rxreassocrsp;	/**< Number of RX REASSOC response */
	uint32	txauth;		/**< Number of TX AUTH */
	uint32	rxauth;		/**< Number of RX AUTH */
	uint32	txdeauth;	/**< Number of TX DEAUTH */
	uint32	rxdeauth;	/**< Number of RX DEAUTH */
	uint32	txprobereq;	/**< Number of TX probe request */
	uint32	rxprobereq;	/**< Number of RX probe request */
	uint32	txprobersp;	/**< Number of TX probe response */
	uint32	rxprobersp;	/**< Number of RX probe response */
	uint32	txaction;	/**< Number of TX action frame */
	uint32	rxaction;	/**< Number of RX action frame */
	uint32  ampdu_wds;      /**< Number of AMPDU watchdogs */
	uint32  txlost;         /**< Number of lost packets reported in txs */
	uint32  txdatamcast;	/**< Number of TX multicast data packets */
	uint32  txdatabcast;	/**< Number of TX broadcast data packets */
	uint32  txbcast;        /* Broadcast TransmittedFrameCount */
	uint32  txdropped;      /* tx dropped pkts */
	uint32  rxbcast;        /* BroadcastReceivedFrameCount */
	uint32  rxdropped;      /* rx dropped pkts (derived: sum of others) */

	/* This structure is deprecated and used only for ver <= 11.
	 * All counter variables have to be of uint32.
	 * Please refer to the following twiki before editing.
	 * http://hwnbu-twiki.sj.broadcom.com/bin/view/
	 * Mwgroup/WlCounters#wlc_layer_counters_non_xTLV
	 */
} wl_cnt_ver_11_t;

typedef struct {
	uint16	version;	/* see definition of WL_CNT_T_VERSION */
	uint16	length;		/* length of entire structure */

	/* transmit stat counters */
	uint32	txframe;	/* tx data frames */
	uint32	txbyte;		/* tx data bytes */
	uint32	txretrans;	/* tx mac retransmits */
	uint32	txerror;	/* tx data errors (derived: sum of others) */
	uint32	txctl;		/* tx management frames */
	uint32	txprshort;	/* tx short preamble frames */
	uint32	txserr;		/* tx status errors */
	uint32	txnobuf;	/* tx out of buffers errors */
	uint32	txnoassoc;	/* tx discard because we're not associated */
	uint32	txrunt;		/* tx runt frames */
	uint32	txchit;		/* tx header cache hit (fastpath) */
	uint32	txcmiss;	/* tx header cache miss (slowpath) */

	/* transmit chip error counters */
	uint32	txuflo;		/* tx fifo underflows */
	uint32	txphyerr;	/* tx phy errors (indicated in tx status) */
	uint32	txphycrs;

	/* receive stat counters */
	uint32	rxframe;	/* rx data frames */
	uint32	rxbyte;		/* rx data bytes */
	uint32	rxerror;	/* rx data errors (derived: sum of others) */
	uint32	rxctl;		/* rx management frames */
	uint32	rxnobuf;	/* rx out of buffers errors */
	uint32	rxnondata;	/* rx non data frames in the data channel errors */
	uint32	rxbadds;	/* rx bad DS errors */
	uint32	rxbadcm;	/* rx bad control or management frames */
	uint32	rxfragerr;	/* rx fragmentation errors */
	uint32	rxrunt;		/* rx runt frames */
	uint32	rxgiant;	/* rx giant frames */
	uint32	rxnoscb;	/* rx no scb error */
	uint32	rxbadproto;	/* rx invalid frames */
	uint32	rxbadsrcmac;	/* rx frames with Invalid Src Mac */
	uint32	rxbadda;	/* rx frames tossed for invalid da */
	uint32	rxfilter;	/* rx frames filtered out */

	/* receive chip error counters */
	uint32	rxoflo;		/* rx fifo overflow errors */
	uint32	rxuflo[NFIFO];	/* rx dma descriptor underflow errors */

	uint32	d11cnt_txrts_off;	/* d11cnt txrts value when reset d11cnt */
	uint32	d11cnt_rxcrc_off;	/* d11cnt rxcrc value when reset d11cnt */
	uint32	d11cnt_txnocts_off;	/* d11cnt txnocts value when reset d11cnt */

	/* misc counters */
	uint32	dmade;		/* tx/rx dma descriptor errors */
	uint32	dmada;		/* tx/rx dma data errors */
	uint32	dmape;		/* tx/rx dma descriptor protocol errors */
	uint32	reset;		/* reset count */
	uint32	tbtt;		/* cnts the TBTT int's */
	uint32	txdmawar;
	uint32	pkt_callback_reg_fail;	/* callbacks register failure */

	/* MAC counters: 32-bit version of d11.h's macstat_t */
	uint32	txallfrm;	/* total number of frames sent, incl. Data, ACK, RTS, CTS,
				 * Control Management (includes retransmissions)
				 */
	uint32	txrtsfrm;	/* number of RTS sent out by the MAC */
	uint32	txctsfrm;	/* number of CTS sent out by the MAC */
	uint32	txackfrm;	/* number of ACK frames sent out */
	uint32	txdnlfrm;	/* Not used */
	uint32	txbcnfrm;	/* beacons transmitted */
	uint32	txfunfl[8];	/* per-fifo tx underflows */
	uint32	txtplunfl;	/* Template underflows (mac was too slow to transmit ACK/CTS
				 * or BCN)
				 */
	uint32	txphyerror;	/* Transmit phy error, type of error is reported in tx-status for
				 * driver enqueued frames
				 */
	uint32	rxfrmtoolong;	/* Received frame longer than legal limit (2346 bytes) */
	uint32	rxfrmtooshrt;	/* Received frame did not contain enough bytes for its frame type */
	uint32	rxinvmachdr;	/* Either the protocol version != 0 or frame type not
				 * data/control/management
				 */
	uint32	rxbadfcs;	/* number of frames for which the CRC check failed in the MAC */
	uint32	rxbadplcp;	/* parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/* PHY was able to correlate the preamble but not the header */
	uint32	rxstrt;		/* Number of received frames with a good PLCP
				 * (i.e. passing parity check)
				 */
	uint32	rxdfrmucastmbss; /* Number of received DATA frames with good FCS and matching RA */
	uint32	rxmfrmucastmbss; /* number of received mgmt frames with good FCS and matching RA */
	uint32	rxcfrmucast;	/* number of received CNTRL frames with good FCS and matching RA */
	uint32	rxrtsucast;	/* number of unicast RTS addressed to the MAC (good FCS) */
	uint32	rxctsucast;	/* number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxackucast;	/* number of ucast ACKS received (good FCS) */
	uint32	rxdfrmocast;	/* number of received DATA frames (good FCS and not matching RA) */
	uint32	rxmfrmocast;	/* number of received MGMT frames (good FCS and not matching RA) */
	uint32	rxcfrmocast;	/* number of received CNTRL frame (good FCS and not matching RA) */
	uint32	rxrtsocast;	/* number of received RTS not addressed to the MAC */
	uint32	rxctsocast;	/* number of received CTS not addressed to the MAC */
	uint32	rxdfrmmcast;	/* number of RX Data multicast frames received by the MAC */
	uint32	rxmfrmmcast;	/* number of RX Management multicast frames received by the MAC */
	uint32	rxcfrmmcast;	/* number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32	rxbeaconmbss;	/* beacons received from member of BSS */
	uint32	rxdfrmucastobss; /* number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32	rxbeaconobss;	/* beacons received from other BSS */
	uint32	rxrsptmout;	/* Number of response timeouts for transmitted frames
				 * expecting a response
				 */
	uint32	bcntxcancl;	/* transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32	rxf0ovfl;	/* Number of receive fifo 0 overflows */
	uint32	rxf1ovfl;	/* Number of receive fifo 1 overflows (obsolete) */
	uint32	rxf2ovfl;	/* Number of receive fifo 2 overflows (obsolete) */
	uint32	txsfovfl;	/* Number of transmit status fifo overflows (obsolete) */
	uint32	pmqovfl;	/* Number of PMQ overflows */
	uint32	rxcgprqfrm;	/* Number of received Probe requests that made it into
				 * the PRQ fifo
				 */
	uint32	rxcgprsqovfl;	/* Rx Probe Request Que overflow in the AP */
	uint32	txcgprsfail;	/* Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32	txcgprssuc;	/* Tx Probe Response Success (ACK was received) */
	uint32	prs_timeout;	/* Number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32	rxnack;		/* obsolete */
	uint32	frmscons;	/* obsolete */
	uint32	txnack;		/* obsolete */
	uint32	txglitch_nack;	/* obsolete */
	uint32	txburst;	/* obsolete */

	/* 802.11 MIB counters, pp. 614 of 802.11 reaff doc. */
	uint32	txfrag;		/* dot11TransmittedFragmentCount */
	uint32	txmulti;	/* dot11MulticastTransmittedFrameCount */
	uint32	txfail;		/* dot11FailedCount */
	uint32	txretry;	/* dot11RetryCount */
	uint32	txretrie;	/* dot11MultipleRetryCount */
	uint32	rxdup;		/* dot11FrameduplicateCount */
	uint32	txrts;		/* dot11RTSSuccessCount */
	uint32	txnocts;	/* dot11RTSFailureCount */
	uint32	txnoack;	/* dot11ACKFailureCount */
	uint32	rxfrag;		/* dot11ReceivedFragmentCount */
	uint32	rxmulti;	/* dot11MulticastReceivedFrameCount */
	uint32	rxcrc;		/* dot11FCSErrorCount */
	uint32	txfrmsnt;	/* dot11TransmittedFrameCount (bogus MIB?) */
	uint32	rxundec;	/* dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32	tkipmicfaill;	/* TKIPLocalMICFailures */
	uint32	tkipcntrmsr;	/* TKIPCounterMeasuresInvoked */
	uint32	tkipreplay;	/* TKIPReplays */
	uint32	ccmpfmterr;	/* CCMPFormatErrors */
	uint32	ccmpreplay;	/* CCMPReplays */
	uint32	ccmpundec;	/* CCMPDecryptErrors */
	uint32	fourwayfail;	/* FourWayHandshakeFailures */
	uint32	wepundec;	/* dot11WEPUndecryptableCount */
	uint32	wepicverr;	/* dot11WEPICVErrorCount */
	uint32	decsuccess;	/* DecryptSuccessCount */
	uint32	tkipicverr;	/* TKIPICVErrorCount */
	uint32	wepexcluded;	/* dot11WEPExcludedCount */

	uint32	txchanrej;	/* Tx frames suppressed due to channel rejection */
	uint32	psmwds;		/* Count PSM watchdogs */
	uint32	phywatchdog;	/* Count Phy watchdogs (triggered by ucode) */

	/* MBSS counters, AP only */
	uint32	prq_entries_handled;	/* PRQ entries read in */
	uint32	prq_undirected_entries;	/*    which were bcast bss & ssid */
	uint32	prq_bad_entries;	/*    which could not be translated to info */
	uint32	atim_suppress_count;	/* TX suppressions on ATIM fifo */
	uint32	bcn_template_not_ready;	/* Template marked in use on send bcn ... */
	uint32	bcn_template_not_ready_done; /* ...but "DMA done" interrupt rcvd */
	uint32	late_tbtt_dpc;	/* TBTT DPC did not happen in time */

	/* per-rate receive stat counters */
	uint32  rx1mbps;	/* packets rx at 1Mbps */
	uint32  rx2mbps;	/* packets rx at 2Mbps */
	uint32  rx5mbps5;	/* packets rx at 5.5Mbps */
	uint32  rx6mbps;	/* packets rx at 6Mbps */
	uint32  rx9mbps;	/* packets rx at 9Mbps */
	uint32  rx11mbps;	/* packets rx at 11Mbps */
	uint32  rx12mbps;	/* packets rx at 12Mbps */
	uint32  rx18mbps;	/* packets rx at 18Mbps */
	uint32  rx24mbps;	/* packets rx at 24Mbps */
	uint32  rx36mbps;	/* packets rx at 36Mbps */
	uint32  rx48mbps;	/* packets rx at 48Mbps */
	uint32  rx54mbps;	/* packets rx at 54Mbps */
	uint32  rx108mbps;	/* packets rx at 108mbps */
	uint32  rx162mbps;	/* packets rx at 162mbps */
	uint32  rx216mbps;	/* packets rx at 216 mbps */
	uint32  rx270mbps;	/* packets rx at 270 mbps */
	uint32  rx324mbps;	/* packets rx at 324 mbps */
	uint32  rx378mbps;	/* packets rx at 378 mbps */
	uint32  rx432mbps;	/* packets rx at 432 mbps */
	uint32  rx486mbps;	/* packets rx at 486 mbps */
	uint32  rx540mbps;	/* packets rx at 540 mbps */

	/* pkteng rx frame stats */
	uint32	pktengrxducast; /* unicast frames rxed by the pkteng code */
	uint32	pktengrxdmcast; /* multicast frames rxed by the pkteng code */

	uint32	rfdisable;	/* count of radio disables */
	uint32	bphy_rxcrsglitch;	/* PHY count of bphy glitches */

	uint32	txexptime;	/* Tx frames suppressed due to timer expiration */

	uint32	txmpdu_sgi;	/* count for sgi transmit */
	uint32	rxmpdu_sgi;	/* count for sgi received */
	uint32	txmpdu_stbc;	/* count for stbc transmit */
	uint32	rxmpdu_stbc;	/* count for stbc received */

	uint32	rxundec_mcst;	/* dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32	tkipmicfaill_mcst;	/* TKIPLocalMICFailures */
	uint32	tkipcntrmsr_mcst;	/* TKIPCounterMeasuresInvoked */
	uint32	tkipreplay_mcst;	/* TKIPReplays */
	uint32	ccmpfmterr_mcst;	/* CCMPFormatErrors */
	uint32	ccmpreplay_mcst;	/* CCMPReplays */
	uint32	ccmpundec_mcst;	/* CCMPDecryptErrors */
	uint32	fourwayfail_mcst;	/* FourWayHandshakeFailures */
	uint32	wepundec_mcst;	/* dot11WEPUndecryptableCount */
	uint32	wepicverr_mcst;	/* dot11WEPICVErrorCount */
	uint32	decsuccess_mcst;	/* DecryptSuccessCount */
	uint32	tkipicverr_mcst;	/* TKIPICVErrorCount */
	uint32	wepexcluded_mcst;	/* dot11WEPExcludedCount */

	uint32	dma_hang;	/* count for stbc received */
	uint32	rxrtry;		/* number of packets with retry bit set to 1 */
} wl_cnt_ver_7_t;

typedef struct {
	uint16  version;    /**< see definition of WL_CNT_T_VERSION */
	uint16  length;     /**< length of entire structure */

	/* transmit stat counters */
	uint32  txframe;    /**< tx data frames */
	uint32  txbyte;     /**< tx data bytes */
	uint32  txretrans;  /**< tx mac retransmits */
	uint32  txerror;    /**< tx data errors (derived: sum of others) */
	uint32  txctl;      /**< tx management frames */
	uint32  txprshort;  /**< tx short preamble frames */
	uint32  txserr;     /**< tx status errors */
	uint32  txnobuf;    /**< tx out of buffers errors */
	uint32  txnoassoc;  /**< tx discard because we're not associated */
	uint32  txrunt;     /**< tx runt frames */
	uint32  txchit;     /**< tx header cache hit (fastpath) */
	uint32  txcmiss;    /**< tx header cache miss (slowpath) */

	/* transmit chip error counters */
	uint32  txuflo;     /**< tx fifo underflows */
	uint32  txphyerr;   /**< tx phy errors (indicated in tx status) */
	uint32  txphycrs;

	/* receive stat counters */
	uint32  rxframe;    /**< rx data frames */
	uint32  rxbyte;     /**< rx data bytes */
	uint32  rxerror;    /**< rx data errors (derived: sum of others) */
	uint32  rxctl;      /**< rx management frames */
	uint32  rxnobuf;    /**< rx out of buffers errors */
	uint32  rxnondata;  /**< rx non data frames in the data channel errors */
	uint32  rxbadds;    /**< rx bad DS errors */
	uint32  rxbadcm;    /**< rx bad control or management frames */
	uint32  rxfragerr;  /**< rx fragmentation errors */
	uint32  rxrunt;     /**< rx runt frames */
	uint32  rxgiant;    /**< rx giant frames */
	uint32  rxnoscb;    /**< rx no scb error */
	uint32  rxbadproto; /**< rx invalid frames */
	uint32  rxbadsrcmac;    /**< rx frames with Invalid Src Mac */
	uint32  rxbadda;    /**< rx frames tossed for invalid da */
	uint32  rxfilter;   /**< rx frames filtered out */

	/* receive chip error counters */
	uint32  rxoflo;     /**< rx fifo overflow errors */
	uint32  rxuflo[NFIFO];  /**< rx dma descriptor underflow errors */

	uint32  d11cnt_txrts_off;   /**< d11cnt txrts value when reset d11cnt */
	uint32  d11cnt_rxcrc_off;   /**< d11cnt rxcrc value when reset d11cnt */
	uint32  d11cnt_txnocts_off; /**< d11cnt txnocts value when reset d11cnt */

	/* misc counters */
	uint32  dmade;      /**< tx/rx dma descriptor errors */
	uint32  dmada;      /**< tx/rx dma data errors */
	uint32  dmape;      /**< tx/rx dma descriptor protocol errors */
	uint32  reset;      /**< reset count */
	uint32  tbtt;       /**< cnts the TBTT int's */
	uint32  txdmawar;
	uint32  pkt_callback_reg_fail;  /**< callbacks register failure */

	/* MAC counters: 32-bit version of d11.h's macstat_t */
	uint32  txallfrm;   /**< total number of frames sent, incl. Data, ACK, RTS, CTS,
			     * Control Management (includes retransmissions)
			     */
	uint32  txrtsfrm;   /**< number of RTS sent out by the MAC */
	uint32  txctsfrm;   /**< number of CTS sent out by the MAC */
	uint32  txackfrm;   /**< number of ACK frames sent out */
	uint32  txdnlfrm;   /**< Not used */
	uint32  txbcnfrm;   /**< beacons transmitted */
	uint32  txfunfl[6]; /**< per-fifo tx underflows */
	uint32	rxtoolate;	/**< receive too late */
	uint32  txfbw;	    /**< transmit at fallback bw (dynamic bw) */
	uint32  txtplunfl;  /**< Template underflows (mac was too slow to transmit ACK/CTS
			     * or BCN)
			     */
	uint32  txphyerror; /**< Transmit phy error, type of error is reported in tx-status for
			     * driver enqueued frames
			     */
	uint32  rxfrmtoolong;   /**< Received frame longer than legal limit (2346 bytes) */
	uint32  rxfrmtooshrt; /**< Received frame did not contain enough bytes for its frame type */
	uint32  rxinvmachdr;    /**< Either the protocol version != 0 or frame type not
				 * data/control/management
			   */
	uint32  rxbadfcs;   /**< number of frames for which the CRC check failed in the MAC */
	uint32  rxbadplcp;  /**< parity check of the PLCP header failed */
	uint32  rxcrsglitch;    /**< PHY was able to correlate the preamble but not the header */
	uint32  rxstrt;     /**< Number of received frames with a good PLCP
			     * (i.e. passing parity check)
			     */
	uint32  rxdfrmucastmbss; /**< # of received DATA frames with good FCS and matching RA */
	uint32  rxmfrmucastmbss; /**< # of received mgmt frames with good FCS and matching RA */
	uint32  rxcfrmucast;     /**< # of received CNTRL frames with good FCS and matching RA */
	uint32  rxrtsucast; /**< number of unicast RTS addressed to the MAC (good FCS) */
	uint32  rxctsucast; /**< number of unicast CTS addressed to the MAC (good FCS) */
	uint32  rxackucast; /**< number of ucast ACKS received (good FCS) */
	uint32  rxdfrmocast;    /**< # of received DATA frames (good FCS and not matching RA) */
	uint32  rxmfrmocast;    /**< # of received MGMT frames (good FCS and not matching RA) */
	uint32  rxcfrmocast;    /**< # of received CNTRL frame (good FCS and not matching RA) */
	uint32  rxrtsocast; /**< number of received RTS not addressed to the MAC */
	uint32  rxctsocast; /**< number of received CTS not addressed to the MAC */
	uint32  rxdfrmmcast;    /**< number of RX Data multicast frames received by the MAC */
	uint32  rxmfrmmcast;    /**< number of RX Management multicast frames received by the MAC */
	uint32  rxcfrmmcast;    /**< number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32  rxbeaconmbss;   /**< beacons received from member of BSS */
	uint32  rxdfrmucastobss; /**< number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32  rxbeaconobss;   /**< beacons received from other BSS */
	uint32  rxrsptmout; /**< Number of response timeouts for transmitted frames
			     * expecting a response
			     */
	uint32  bcntxcancl; /**< transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32  rxf0ovfl;   /**< Number of receive fifo 0 overflows */
	uint32  rxf1ovfl;   /**< Number of receive fifo 1 overflows (obsolete) */
	uint32  rxf2ovfl;   /**< Number of receive fifo 2 overflows (obsolete) */
	uint32  txsfovfl;   /**< Number of transmit status fifo overflows (obsolete) */
	uint32  pmqovfl;    /**< Number of PMQ overflows */
	uint32  rxcgprqfrm; /**< Number of received Probe requests that made it into
			     * the PRQ fifo
			     */
	uint32  rxcgprsqovfl;   /**< Rx Probe Request Que overflow in the AP */
	uint32  txcgprsfail;    /**< Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32  txcgprssuc; /**< Tx Probe Response Success (ACK was received) */
	uint32  prs_timeout;    /**< Number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32  rxnack;     /**< Number of NACKS received (Afterburner) */
	uint32  frmscons;   /**< Number of frames completed without transmission because of an
			     * Afterburner re-queue
			     */
	uint32  txnack;		/**< obsolete */
	uint32	rxback;		/**< blockack rxcnt */
	uint32	txback;		/**< blockack txcnt */

	/* 802.11 MIB counters, pp. 614 of 802.11 reaff doc. */
	uint32  txfrag;     /**< dot11TransmittedFragmentCount */
	uint32  txmulti;    /**< dot11MulticastTransmittedFrameCount */
	uint32  txfail;     /**< dot11FailedCount */
	uint32  txretry;    /**< dot11RetryCount */
	uint32  txretrie;   /**< dot11MultipleRetryCount */
	uint32  rxdup;      /**< dot11FrameduplicateCount */
	uint32  txrts;      /**< dot11RTSSuccessCount */
	uint32  txnocts;    /**< dot11RTSFailureCount */
	uint32  txnoack;    /**< dot11ACKFailureCount */
	uint32  rxfrag;     /**< dot11ReceivedFragmentCount */
	uint32  rxmulti;    /**< dot11MulticastReceivedFrameCount */
	uint32  rxcrc;      /**< dot11FCSErrorCount */
	uint32  txfrmsnt;   /**< dot11TransmittedFrameCount (bogus MIB?) */
	uint32  rxundec;    /**< dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32  tkipmicfaill;   /**< TKIPLocalMICFailures */
	uint32  tkipcntrmsr;    /**< TKIPCounterMeasuresInvoked */
	uint32  tkipreplay; /**< TKIPReplays */
	uint32  ccmpfmterr; /**< CCMPFormatErrors */
	uint32  ccmpreplay; /**< CCMPReplays */
	uint32  ccmpundec;  /**< CCMPDecryptErrors */
	uint32  fourwayfail;    /**< FourWayHandshakeFailures */
	uint32  wepundec;   /**< dot11WEPUndecryptableCount */
	uint32  wepicverr;  /**< dot11WEPICVErrorCount */
	uint32  decsuccess; /**< DecryptSuccessCount */
	uint32  tkipicverr; /**< TKIPICVErrorCount */
	uint32  wepexcluded;    /**< dot11WEPExcludedCount */

	uint32  rxundec_mcst;   /**< dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32  tkipmicfaill_mcst;  /**< TKIPLocalMICFailures */
	uint32  tkipcntrmsr_mcst;   /**< TKIPCounterMeasuresInvoked */
	uint32  tkipreplay_mcst;    /**< TKIPReplays */
	uint32  ccmpfmterr_mcst;    /**< CCMPFormatErrors */
	uint32  ccmpreplay_mcst;    /**< CCMPReplays */
	uint32  ccmpundec_mcst; /**< CCMPDecryptErrors */
	uint32  fourwayfail_mcst;   /**< FourWayHandshakeFailures */
	uint32  wepundec_mcst;  /**< dot11WEPUndecryptableCount */
	uint32  wepicverr_mcst; /**< dot11WEPICVErrorCount */
	uint32  decsuccess_mcst;    /**< DecryptSuccessCount */
	uint32  tkipicverr_mcst;    /**< TKIPICVErrorCount */
	uint32  wepexcluded_mcst;   /**< dot11WEPExcludedCount */

	uint32  txchanrej;  /**< Tx frames suppressed due to channel rejection */
	uint32  txexptime;  /**< Tx frames suppressed due to timer expiration */
	uint32  psmwds;     /**< Count PSM watchdogs */
	uint32  phywatchdog;    /**< Count Phy watchdogs (triggered by ucode) */

	/* MBSS counters, AP only */
	uint32  prq_entries_handled;    /**< PRQ entries read in */
	uint32  prq_undirected_entries; /**<    which were bcast bss & ssid */
	uint32  prq_bad_entries;    /**<    which could not be translated to info */
	uint32  atim_suppress_count;    /**< TX suppressions on ATIM fifo */
	uint32  bcn_template_not_ready; /**< Template marked in use on send bcn ... */
	uint32  bcn_template_not_ready_done; /**< ...but "DMA done" interrupt rcvd */
	uint32  late_tbtt_dpc;  /**< TBTT DPC did not happen in time */

	/* per-rate receive stat counters */
	uint32  rx1mbps;    /**< packets rx at 1Mbps */
	uint32  rx2mbps;    /**< packets rx at 2Mbps */
	uint32  rx5mbps5;   /**< packets rx at 5.5Mbps */
	uint32  rx6mbps;    /**< packets rx at 6Mbps */
	uint32  rx9mbps;    /**< packets rx at 9Mbps */
	uint32  rx11mbps;   /**< packets rx at 11Mbps */
	uint32  rx12mbps;   /**< packets rx at 12Mbps */
	uint32  rx18mbps;   /**< packets rx at 18Mbps */
	uint32  rx24mbps;   /**< packets rx at 24Mbps */
	uint32  rx36mbps;   /**< packets rx at 36Mbps */
	uint32  rx48mbps;   /**< packets rx at 48Mbps */
	uint32  rx54mbps;   /**< packets rx at 54Mbps */
	uint32  rx108mbps;  /**< packets rx at 108mbps */
	uint32  rx162mbps;  /**< packets rx at 162mbps */
	uint32  rx216mbps;  /**< packets rx at 216 mbps */
	uint32  rx270mbps;  /**< packets rx at 270 mbps */
	uint32  rx324mbps;  /**< packets rx at 324 mbps */
	uint32  rx378mbps;  /**< packets rx at 378 mbps */
	uint32  rx432mbps;  /**< packets rx at 432 mbps */
	uint32  rx486mbps;  /**< packets rx at 486 mbps */
	uint32  rx540mbps;  /**< packets rx at 540 mbps */

	/* pkteng rx frame stats */
	uint32  pktengrxducast; /**< unicast frames rxed by the pkteng code */
	uint32  pktengrxdmcast; /**< multicast frames rxed by the pkteng code */

	uint32  rfdisable;  /**< count of radio disables */
	uint32  bphy_rxcrsglitch;   /**< PHY count of bphy glitches */
	uint32  bphy_badplcp;

	uint32  txmpdu_sgi; /**< count for sgi transmit */
	uint32  rxmpdu_sgi; /**< count for sgi received */
	uint32  txmpdu_stbc;    /**< count for stbc transmit */
	uint32  rxmpdu_stbc;    /**< count for stbc received */

	uint32	rxdrop20s;	/**< drop secondary cnt */
	/* All counter variables have to be of uint32. */
} wl_cnt_ver_6_t;

#define	WL_DELTA_STATS_T_VERSION	2	/**< current version of wl_delta_stats_t struct */

typedef struct {
	uint16 version;     /**< see definition of WL_DELTA_STATS_T_VERSION */
	uint16 length;      /**< length of entire structure */

	/* transmit stat counters */
	uint32 txframe;     /**< tx data frames */
	uint32 txbyte;      /**< tx data bytes */
	uint32 txretrans;   /**< tx mac retransmits */
	uint32 txfail;      /**< tx failures */

	/* receive stat counters */
	uint32 rxframe;     /**< rx data frames */
	uint32 rxbyte;      /**< rx data bytes */

	/* per-rate receive stat counters */
	uint32  rx1mbps;	/**< packets rx at 1Mbps */
	uint32  rx2mbps;	/**< packets rx at 2Mbps */
	uint32  rx5mbps5;	/**< packets rx at 5.5Mbps */
	uint32  rx6mbps;	/**< packets rx at 6Mbps */
	uint32  rx9mbps;	/**< packets rx at 9Mbps */
	uint32  rx11mbps;	/**< packets rx at 11Mbps */
	uint32  rx12mbps;	/**< packets rx at 12Mbps */
	uint32  rx18mbps;	/**< packets rx at 18Mbps */
	uint32  rx24mbps;	/**< packets rx at 24Mbps */
	uint32  rx36mbps;	/**< packets rx at 36Mbps */
	uint32  rx48mbps;	/**< packets rx at 48Mbps */
	uint32  rx54mbps;	/**< packets rx at 54Mbps */
	uint32  rx108mbps;	/**< packets rx at 108mbps */
	uint32  rx162mbps;	/**< packets rx at 162mbps */
	uint32  rx216mbps;	/**< packets rx at 216 mbps */
	uint32  rx270mbps;	/**< packets rx at 270 mbps */
	uint32  rx324mbps;	/**< packets rx at 324 mbps */
	uint32  rx378mbps;	/**< packets rx at 378 mbps */
	uint32  rx432mbps;	/**< packets rx at 432 mbps */
	uint32  rx486mbps;	/**< packets rx at 486 mbps */
	uint32  rx540mbps;	/**< packets rx at 540 mbps */

	/* phy stats */
	uint32 rxbadplcp;
	uint32 rxcrsglitch;
	uint32 bphy_rxcrsglitch;
	uint32 bphy_badplcp;

	uint32 slice_index; /**< Slice for which stats are reported */

} wl_delta_stats_t;

/* Partial statistics counter report */
#define WL_CNT_CTL_MGT_FRAMES	0

typedef struct {
	uint16	type;
	uint16	len;

	/* detailed control/management frames */
	uint32	txnull;
	uint32	rxnull;
	uint32	txqosnull;
	uint32	rxqosnull;
	uint32	txassocreq;
	uint32	rxassocreq;
	uint32	txreassocreq;
	uint32	rxreassocreq;
	uint32	txdisassoc;
	uint32	rxdisassoc;
	uint32	txassocrsp;
	uint32	rxassocrsp;
	uint32	txreassocrsp;
	uint32	rxreassocrsp;
	uint32	txauth;
	uint32	rxauth;
	uint32	txdeauth;
	uint32	rxdeauth;
	uint32	txprobereq;
	uint32	rxprobereq;
	uint32	txprobersp;
	uint32	rxprobersp;
	uint32	txaction;
	uint32	rxaction;
	uint32	txrts;
	uint32	rxrts;
	uint32	txcts;
	uint32	rxcts;
	uint32	txack;
	uint32	rxack;
	uint32	txbar;
	uint32	rxbar;
	uint32	txback;
	uint32	rxback;
	uint32	txpspoll;
	uint32	rxpspoll;
} wl_ctl_mgt_cnt_t;

typedef struct {
	uint32 packets;
	uint32 bytes;
} wl_traffic_stats_t;

typedef struct {
	uint16	version;	/**< see definition of WL_WME_CNT_VERSION */
	uint16	length;		/**< length of entire structure */

	wl_traffic_stats_t tx[AC_COUNT];	/**< Packets transmitted */
	wl_traffic_stats_t tx_failed[AC_COUNT];	/**< Packets dropped or failed to transmit */
	wl_traffic_stats_t rx[AC_COUNT];	/**< Packets received */
	wl_traffic_stats_t rx_failed[AC_COUNT];	/**< Packets failed to receive */

	wl_traffic_stats_t forward[AC_COUNT];	/**< Packets forwarded by AP */

	wl_traffic_stats_t tx_expired[AC_COUNT]; /**< packets dropped due to lifetime expiry */

} wl_wme_cnt_t;

struct wl_msglevel2 {
	uint32 low;
	uint32 high;
};

/* A versioned structure for setting and retrieving debug message levels. */
#define WL_MSGLEVEL_STRUCT_VERSION_1	1

typedef struct wl_msglevel_v1 {
	uint16 version;
	uint16 length;
	uint32 msglevel1;
	uint32 msglevel2;
	uint32 msglevel3;
	/* add another uint32 when full */
} wl_msglevel_v1_t;

#define WL_ICMP_IPV6_CFG_VERSION         1
#define WL_ICMP_IPV6_CLEAR_ALL           (1 << 0)

typedef struct wl_icmp_ipv6_cfg {
	uint16 version;
	uint16 length;
	uint16 fixed_length;
	uint16 flags;
	uint32 num_ipv6;
	/* num_ipv6 to follow */
	struct ipv6_addr host_ipv6[];
} wl_icmp_ipv6_cfg_t;

#define WL_ICMP_CFG_IPV6_FIXED_LEN      OFFSETOF(wl_icmp_ipv6_cfg_t, host_ipv6)
#define WL_ICMP_CFG_IPV6_LEN(count)    (WL_ICMP_CFG_IPV6_FIXED_LEN + \
					((count) * sizeof(struct ipv6_addr)))

typedef struct wl_mkeep_alive_pkt {
	uint16	version; /* Version for mkeep_alive */
	uint16	length; /* length of fixed parameters in the structure */
	uint32	period_msec; /* high bit on means immediate send */
	uint16	len_bytes;
	uint8	keep_alive_id; /* 0 - 3 for N = 4 */
	uint8	data[1];
} wl_mkeep_alive_pkt_t;

#define WL_MKEEP_ALIVE_VERSION		1
#define WL_MKEEP_ALIVE_FIXED_LEN	OFFSETOF(wl_mkeep_alive_pkt_t, data)
/* 1/2 second precision since idle time is a seconds counter anyway */
#define WL_MKEEP_ALIVE_PRECISION	500
#define WL_MKEEP_ALIVE_PERIOD_MASK  0x7FFFFFFF
#define WL_MKEEP_ALIVE_IMMEDIATE    0x80000000

/** TCP Keep-Alive conn struct */
typedef struct wl_mtcpkeep_alive_conn_pkt {
	struct ether_addr saddr;		/**< src mac address */
	struct ether_addr daddr;		/**< dst mac address */
	struct ipv4_addr sipaddr;		/**< source IP addr */
	struct ipv4_addr dipaddr;		/**< dest IP addr */
	uint16 sport;				/**< src port */
	uint16 dport;				/**< dest port */
	uint32 seq;				/**< seq number */
	uint32 ack;				/**< ACK number */
	uint16 tcpwin;				/**< TCP window */
	uint16 PAD;
} wl_mtcpkeep_alive_conn_pkt_t;

/** TCP Keep-Alive interval struct */
typedef struct wl_mtcpkeep_alive_timers_pkt {
	uint16 interval;		/**< interval timer */
	uint16 retry_interval;		/**< retry_interval timer */
	uint16 retry_count;		/**< retry_count */
} wl_mtcpkeep_alive_timers_pkt_t;

typedef struct wake_info {
	uint32 wake_reason;
	uint32 wake_info_len;		/**< size of packet */
	uint8  packet[];
} wake_info_t;

typedef struct wake_pkt {
	uint32 wake_pkt_len;		/**< size of packet */
	uint8  packet[];
} wake_pkt_t;

#define WL_MTCPKEEP_ALIVE_VERSION		1

/* #ifdef WLBA */

#define WLC_BA_CNT_VERSION  1   /**< current version of wlc_ba_cnt_t */

/** block ack related stats */
typedef struct wlc_ba_cnt {
	uint16  version;    /**< WLC_BA_CNT_VERSION */
	uint16  length;     /**< length of entire structure */

	/* transmit stat counters */
	uint32 txpdu;       /**< pdus sent */
	uint32 txsdu;       /**< sdus sent */
	uint32 txfc;        /**< tx side flow controlled packets */
	uint32 txfci;       /**< tx side flow control initiated */
	uint32 txretrans;   /**< retransmitted pdus */
	uint32 txbatimer;   /**< ba resend due to timer */
	uint32 txdrop;      /**< dropped packets */
	uint32 txaddbareq;  /**< addba req sent */
	uint32 txaddbaresp; /**< addba resp sent */
	uint32 txdelba;     /**< delba sent */
	uint32 txba;        /**< ba sent */
	uint32 txbar;       /**< bar sent */
	uint32 txpad[4];    /**< future */

	/* receive side counters */
	uint32 rxpdu;       /**< pdus recd */
	uint32 rxqed;       /**< pdus buffered before sending up */
	uint32 rxdup;       /**< duplicate pdus */
	uint32 rxnobuf;     /**< pdus discarded due to no buf */
	uint32 rxaddbareq;  /**< addba req recd */
	uint32 rxaddbaresp; /**< addba resp recd */
	uint32 rxdelba;     /**< delba recd */
	uint32 rxba;        /**< ba recd */
	uint32 rxbar;       /**< bar recd */
	uint32 rxinvba;     /**< invalid ba recd */
	uint32 rxbaholes;   /**< ba recd with holes */
	uint32 rxunexp;     /**< unexpected packets */
	uint32 rxpad[4];    /**< future */
} wlc_ba_cnt_t;
/* #endif  WLBA */

/** structure for per-tid ampdu control */
struct ampdu_tid_control {
	uint8 tid;			/* tid */
	uint8 enable;			/* enable/disable */
};

/** struct for ampdu tx/rx aggregation control */
struct ampdu_aggr {
	int8 aggr_override;	/**< aggr overrided by dongle. Not to be set by host. */
	uint16 conf_TID_bmap;	/**< bitmap of TIDs to configure */
	uint16 enab_TID_bmap;	/**< enable/disable per TID */
};

/** structure for identifying ea/tid for sending addba/delba */
struct ampdu_ea_tid {
	struct ether_addr ea;		/**< Station address */
	uint8 tid;			/**< tid */
	uint8 initiator;	/**< 0 is recipient, 1 is originator */
};

/** structure for identifying retry/tid for retry_limit_tid/rr_retry_limit_tid */
struct ampdu_retry_tid {
	uint8 tid;	/**< tid */
	uint8 retry;	/**< retry value */
};

#define BDD_FNAME_LEN       32  /**< Max length of friendly name */
typedef struct bdd_fname {
	uint8 len;          /**< length of friendly name */
	uchar name[BDD_FNAME_LEN];  /**< friendly name */
} bdd_fname_t;

/* structure for addts arguments */
/** For ioctls that take a list of TSPEC */
struct tslist {
	int32 count;			/**< number of tspecs */
	struct tsinfo_arg tsinfo[];	/**< variable length array of tsinfo */
};

/* WLTDLS */
/**structure for tdls iovars */
typedef struct tdls_iovar {
	struct ether_addr ea;		/**< Station address */
	uint8 mode;			/**< mode: depends on iovar */
	uint8 PAD;
	chanspec_t chanspec;
	uint8 PAD[6];
} tdls_iovar_t;

#define TDLS_WFD_IE_SIZE		512
/**structure for tdls wfd ie */
typedef struct tdls_wfd_ie_iovar {
	struct ether_addr ea;		/**< Station address */
	uint8 mode;
	uint8 PAD;
	uint16 length;
	uint8 data[TDLS_WFD_IE_SIZE];
} tdls_wfd_ie_iovar_t;
/* #endif WLTDLS */

/** structure for addts/delts arguments */
typedef struct tspec_arg {
	uint16 version;			/**< see definition of TSPEC_ARG_VERSION */
	uint16 length;			/**< length of entire structure */
	uint32 flag;			/**< bit field */
	/* TSPEC Arguments */
	struct tsinfo_arg tsinfo;	/**< TS Info bit field */
	uint8  PAD;
	uint16 nom_msdu_size;		/**< (Nominal or fixed) MSDU Size (bytes) */
	uint16 max_msdu_size;		/**< Maximum MSDU Size (bytes) */
	uint32 min_srv_interval;		/**< Minimum Service Interval (us) */
	uint32 max_srv_interval;		/**< Maximum Service Interval (us) */
	uint32 inactivity_interval;	/**< Inactivity Interval (us) */
	uint32 suspension_interval;	/**< Suspension Interval (us) */
	uint32 srv_start_time;		/**< Service Start Time (us) */
	uint32 min_data_rate;		/**< Minimum Data Rate (bps) */
	uint32 mean_data_rate;		/**< Mean Data Rate (bps) */
	uint32 peak_data_rate;		/**< Peak Data Rate (bps) */
	uint32 max_burst_size;		/**< Maximum Burst Size (bytes) */
	uint32 delay_bound;		/**< Delay Bound (us) */
	uint32 min_phy_rate;		/**< Minimum PHY Rate (bps) */
	uint16 surplus_bw;		/**< Surplus Bandwidth Allowance (range 1.0 to 8.0) */
	uint16 medium_time;		/**< Medium Time (32 us/s periods) */
	uint8 dialog_token;		/**< dialog token */
	uint8  PAD[3];
} tspec_arg_t;

/** tspec arg for desired station */
typedef	struct tspec_per_sta_arg {
	struct ether_addr ea;
	uint8  PAD[2];
	struct tspec_arg ts;
} tspec_per_sta_arg_t;

/** structure for max bandwidth for each access category */
typedef	struct wme_max_bandwidth {
	uint32	ac[AC_COUNT];	/**< max bandwidth for each access category */
} wme_max_bandwidth_t;

#define WL_WME_MBW_PARAMS_IO_BYTES (sizeof(wme_max_bandwidth_t))

/* current version of wl_tspec_arg_t struct */
#define	TSPEC_ARG_VERSION		2	/**< current version of wl_tspec_arg_t struct */
#define TSPEC_ARG_LENGTH		55	/**< argument length from tsinfo to medium_time */
#define TSPEC_DEFAULT_DIALOG_TOKEN	42	/**< default dialog token */
#define TSPEC_DEFAULT_SBW_FACTOR	0x3000	/**< default surplus bw */

#define WL_WOWL_KEEPALIVE_MAX_PACKET_SIZE  80
#define WLC_WOWL_MAX_KEEPALIVE	2

/** Packet lifetime configuration per ac */
typedef struct wl_lifetime {
	uint32 ac;	        /**< access class */
	uint32 lifetime;    /**< Packet lifetime value in ms */
} wl_lifetime_t;

/** Management time configuration */
typedef struct wl_lifetime_mg {
	uint32 mgmt_bitmap;	/**< Mgmt subtype */
	uint32 lifetime;    /**< Packet lifetime value in us */
} wl_lifetime_mg_t;

/* MAC Sample Capture related */
#define	WL_MACCAPTR_DEFSTART_PTR	0xA00
#define	WL_MACCAPTR_DEFSTOP_PTR		0xA3F
#define	WL_MACCAPTR_DEFSZ		0x3F

#define WL_MACCAPTR_DEF_MASK		0xFFFFFFFF

typedef enum {
	WL_MACCAPT_TRIG		= 0,
	WL_MACCAPT_STORE	= 1,
	WL_MACCAPT_TRANS	= 2,
	WL_MACCAPT_MATCH	= 3
} maccaptr_optn;

typedef enum {
	WL_MACCAPT_STRT	= 1,
	WL_MACCAPT_STOP	= 2,
	WL_MACCAPT_RST	= 3
} maccaptr_cmd_t;

/* MAC Sample Capture Set-up Paramters */
typedef struct wl_maccapture_params {
	uint8	gpio_sel;
	uint8	la_mode;	/* TRUE: GPIO Out Enabled */
	uint8 	PAD[2];
	uint32	start_ptr;	/* Start address to store */
	uint32	stop_ptr;	/* Stop address to store */
	uint8	optn_bmp;	/* Options */
	uint8 	PAD[3];
	/* Don't change the order after this nor
	 * add anything in betw. Code uses offsets to populate
	 * registers
	 */
	uint32	tr_mask;	/* Trigger Mask */
	uint32	tr_val;		/* Trigger Value */
	uint32	s_mask;		/* Store Mode Mask */
	uint32	x_mask;		/* Trans. Mode Mask */
	uint32	m_mask;		/* Match Mode Mask */
	uint32	m_val;		/* Match Value */
	maccaptr_cmd_t cmd;	/* Start / Stop */
} wl_maccapture_params_t;

/** Channel Switch Announcement param */
typedef struct wl_chan_switch {
	uint8 mode;		/**< value 0 or 1 */
	uint8 count;		/**< count # of beacons before switching */
	chanspec_t chspec;	/**< chanspec */
	uint8 reg;		/**< regulatory class */
	uint8 frame_type;		/**< csa frame type, unicast or broadcast */
} wl_chan_switch_t;

enum {
	PFN_LIST_ORDER,
	PFN_RSSI
};

enum {
	DISABLE,
	ENABLE
};

enum {
	OFF_ADAPT,
	SMART_ADAPT,
	STRICT_ADAPT,
	SLOW_ADAPT
};

#define SORT_CRITERIA_BIT		0
#define AUTO_NET_SWITCH_BIT		1
#define ENABLE_BKGRD_SCAN_BIT		2
#define IMMEDIATE_SCAN_BIT		3
#define	AUTO_CONNECT_BIT		4
#define	ENABLE_BD_SCAN_BIT		5
#define ENABLE_ADAPTSCAN_BIT		6
#define IMMEDIATE_EVENT_BIT		8
#define SUPPRESS_SSID_BIT		9
#define ENABLE_NET_OFFLOAD_BIT		10
/** report found/lost events for SSID and BSSID networks seperately */
#define REPORT_SEPERATELY_BIT		11

#define SORT_CRITERIA_MASK	0x0001
#define AUTO_NET_SWITCH_MASK	0x0002
#define ENABLE_BKGRD_SCAN_MASK	0x0004
#define IMMEDIATE_SCAN_MASK	0x0008
#define AUTO_CONNECT_MASK	0x0010

#define ENABLE_BD_SCAN_MASK	0x0020
#define ENABLE_ADAPTSCAN_MASK	0x00c0
#define IMMEDIATE_EVENT_MASK	0x0100
#define SUPPRESS_SSID_MASK	0x0200
#define ENABLE_NET_OFFLOAD_MASK	0x0400
/** report found/lost events for SSID and BSSID networks seperately */
#define REPORT_SEPERATELY_MASK	0x0800

#define PFN_VERSION			2

#define PFN_COMPLETE			1
#define PFN_INCOMPLETE			0

#define DEFAULT_BESTN			2
#define DEFAULT_MSCAN			0
#define DEFAULT_REPEAT			10
#define DEFAULT_EXP			2

#define PFN_PARTIAL_SCAN_BIT		0
#define PFN_PARTIAL_SCAN_MASK		1

#define PFN_SWC_RSSI_WINDOW_MAX   8
#define PFN_SWC_MAX_NUM_APS       16
#define PFN_HOTLIST_MAX_NUM_APS   64

#define MAX_EPNO_HIDDEN_SSID    8
#define MAX_WHITELIST_SSID      2

/* Version 1 and 2 for various scan results structures defined below */
#define PFN_SCANRESULTS_VERSION_V1	1
#define PFN_SCANRESULTS_VERSION_V2	2

/** PFN network info structure */
typedef struct wl_pfn_subnet_info_v1 {
	struct ether_addr BSSID;
	uint8	channel; /**< channel number only */
	uint8	SSID_len;
	uint8	SSID[32];
} wl_pfn_subnet_info_v1_t;

typedef struct wl_pfn_subnet_info_v2 {
	struct ether_addr BSSID;
	uint8   channel; /**< channel number only */
	uint8   SSID_len;
	union {
		uint8   SSID[32];
		uint16 index;
	} u;
} wl_pfn_subnet_info_v2_t;

typedef struct wl_pfn_net_info_v1 {
	wl_pfn_subnet_info_v1_t pfnsubnet;
	int16	RSSI; /**< receive signal strength (in dBm) */
	uint16	timestamp; /**< age in seconds */
} wl_pfn_net_info_v1_t;

typedef struct wl_pfn_net_info_v2 {
	wl_pfn_subnet_info_v2_t pfnsubnet;
	int16   RSSI; /**< receive signal strength (in dBm) */
	uint16  timestamp; /**< age in seconds */
} wl_pfn_net_info_v2_t;

/* Version 1 and 2 for various lbest scan results structures below */
#define PFN_LBEST_SCAN_RESULT_VERSION_V1 1
#define PFN_LBEST_SCAN_RESULT_VERSION_V2 2

#define MAX_CHBKT_PER_RESULT		4

typedef struct wl_pfn_lnet_info_v1 {
	wl_pfn_subnet_info_v1_t pfnsubnet; /**< BSSID + channel + SSID len + SSID */
	uint16	flags; /**< partial scan, etc */
	int16	RSSI; /**< receive signal strength (in dBm) */
	uint32	timestamp; /**< age in miliseconds */
	uint16	rtt0; /**< estimated distance to this AP in centimeters */
	uint16	rtt1; /**< standard deviation of the distance to this AP in centimeters */
} wl_pfn_lnet_info_v1_t;

typedef struct wl_pfn_lnet_info_v2 {
	wl_pfn_subnet_info_v2_t pfnsubnet; /**< BSSID + channel + SSID len + SSID */
	uint16  flags; /**< partial scan, etc */
	int16   RSSI; /**< receive signal strength (in dBm) */
	uint32  timestamp; /**< age in miliseconds */
	uint16  rtt0; /**< estimated distance to this AP in centimeters */
	uint16  rtt1; /**< standard deviation of the distance to this AP in centimeters */
} wl_pfn_lnet_info_v2_t;

typedef struct wl_pfn_lscanresults_v1 {
	uint32 version;
	uint32 status;
	uint32 count;
	wl_pfn_lnet_info_v1_t netinfo[1];
} wl_pfn_lscanresults_v1_t;

typedef struct wl_pfn_lscanresults_v2 {
	uint32 version;
	uint16 status;
	uint16 count;
	uint32 scan_ch_buckets[MAX_CHBKT_PER_RESULT];
	wl_pfn_lnet_info_v2_t netinfo[1];
} wl_pfn_lscanresults_v2_t;

/**this is used to report on 1-* pfn scan results */
typedef struct wl_pfn_scanresults_v1 {
	uint32 version;
	uint32 status;
	uint32 count;
	wl_pfn_net_info_v1_t netinfo[1];
} wl_pfn_scanresults_v1_t;

typedef struct wl_pfn_scanresults_v2 {
	uint32 version;
	uint32 status;
	uint32 count;
	uint32 scan_ch_bucket;
	wl_pfn_net_info_v2_t netinfo[1];
} wl_pfn_scanresults_v2_t;

typedef struct wl_pfn_significant_net {
	uint16 flags;
	uint16 channel;
	struct ether_addr BSSID;
	int8 rssi[PFN_SWC_RSSI_WINDOW_MAX];
} wl_pfn_significant_net_t;

#define PFN_SWC_SCANRESULT_VERSION     1

typedef struct wl_pfn_swc_results {
	uint32 version;
	uint32 pkt_count;   /**< No. of results in current frame */
	uint32 total_count; /**< Total expected results */
	wl_pfn_significant_net_t list[];
} wl_pfn_swc_results_t;
typedef struct wl_pfn_net_info_bssid {
	struct ether_addr BSSID;
	uint8 channel;	/**< channel number only */
	int8  RSSI;	/**< receive signal strength (in dBm) */
	uint16 flags;	/**< (e.g. partial scan, off channel) */
	uint16 timestamp; /**< age in seconds */
} wl_pfn_net_info_bssid_t;

typedef struct wl_pfn_scanhist_bssid {
	uint32 version;
	uint32 status;
	uint32 count;
	wl_pfn_net_info_bssid_t netinfo[1];
} wl_pfn_scanhist_bssid_t;

/* Version 1 and 2 for various single scan result */
#define PFN_SCANRESULT_VERSION_V1	1
#define PFN_SCANRESULT_VERSION_V2	2

/* used to report exactly one scan result */
/* plus reports detailed scan info in bss_info */
typedef struct wl_pfn_scanresult_v1 {
	uint32 version;
	uint32 status;
	uint32 count;
	wl_pfn_net_info_v1_t netinfo;
	wl_bss_info_v109_t bss_info;
} wl_pfn_scanresult_v1_t;

typedef struct wl_pfn_scanresult_v2 {
	uint32 version;
	uint32 status;
	uint32 count;
	wl_pfn_net_info_v2_t netinfo;
	wl_bss_info_v109_t bss_info;
} wl_pfn_scanresult_v2_t;

typedef struct wl_pfn_scanresult_v2_1 {
	uint32 version;
	uint32 status;
	uint32 count;
	wl_pfn_net_info_v2_t netinfo;
	uint8 bss_info[];	/* var length wl_bss_info_X structures */
} wl_pfn_scanresult_v2_1_t;

/**PFN data structure */
typedef struct wl_pfn_param {
	int32 version;			/**< PNO parameters version */
	int32 scan_freq;		/**< Scan frequency */
	int32 lost_network_timeout;	/**< Timeout in sec. to declare
								* discovered network as lost
								*/
	int16 flags;			/**< Bit field to control features
							* of PFN such as sort criteria auto
							* enable switch and background scan
							*/
	int16 rssi_margin;		/**< Margin to avoid jitter for choosing a
							* PFN based on RSSI sort criteria
							*/
	uint8 bestn; /**< number of best networks in each scan */
	uint8 mscan; /**< number of scans recorded */
	uint8 repeat; /**< Minimum number of scan intervals
				     *before scan frequency changes in adaptive scan
				     */
	uint8 exp; /**< Exponent of 2 for maximum scan interval */
	int32 slow_freq; /**< slow scan period */
} wl_pfn_param_t;

typedef struct wl_pfn_bssid {
	struct ether_addr  macaddr;
	/* Bit4: suppress_lost, Bit3: suppress_found */
	uint16             flags;
} wl_pfn_bssid_t;
typedef struct wl_pfn_significant_bssid {
	struct ether_addr	macaddr;
	int8    rssi_low_threshold;
	int8    rssi_high_threshold;
} wl_pfn_significant_bssid_t;
#define WL_PFN_SUPPRESSFOUND_MASK	0x08
#define WL_PFN_SUPPRESSLOST_MASK	0x10
#define WL_PFN_SSID_IMPRECISE_MATCH	0x80
#define WL_PFN_SSID_SAME_NETWORK	0x10000
#define WL_PFN_SUPPRESS_AGING_MASK	0x20000
#define WL_PFN_FLUSH_ALL_SSIDS		0x40000

#define WL_PFN_IOVAR_FLAG_MASK		0xFFFF00FF
#define WL_PFN_RSSI_MASK		0xff00
#define WL_PFN_RSSI_SHIFT		8

typedef struct wl_pfn_cfg {
	uint32	reporttype;
	int32	channel_num;
	uint16	channel_list[WL_NUMCHANNELS];
	uint32	flags;
} wl_pfn_cfg_t;

#define WL_PFN_SSID_CFG_VERSION		1
#define WL_PFN_SSID_CFG_CLEAR		0x1

typedef struct wl_pfn_ssid_params {
	int8 min5G_rssi;           /* minimum 5GHz RSSI for a BSSID to be considered      */
	int8 min2G_rssi;           /* minimum 2.4GHz RSSI for a BSSID to be considered   */
	int16 init_score_max;     /* The maximum score that a network can have before bonuses  */

	int16 cur_bssid_bonus;    /* Add to current bssid                                      */
	int16 same_ssid_bonus;    /* score bonus for all networks with the same network flag   */
	int16 secure_bonus;       /* score bonus for networks that are not open          */
	int16 band_5g_bonus;
} wl_pfn_ssid_params_t;

typedef struct wl_ssid_ext_params {
	int8 min5G_rssi;	/* minimum 5GHz RSSI for a BSSID to be considered      */
	int8 min2G_rssi;	/* minimum 2.4GHz RSSI for a BSSID to be considered   */
	int16 init_score_max;	/* The maximum score that a network can have before bonuses  */
	int16 cur_bssid_bonus;	/* Add to current bssid                                      */
	int16 same_ssid_bonus;	/* score bonus for all networks with the same network flag   */
	int16 secure_bonus;	/* score bonus for networks that are not open                */
	int16 band_5g_bonus;
} wl_ssid_ext_params_t;

typedef struct wl_pfn_ssid_cfg {
	uint16 version;
	uint16 flags;
	wl_ssid_ext_params_t params;
} wl_pfn_ssid_cfg_t;

#define CH_BUCKET_REPORT_NONE                   0
#define CH_BUCKET_REPORT_SCAN_COMPLETE_ONLY     1
#define CH_BUCKET_REPORT_FULL_RESULT            2
#define CH_BUCKET_REPORT_SCAN_COMPLETE    (CH_BUCKET_REPORT_SCAN_COMPLETE_ONLY | \
								CH_BUCKET_REPORT_FULL_RESULT)
#define CH_BUCKET_REPORT_REGULAR            0
#define CH_BUCKET_GSCAN                     4

typedef struct wl_pfn_gscan_ch_bucket_cfg {
	uint8 bucket_end_index;
	uint8 bucket_freq_multiple;
	uint8 flag;
	uint8 reserved;
	uint16 repeat;
	uint16 max_freq_multiple;
} wl_pfn_gscan_ch_bucket_cfg_t;

typedef struct wl_pfn_capabilities {
	uint16 max_mscan;
	uint16 max_bestn;
	uint16 max_swc_bssid;
	uint16 max_hotlist_bssid;
} wl_pfn_capabilities_t;

#define GSCAN_SEND_ALL_RESULTS_MASK          (1 << 0)
#define GSCAN_ALL_BUCKETS_IN_FIRST_SCAN_MASK (1 << 3)
#define GSCAN_CFG_FLAGS_ONLY_MASK            (1 << 7)
#define WL_GSCAN_CFG_VERSION                     1
typedef struct wl_pfn_gscan_cfg {
	uint16 version;
	/**
	 * BIT0 1 = send probes/beacons to HOST
	 * BIT1 Reserved
	 * BIT2 Reserved
	 * Add any future flags here
	 * BIT7 1 = no other useful cfg sent
	 */
	uint8  flags;
	/** Buffer filled threshold in % to generate an event */
	uint8   buffer_threshold;
	/**
	 * No. of BSSIDs with "change" to generate an evt
	 * change - crosses rssi threshold/lost
	 */
	uint8   swc_nbssid_threshold;
	/* Max=8 (for now) Size of rssi cache buffer */
	uint8  swc_rssi_window_size;
	uint8  count_of_channel_buckets;
	uint8  retry_threshold;
	uint16  lost_ap_window;
	wl_pfn_gscan_ch_bucket_cfg_t channel_bucket[1];
} wl_pfn_gscan_cfg_t;

#define WL_PFN_REPORT_ALLNET    0
#define WL_PFN_REPORT_SSIDNET   1
#define WL_PFN_REPORT_BSSIDNET  2

#define WL_PFN_CFG_FLAGS_PROHIBITED	0x00000001	/* Accept and use prohibited channels */
#define WL_PFN_CFG_FLAGS_RESERVED	0xfffffffe	/**< Remaining reserved for future use */

typedef struct wl_pfn {
	wlc_ssid_t		ssid;			/**< ssid name and its length */
	int32			flags;			/**< bit2: hidden */
	int32			infra;			/**< BSS Vs IBSS */
	int32			auth;			/**< Open Vs Closed */
	int32			wpa_auth;		/**< WPA type */
	int32			wsec;			/**< wsec value */
} wl_pfn_t;

typedef struct wl_pfn_list {
	uint32		version;
	uint32		enabled;
	uint32		count;
	wl_pfn_t	pfn[1];
} wl_pfn_list_t;

#define PFN_SSID_EXT_VERSION   1

typedef struct wl_pfn_ext {
	uint8 flags;
	int8 rssi_thresh; /* RSSI threshold, track only if RSSI > threshold */
	uint16 wpa_auth; /* Match the wpa auth type defined in wlioctl_defs.h */
	uint8 ssid[DOT11_MAX_SSID_LEN];
	uint8 ssid_len;
	uint8 pad;
} wl_pfn_ext_t;
typedef struct wl_pfn_ext_list {
	uint16 version;
	uint16 count;
	wl_pfn_ext_t pfn_ext[1];
} wl_pfn_ext_list_t;

#define WL_PFN_SSID_EXT_FOUND   0x1
#define WL_PFN_SSID_EXT_LOST    0x2
typedef struct wl_pfn_result_ssid {
	uint8 flags;
	int8 rssi;
	/* channel number */
	uint16 channel;
	/* Assume idx in order of cfg */
	uint32 index;
} wl_pfn_result_ssid_crc32_t;

typedef struct wl_pfn_ssid_ext_result {
	uint16 version;
	uint16 count;
	wl_pfn_result_ssid_crc32_t net[1];
} wl_pfn_ssid_ext_result_t;

#define PFN_EXT_AUTH_CODE_OPEN   1 /* open */
#define PFN_EXT_AUTH_CODE_PSK   2 /* WPA_PSK or WPA2PSK */
#define PFN_EXT_AUTH_CODE_EAPOL 4 /* any EAPOL  */

#define WL_PFN_HIDDEN_BIT		2
#define WL_PFN_HIDDEN_MASK		0x4

#ifndef BESTN_MAX
#define BESTN_MAX			10
#endif // endif

#ifndef MSCAN_MAX
#define MSCAN_MAX			90
#endif // endif

/* Dynamic scan configuration for motion profiles */

#define WL_PFN_MPF_VERSION 1

/* Valid group IDs, may be expanded in the future */
#define WL_PFN_MPF_GROUP_SSID 0
#define WL_PFN_MPF_GROUP_BSSID 1
#define WL_PFN_MPF_MAX_GROUPS 2

/* Max number of MPF states supported in this time */
#define WL_PFN_MPF_STATES_MAX 4

/* Flags for the mpf-specific stuff */
#define WL_PFN_MPF_ADAPT_ON_BIT		0
#define WL_PFN_MPF_ADAPTSCAN_BIT	1

#define WL_PFN_MPF_ADAPT_ON_MASK	0x0001
#define WL_PFN_MPF_ADAPTSCAN_MASK 	0x0006

/* Per-state timing values */
typedef struct wl_pfn_mpf_state_params {
	int32  scan_freq;	/* Scan frequency (secs) */
	int32  lost_network_timeout; /* Timeout to declare net lost (secs) */
	int16  flags;		/* Space for flags: ADAPT etc */
	uint8  exp;		/* Exponent of 2 for max interval for SMART/STRICT_ADAPT */
	uint8  repeat;		/* Number of scans before changing adaptation level */
	int32  slow_freq;	/* Slow scan period for SLOW_ADAPT */
} wl_pfn_mpf_state_params_t;

typedef struct wl_pfn_mpf_param {
	uint16 version;		/* Structure version */
	uint16 groupid;		/* Group ID: 0 (SSID), 1 (BSSID), other: reserved */
	wl_pfn_mpf_state_params_t state[WL_PFN_MPF_STATES_MAX];
} wl_pfn_mpf_param_t;

/* Structure for setting pfn_override iovar */
typedef struct wl_pfn_override_param {
	uint16 version;         /* Structure version */
	uint16 start_offset;    /* Seconds from now to apply new params */
	uint16 duration;        /* Seconds to keep new params applied */
	uint16 reserved;
	wl_pfn_mpf_state_params_t override;
} wl_pfn_override_param_t;
#define WL_PFN_OVERRIDE_VERSION	1

/*
 * Definitions for base MPF configuration
 */

#define WL_MPF_VERSION 1
#define WL_MPF_MAX_BITS 3
#define WL_MPF_MAX_STATES (1 << WL_MPF_MAX_BITS)

#define WL_MPF_STATE_NAME_MAX 12

typedef struct wl_mpf_val {
	uint16 val;		/* Value of GPIO bits */
	uint16 state;		/* State identifier */
	char name[WL_MPF_STATE_NAME_MAX]; /* Optional name */
} wl_mpf_val_t;

typedef struct wl_mpf_map {
	uint16 version;
	uint16 type;
	uint16 mask;		/* Which GPIO bits to use */
	uint8  count;		/* Count of state/value mappings */
	uint8  PAD;
	wl_mpf_val_t vals[WL_MPF_MAX_STATES];
} wl_mpf_map_t;

#define WL_MPF_STATE_AUTO (0xFFFF) /* (uint16)-1) */

typedef struct wl_mpf_state {
	uint16 version;
	uint16 type;
	uint16 state;		/* Get/Set */
	uint8 force;		/* 0 - auto (HW) state, 1 - forced state */
	char name[WL_MPF_STATE_NAME_MAX]; /* Get/Set: Optional/actual name */
	uint8  PAD;
} wl_mpf_state_t;
/*
 * WLFCTS definition
 */
typedef struct wl_txstatus_additional_info {
	uint32 rspec;
	uint32 enq_ts;
	uint32 last_ts;
	uint32 entry_ts;
	uint16 seq;
	uint8  rts_cnt;
	uint8  tx_cnt;
} wl_txstatus_additional_info_t;

/** Service discovery */
typedef struct {
	uint8	transaction_id;	/**< Transaction id */
	uint8	protocol;	/**< Service protocol type */
	uint16	query_len;	/**< Length of query */
	uint16	response_len;	/**< Length of response */
	uint8	qrbuf[];
} wl_p2po_qr_t;

typedef struct {
	uint16			period;			/**< extended listen period */
	uint16			interval;		/**< extended listen interval */
	uint16                  count;                  /* count to repeat */
	uint16                  pad;                    /* pad for 32bit align */
} wl_p2po_listen_t;

/** GAS state machine tunable parameters.  Structure field values of 0 means use the default. */
typedef struct wl_gas_config {
	uint16 max_retransmit;		/**< Max # of firmware/driver retransmits on no Ack
					 * from peer (on top of the ucode retries).
					 */
	uint16 response_timeout;	/**< Max time to wait for a GAS-level response
					 * after sending a packet.
					 */
	uint16 max_comeback_delay;	/**< Max GAS response comeback delay.
					 * Exceeding this fails the GAS exchange.
					 */
	uint16 max_retries;		/**< Max # of GAS state machine retries on failure
					 * of a GAS frame exchange.
					 */
} wl_gas_config_t;

/** P2P Find Offload parameters */
typedef struct wl_p2po_find_config {
	uint16 version;			/**< Version of this struct */
	uint16 length;			/**< sizeof(wl_p2po_find_config_t) */
	int32 search_home_time;		/**< P2P search state home time when concurrent
					 * connection exists.  -1 for default.
					 */
	uint8 num_social_channels;
			/**< Number of social channels up to WL_P2P_SOCIAL_CHANNELS_MAX.
			 * 0 means use default social channels.
			 */
	uint8 flags;
	uint16 social_channels[1];	/**< Variable length array of social channels */
} wl_p2po_find_config_t;
#define WL_P2PO_FIND_CONFIG_VERSION 2	/**< value for version field */

/** wl_p2po_find_config_t flags */
#define P2PO_FIND_FLAG_SCAN_ALL_APS 0x01	/**< Whether to scan for all APs in the p2po_find
						 * periodic scans of all channels.
						 * 0 means scan for only P2P devices.
						 * 1 means scan for P2P devices plus non-P2P APs.
						 */

/** For adding a WFDS service to seek */
typedef struct {
	uint32 seek_hdl;		/**< unique id chosen by host */
	uint8 addr[6];			/**< Seek service from a specific device with this
					 * MAC address, all 1's for any device.
					 */
	uint8 service_hash[P2P_WFDS_HASH_LEN];
	uint8 service_name_len;
	uint8 service_name[MAX_WFDS_SEEK_SVC_NAME_LEN];
					/**< Service name to seek, not null terminated */
	uint8 service_info_req_len;
	uint8 service_info_req[1];	/**< Service info request, not null terminated.
					 * Variable length specified by service_info_req_len.
					 * Maximum length is MAX_WFDS_SEEK_SVC_INFO_LEN.
					 */
} wl_p2po_wfds_seek_add_t;

/** For deleting a WFDS service to seek */
typedef struct {
	uint32 seek_hdl;		/**< delete service specified by id */
} wl_p2po_wfds_seek_del_t;

/** For adding a WFDS service to advertise */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint32 advertise_hdl;		/**< unique id chosen by host */
	uint8 service_hash[P2P_WFDS_HASH_LEN];
	uint32 advertisement_id;
	uint16 service_config_method;
	uint8 service_name_len;
	uint8 service_name[MAX_WFDS_SVC_NAME_LEN];
					/**< Service name , not null terminated */
	uint8 service_status;
	uint16 service_info_len;
	uint8 service_info[1];		/**< Service info, not null terminated.
					 * Variable length specified by service_info_len.
					 * Maximum length is MAX_WFDS_ADV_SVC_INFO_LEN.
					 */
} BWL_POST_PACKED_STRUCT wl_p2po_wfds_advertise_add_t;
#include <packed_section_end.h>

/** For deleting a WFDS service to advertise */
typedef struct {
	uint32 advertise_hdl;	/**< delete service specified by hdl */
} wl_p2po_wfds_advertise_del_t;

/** P2P Offload discovery mode for the p2po_state iovar */
typedef enum {
	WL_P2PO_DISC_STOP,
	WL_P2PO_DISC_LISTEN,
	WL_P2PO_DISC_DISCOVERY
} disc_mode_t;

/* ANQP offload */

#define ANQPO_MAX_QUERY_SIZE		256
typedef struct {
	uint16 max_retransmit;		/**< ~0 use default, max retransmit on no ACK from peer */
	uint16 response_timeout; /**< ~0 use default, msec to wait for resp after tx packet */
	uint16 max_comeback_delay;	/**< ~0 use default, max comeback delay in resp else fail */
	uint16 max_retries;		/**< ~0 use default, max retries on failure */
	uint16 query_len;		/**< length of ANQP query */
	uint8 query_data[1];		/**< ANQP encoded query (max ANQPO_MAX_QUERY_SIZE) */
} wl_anqpo_set_t;

#define WL_ANQPO_FLAGS_BSSID_WILDCARD		0x0001
#define WL_ANQPO_PEER_LIST_VERSION_2		2
typedef struct {
	uint16 channel;			/**< channel of the peer */
	struct ether_addr addr;		/**< addr of the peer */
} wl_anqpo_peer_v1_t;
typedef struct {
	uint16 channel;			/**< channel of the peer */
	struct ether_addr addr;		/**< addr of the peer */
	uint32 flags;			/**< 0x01-Peer is MBO Capable */
} wl_anqpo_peer_v2_t;

#define ANQPO_MAX_PEER_LIST			64
typedef struct {
	uint16 count;				/**< number of peers in list */
	wl_anqpo_peer_v1_t peer[1];	/**< max ANQPO_MAX_PEER_LIST */
} wl_anqpo_peer_list_v1_t;

typedef struct {
	uint16  version;    /**<VERSION */
	uint16  length;     /**< length of entire structure */
	uint16 count;				/**< number of peers in list */
	wl_anqpo_peer_v2_t peer[1];	/**< max ANQPO_MAX_PEER_LIST */
} wl_anqpo_peer_list_v2_t;

#ifndef WL_ANQPO_PEER_LIST_TYPEDEF_HAS_ALIAS
typedef wl_anqpo_peer_list_v1_t wl_anqpo_peer_list_t;
typedef wl_anqpo_peer_v1_t wl_anqpo_peer_t;
#endif /* WL_ANQPO_PEER_LIST_TYPEDEF_HAS_ALIAS */

#define ANQPO_MAX_IGNORE_SSID		64
typedef struct {
	uint8 is_clear;				/**< set to clear list (not used on GET) */
	uint8 PAD;
	uint16 count;				/**< number of SSID in list */
	wlc_ssid_t ssid[1];			/**< max ANQPO_MAX_IGNORE_SSID */
} wl_anqpo_ignore_ssid_list_t;

#define ANQPO_MAX_IGNORE_BSSID		64
typedef struct {
	uint8 is_clear;				/**< set to clear list (not used on GET) */
	uint8 PAD;
	uint16 count;				/**< number of addr in list */
	struct ether_addr bssid[];	/**< max ANQPO_MAX_IGNORE_BSSID */
} wl_anqpo_ignore_bssid_list_t;

struct toe_ol_stats_t {
	/** Num of tx packets that don't need to be checksummed */
	uint32 tx_summed;

	/* Num of tx packets where checksum is filled by offload engine */
	uint32 tx_iph_fill;
	uint32 tx_tcp_fill;
	uint32 tx_udp_fill;
	uint32 tx_icmp_fill;

	/*  Num of rx packets where toe finds out if checksum is good or bad */
	uint32 rx_iph_good;
	uint32 rx_iph_bad;
	uint32 rx_tcp_good;
	uint32 rx_tcp_bad;
	uint32 rx_udp_good;
	uint32 rx_udp_bad;
	uint32 rx_icmp_good;
	uint32 rx_icmp_bad;

	/* Num of tx packets in which csum error is injected */
	uint32 tx_tcp_errinj;
	uint32 tx_udp_errinj;
	uint32 tx_icmp_errinj;

	/* Num of rx packets in which csum error is injected */
	uint32 rx_tcp_errinj;
	uint32 rx_udp_errinj;
	uint32 rx_icmp_errinj;
};

/** Arp offload statistic counts */
struct arp_ol_stats_t {
	uint32  host_ip_entries; /**< Host IP table addresses (more than one if multihomed) */
	uint32  host_ip_overflow;	/**< Host IP table additions skipped due to overflow */

	uint32  arp_table_entries;	/**< ARP table entries */
	uint32  arp_table_overflow;	/**< ARP table additions skipped due to overflow */

	uint32  host_request;		/**< ARP requests from host */
	uint32  host_reply;		/**< ARP replies from host */
	uint32  host_service;		/**< ARP requests from host serviced by ARP Agent */

	uint32  peer_request;		/**< ARP requests received from network */
	uint32  peer_request_drop;	/**< ARP requests from network that were dropped */
	uint32  peer_reply;		/**< ARP replies received from network */
	uint32  peer_reply_drop;	/**< ARP replies from network that were dropped */
	uint32  peer_service;		/**< ARP request from host serviced by ARP Agent */
};

/** NS offload statistic counts */
struct nd_ol_stats_t {
	uint32  host_ip_entries;    /**< Host IP table addresses (more than one if multihomed) */
	uint32  host_ip_overflow;   /**< Host IP table additions skipped due to overflow */
	uint32  peer_request;       /**< NS requests received from network */
	uint32  peer_request_drop;  /**< NS requests from network that were dropped */
	uint32  peer_reply_drop;    /**< NA replies from network that were dropped */
	uint32  peer_service;       /**< NS request from host serviced by firmware */
};

/*
 * Neighbor Discovery Offloading
 */
enum {
	WL_ND_IPV6_ADDR_TYPE_UNICAST = 0,
	WL_ND_IPV6_ADDR_TYPE_ANYCAST
};

typedef struct wl_nd_host_ip_addr {
	struct ipv6_addr ip_addr;	/* host ip address */
	uint8 type;			/* type of address */
	uint8 pad[3];
} wl_nd_host_ip_addr_t;

typedef struct wl_nd_host_ip_list {
	uint32 count;
	wl_nd_host_ip_addr_t host_ip[1];
} wl_nd_host_ip_list_t;

#define WL_ND_HOSTIP_IOV_VER    1

enum {
	WL_ND_HOSTIP_OP_VER = 0,	/* get version */
	WL_ND_HOSTIP_OP_ADD,		/* add address */
	WL_ND_HOSTIP_OP_DEL,		/* delete specified address */
	WL_ND_HOSTIP_OP_DEL_UC,		/* delete all unicast address */
	WL_ND_HOSTIP_OP_DEL_AC,		/* delete all anycast address */
	WL_ND_HOSTIP_OP_DEL_ALL,	/* delete all addresses */
	WL_ND_HOSTIP_OP_LIST,		/* get list of host ip address */
	WL_ND_HOSTIP_OP_MAX
};

typedef struct wl_nd_hostip {
	uint16 version;				/* version of iovar buf */
	uint16 op_type;				/* operation type */
	uint32 length;				/* length of entire structure */
	union {
		wl_nd_host_ip_addr_t host_ip;	/* set param for add */
		uint16 version;			/* get return for ver */
	} u;
} wl_nd_hostip_t;

#define WL_ND_HOSTIP_FIXED_LEN		OFFSETOF(wl_nd_hostip_t, u)
#define WL_ND_HOSTIP_WITH_ADDR_LEN	(WL_ND_HOSTIP_FIXED_LEN + sizeof(wl_nd_host_ip_addr_t))

/*
 * Keep-alive packet offloading.
 */

/**
 * NAT keep-alive packets format: specifies the re-transmission period, the packet
 * length, and packet contents.
 */
typedef struct wl_keep_alive_pkt {
	uint32	period_msec;	/** Retransmission period (0 to disable packet re-transmits) */
	uint16	len_bytes;	/* Size of packet to transmit (0 to disable packet re-transmits) */
	uint8	data[1];	/** Variable length packet to transmit.  Contents should include
				 * entire ethernet packet (enet header, IP header, UDP header,
				 * and UDP payload) in network byte order.
				 */
} wl_keep_alive_pkt_t;

#define WL_KEEP_ALIVE_FIXED_LEN		OFFSETOF(wl_keep_alive_pkt_t, data)

#define MAX_RSSI_COUNT			8
typedef struct rssi_struct {
	int8	val[MAX_RSSI_COUNT];	/**< rssi values in AFs */
	int16	sum;			/**< total rssi sum */
	uint8	cnt;			/**< number rssi samples */
	uint8	idx;			/**< next rssi location */
} rssi_struct_t;

#ifdef WLDFSP
#define DFSP_EVT_OFFSET			OFFSETOF(dfsp_event_data_t, ie)
#define DFSP_EVT_FLAGS_AP_ASSOC		(1 << 0)
#define DFSP_EVT_FLAGS_AP_BCNMON	(1 << 1)
#define DFSP_EVT_FLAGS_PROXY_BCSA	(1 << 2)
#define DFSP_EVT_FLAGS_PROXY_UCSA	(1 << 3)
#define DFSP_EVT_FLAGS_PROXY_PCSA	(1 << 4)

typedef struct dfsp_event_data {
	uint16 flags;	/* indicate what triggers the event */
	uint16 ie_len;
	uint8 ie[];    /* variable length */
} dfsp_event_data_t;

/* Proxy Channel Switch Announcement is a collection of IEs */
typedef struct dfsp_pcsa {
	dot11_ext_csa_ie_t		ecsa;
	dot11_mesh_csp_ie_t		mcsp;
	dot11_wide_bw_chan_switch_ie_t	wbcs;
} dfsp_pcsa_t;

/* DFS Proxy */
#define DFSP_CFG_VERSION	1
#define DFSP_FLAGS_ENAB		0x1
typedef struct dfsp_cfg {
	uint16 version;
	uint16 len;
	uint16 flags;			/**< bit 1 to enable/disable the feature */
	uint16 max_bcn_miss_dur;        /**< maximum beacon miss duration before ceasing data tx */
	uint8 mcsp_ttl;                 /**< remaining number of hops allowed for pcsa message */
	uint8 bcsa_cnt;                 /**< repeat numbers of broadcast CSA */
	chanspec_t mon_chan;            /**< passive monitoring channel spec */
	struct ether_addr mon_bssid;    /**< broadcast means monitoring all */
	uint16 max_bcn_miss_dur_af;	/**< maximum beacon miss duration before ceasing AF tx */
} dfsp_cfg_t;

#define DFSP_UCSA_VERSION	1
typedef struct dfsp_ucsa {
	uint16 version;
	uint16 len;
	struct ether_addr address;
	uint8 enable;
	uint8 retry_cnt;		/**< just in case host needs to control the value */
} dfsp_ucsa_t;

typedef struct dfsp_ucsa_tbl {
	uint8 tbl_num;
	uint8 tbl[];
} dfsp_ucsa_tbl_t;

typedef struct dfsp_stats {
	uint32  dfsp_csainfra;
	uint32  dfsp_csabcnmon;
	uint32  dfsp_bcsarx;
	uint32  dfsp_ucsarx;
	uint32  dfsp_pcsarx;
	uint32  dfsp_bcsatx;
	uint32  dfsp_ucsatx;
	uint32  dfsp_pcsatx;
	uint32  dfsp_ucsatxfail;
	uint32  dfsp_evtnotif;
	uint32  dfsp_evtsuspect;
	uint32  dfsp_evtresume;
} dfsp_stats_t;
#endif /* WLDFSP */

/*
 * ptk_start: iovar to start 4-way handshake for secured ranging
*/

/* ptk negotiation security type - determines negotiation parameters */
typedef enum {
	WL_PTK_START_SEC_TYPE_PMK = 1
} wl_ptk_start_sec_type_t;

/* ptk negotiation role */
typedef enum {
	ROLE_NONE	= 0x0,
	ROLE_AUTH	= 0x1,
	ROLE_SUP	= 0x2,
	ROLE_STATIC	= 0x3,
	ROLE_INVALID	= 0xff,
	WL_PTK_START_ROLE_NONE = ROLE_NONE,
	WL_PTK_START_ROLE_AUTH = ROLE_AUTH,
	WL_PTK_START_ROLE_SUP = ROLE_SUP,
	WL_PTK_START_ROLE_STATIC = ROLE_STATIC,
	WL_PTK_START_ROLE_INVALID = ROLE_INVALID
} wl_ptk_start_role_t;

typedef struct wl_ptk_start_tlv {
	uint16 id;
	uint16 len;
	uint8 data[1];
} wl_ptk_start_tlv_t;

typedef enum {
	WL_PTK_START_TLV_PMK	= 1	/* uint8[] */
} wl_ptk_start_tlv_type;

typedef enum {
	WL_PTK_START_FLAG_NO_DATA_PROT	= 1,	/* data frame protection disabled */
	WL_PTK_START_FLAG_GEN_FTM_TPK	= 2	/* Generate FTM Toast/Seq Protection Key */
} wl_ptk_start_flags_t;

typedef struct wl_ptk_start_iov {
	uint16 version;
	uint16 len;				/* length of entire iov from version */
	wl_ptk_start_flags_t flags;
	wl_ptk_start_sec_type_t sec_type;
	wl_ptk_start_role_t role;
	struct ether_addr peer_addr;
	uint16 pad;				/* reserved/32 bit alignment */
	wl_ptk_start_tlv_t tlvs[1];
} wl_ptk_start_iov_t;

/*
 * Dongle pattern matching filter.
 */

#define MAX_WAKE_PACKET_CACHE_BYTES 128 /**< Maximum cached wake packet */

#define MAX_WAKE_PACKET_BYTES	    (DOT11_A3_HDR_LEN +			    \
				     DOT11_QOS_LEN +			    \
				     sizeof(struct dot11_llc_snap_header) + \
				     ETHER_MAX_DATA)

typedef struct pm_wake_packet {
	uint32	status;		/**< Is the wake reason a packet (if all the other field's valid) */
	uint32	pattern_id;	/**< Pattern ID that matched */
	uint32	original_packet_size;
	uint32	saved_packet_size;
	uint8	packet[MAX_WAKE_PACKET_CACHE_BYTES];
} pm_wake_packet_t;

/* Packet filter types. Currently, only pattern matching is supported. */
typedef enum wl_pkt_filter_type {
	WL_PKT_FILTER_TYPE_PATTERN_MATCH=0,       /**< Pattern matching filter */
	WL_PKT_FILTER_TYPE_MAGIC_PATTERN_MATCH=1, /**< Magic packet match */
	WL_PKT_FILTER_TYPE_PATTERN_LIST_MATCH=2,  /**< A pattern list (match all to match filter) */
	WL_PKT_FILTER_TYPE_ENCRYPTED_PATTERN_MATCH=3, /**< SECURE WOWL magic / net pattern match */
	WL_PKT_FILTER_TYPE_APF_MATCH=4, /* Android packet filter match */
	WL_PKT_FILTER_TYPE_PATTERN_MATCH_TIMEOUT=5, /* Pattern matching filter with timeout event */
	WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH=6, /* Immediately pattern matching filter */
	WL_PKT_FILTYER_TYPE_MAX = 7,	/* Pkt filter type MAX */
} wl_pkt_filter_type_t;

#define WL_PKT_FILTER_TYPE wl_pkt_filter_type_t

/* String mapping for types that may be used by applications or debug */
#define WL_PKT_FILTER_TYPE_NAMES \
	{ "PATTERN", WL_PKT_FILTER_TYPE_PATTERN_MATCH },	\
	{ "MAGIC",   WL_PKT_FILTER_TYPE_MAGIC_PATTERN_MATCH },	\
	{ "PATLIST", WL_PKT_FILTER_TYPE_PATTERN_LIST_MATCH },	\
	{ "SECURE WOWL", WL_PKT_FILTER_TYPE_ENCRYPTED_PATTERN_MATCH },	\
	{ "APF", WL_PKT_FILTER_TYPE_APF_MATCH }, \
	{ "PATTERN TIMEOUT", WL_PKT_FILTER_TYPE_PATTERN_MATCH_TIMEOUT }, \
	{ "IMMEDIATE", WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH }

/** Secured WOWL packet was encrypted, need decrypted before check filter match */
typedef struct wl_pkt_decrypter {
	uint8* (*dec_cb)(void* dec_ctx, const void *sdu, int sending);
	void*  dec_ctx;
} wl_pkt_decrypter_t;

/**
 * Pattern matching filter. Specifies an offset within received packets to
 * start matching, the pattern to match, the size of the pattern, and a bitmask
 * that indicates which bits within the pattern should be matched.
 */
typedef struct wl_pkt_filter_pattern {
	uint32	offset;		/**< Offset within received packet to start pattern matching.
				 * Offset '0' is the first byte of the ethernet header.
				 */
	uint32	size_bytes;	/**< Size of the pattern.  Bitmask must be the same size. */
	uint8   mask_and_pattern[]; /**< Variable length mask and pattern data.  mask starts
				      * at offset 0.  Pattern immediately follows mask. for
				      * secured pattern, put the descrypter pointer to the
				      * beginning, mask and pattern postponed correspondingly
				      */
} wl_pkt_filter_pattern_t;

/** A pattern list is a numerically specified list of modified pattern structures. */
typedef struct wl_pkt_filter_pattern_listel {
	uint16 rel_offs;	/**< Offset to begin match (relative to 'base' below) */
	uint16 base_offs;	/**< Base for offset (defined below) */
	uint16 size_bytes;	/**< Size of mask/pattern */
	uint16 match_flags;	/**< Addition flags controlling the match */
	uint8  mask_and_data[]; /**< Variable length mask followed by data, each size_bytes */
} wl_pkt_filter_pattern_listel_t;

typedef struct wl_pkt_filter_pattern_list {
	uint8 list_cnt;		/**< Number of elements in the list */
	uint8 PAD1[1];		/**< Reserved (possible version: reserved) */
	uint16 totsize;		/**< Total size of this pattern list (includes this struct) */
	uint8 patterns[]; /**< Variable number of wl_pkt_filter_pattern_listel_t elements */
} wl_pkt_filter_pattern_list_t;

typedef struct wl_apf_program {
	uint16 version;
	uint16 instr_len;	/* number of instruction blocks */
	uint32 inst_ts;		/* program installation timestamp */
	uint8 instrs[];	/* variable length instructions */
} wl_apf_program_t;
