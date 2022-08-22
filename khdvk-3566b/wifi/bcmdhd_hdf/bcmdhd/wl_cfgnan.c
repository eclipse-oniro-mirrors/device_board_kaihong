/*
 * Neighbor Awareness Networking
 *
 * Copyright (C) 2022 Broadcom.
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
 * $Id: wl_cfgnan.c 825970 2019-06-18 05:28:31Z $
 */

#ifdef WL_NAN
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmwifi_channels.h>
#include <nan.h>
#include <bcmiov.h>
#include <net/rtnetlink.h>

#include <wl_cfg80211.h>
#include <wl_cfgscan.h>
#include <wl_android.h>
#include <wl_cfgnan.h>

#include <dngl_stats.h>
#include <dhd.h>
#ifdef RTT_SUPPORT
#include <dhd_rtt.h>
#endif /* RTT_SUPPORT */
#include <wl_cfgvendor.h>
#include <bcmbloom.h>
#include <wl_cfgp2p.h>
#ifdef RTT_SUPPORT
#include <dhd_rtt.h>
#endif /* RTT_SUPPORT */
#include <bcmstdlib_s.h>

#define NAN_RANGE_REQ_EVNT 1
#define NAN_RAND_MAC_RETRIES 10
#define NAN_SCAN_DWELL_TIME_DELTA_MS 10

#ifdef WL_NAN_DISC_CACHE
/* Disc Cache Parameters update Flags */
#define NAN_DISC_CACHE_PARAM_SDE_CONTROL	0x0001

static int wl_cfgnan_cache_disc_result(struct bcm_cfg80211 *cfg, void * data,
	u16 *disc_cache_update_flags);
static int wl_cfgnan_remove_disc_result(struct bcm_cfg80211 * cfg, uint8 local_subid);
static nan_disc_result_cache * wl_cfgnan_get_disc_result(struct bcm_cfg80211 *cfg,
	uint8 remote_pubid, struct ether_addr *peer);
#endif /* WL_NAN_DISC_CACHE */
static int wl_cfgnan_clear_disc_cache(struct bcm_cfg80211 *cfg, wl_nan_instance_id_t sub_id);
static int wl_cfgnan_set_if_addr(struct bcm_cfg80211 *cfg);

static int wl_cfgnan_get_capability(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, nan_hal_capabilities_t *capabilities);

static int32 wl_cfgnan_notify_disc_with_ranging(struct bcm_cfg80211 *cfg,
	nan_ranging_inst_t *rng_inst, nan_event_data_t *nan_event_data, uint32 distance);

static void wl_cfgnan_disc_result_on_geofence_cancel(struct bcm_cfg80211 *cfg,
	nan_ranging_inst_t *rng_inst);

static void wl_cfgnan_clear_nan_event_data(struct bcm_cfg80211 *cfg,
	nan_event_data_t *nan_event_data);

void wl_cfgnan_data_remove_peer(struct bcm_cfg80211 *cfg,
        struct ether_addr *peer_addr);

static void wl_cfgnan_send_stop_event(struct bcm_cfg80211 *cfg);

static void wl_cfgnan_terminate_ranging_session(struct bcm_cfg80211 *cfg,
	nan_ranging_inst_t *ranging_inst);

#ifdef RTT_SUPPORT
static s32 wl_cfgnan_clear_peer_ranging(struct bcm_cfg80211 * cfg,
	struct ether_addr * peer, int reason);
#endif /* RTT_SUPPORT */

static const char *nan_role_to_str(u8 role)
{
	switch (role) {
		C2S(WL_NAN_ROLE_AUTO)
		C2S(WL_NAN_ROLE_NON_MASTER_NON_SYNC)
		C2S(WL_NAN_ROLE_NON_MASTER_SYNC)
		C2S(WL_NAN_ROLE_MASTER)
		C2S(WL_NAN_ROLE_ANCHOR_MASTER)
		default:
			return "WL_NAN_ROLE_UNKNOWN";
	}
}

static const char *nan_event_to_str(u16 cmd)
{
	switch (cmd) {
	C2S(WL_NAN_EVENT_START)
	C2S(WL_NAN_EVENT_DISCOVERY_RESULT)
	C2S(WL_NAN_EVENT_TERMINATED)
	C2S(WL_NAN_EVENT_RECEIVE)
	C2S(WL_NAN_EVENT_MERGE)
	C2S(WL_NAN_EVENT_STOP)
	C2S(WL_NAN_EVENT_PEER_DATAPATH_IND)
	C2S(WL_NAN_EVENT_DATAPATH_ESTB)
	C2S(WL_NAN_EVENT_SDF_RX)
	C2S(WL_NAN_EVENT_DATAPATH_END)
	C2S(WL_NAN_EVENT_RNG_REQ_IND)
	C2S(WL_NAN_EVENT_RNG_RPT_IND)
	C2S(WL_NAN_EVENT_RNG_TERM_IND)
	C2S(WL_NAN_EVENT_TXS)
	C2S(WL_NAN_EVENT_INVALID)

	default:
		return "WL_NAN_EVENT_UNKNOWN";
	}
}

static int wl_cfgnan_execute_ioctl(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, bcm_iov_batch_buf_t *nan_buf,
	uint16 nan_buf_size, uint32 *status, uint8 *resp_buf,
	uint16 resp_buf_len);
int
wl_cfgnan_generate_inst_id(struct bcm_cfg80211 *cfg, uint8 *p_inst_id)
{
	s32 ret = BCME_OK;
	uint8 i = 0;
	if (p_inst_id == NULL) {
		WL_ERR(("Invalid arguments\n"));
		ret = -EINVAL;
		goto exit;
	}

	if (cfg->nancfg.inst_id_start == NAN_ID_MAX) {
		WL_ERR(("Consumed all IDs, resetting the counter\n"));
		cfg->nancfg.inst_id_start = 0;
	}

	for (i = cfg->nancfg.inst_id_start; i < NAN_ID_MAX; i++) {
		if (isclr(cfg->nancfg.svc_inst_id_mask, i)) {
			setbit(cfg->nancfg.svc_inst_id_mask, i);
			*p_inst_id = i + 1;
			cfg->nancfg.inst_id_start = *p_inst_id;
			WL_DBG(("Instance ID=%d\n", *p_inst_id));
			goto exit;
		}
	}
	WL_ERR(("Allocated maximum IDs\n"));
	ret = BCME_NORESOURCE;
exit:
	return ret;
}

int
wl_cfgnan_remove_inst_id(struct bcm_cfg80211 *cfg, uint8 inst_id)
{
	s32 ret = BCME_OK;
	WL_DBG(("%s: Removing svc instance id %d\n", __FUNCTION__, inst_id));
	clrbit(cfg->nancfg.svc_inst_id_mask, inst_id-1);
	return ret;
}
s32 wl_cfgnan_parse_sdea_data(osl_t *osh, const uint8 *p_attr,
		uint16 len, nan_event_data_t *tlv_data)
{
	const wifi_nan_svc_desc_ext_attr_t *nan_svc_desc_ext_attr = NULL;
	uint8 offset;
	s32 ret = BCME_OK;

	/* service descriptor ext attributes */
	nan_svc_desc_ext_attr = (const wifi_nan_svc_desc_ext_attr_t *)p_attr;

	/* attribute ID */
	WL_TRACE(("> attr id: 0x%02x\n", nan_svc_desc_ext_attr->id));

	/* attribute length */
	WL_TRACE(("> attr len: 0x%x\n", nan_svc_desc_ext_attr->len));
	if (nan_svc_desc_ext_attr->instance_id == tlv_data->pub_id) {
		tlv_data->sde_control_flag = nan_svc_desc_ext_attr->control;
	}
	offset = sizeof(*nan_svc_desc_ext_attr);
	if (offset > len) {
		WL_ERR(("Invalid event buffer len\n"));
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	p_attr += offset;
	len -= offset;

	if (tlv_data->sde_control_flag & NAN_SC_RANGE_LIMITED) {
		WL_TRACE(("> svc_control: range limited present\n"));
	}
	if (tlv_data->sde_control_flag & NAN_SDE_CF_SVC_UPD_IND_PRESENT) {
		WL_TRACE(("> svc_control: sdea svc specific info present\n"));
		tlv_data->sde_svc_info.dlen = (p_attr[1] | (p_attr[2] << 8));
		WL_TRACE(("> sdea svc info len: 0x%02x\n", tlv_data->sde_svc_info.dlen));
		if (!tlv_data->sde_svc_info.dlen ||
				tlv_data->sde_svc_info.dlen > NAN_MAX_SERVICE_SPECIFIC_INFO_LEN) {
			/* must be able to handle null msg which is not error */
			tlv_data->sde_svc_info.dlen = 0;
			WL_ERR(("sde data length is invalid\n"));
			ret = BCME_BADLEN;
			goto fail;
		}

		if (tlv_data->sde_svc_info.dlen > 0) {
			tlv_data->sde_svc_info.data = MALLOCZ(osh, tlv_data->sde_svc_info.dlen);
			if (!tlv_data->sde_svc_info.data) {
				WL_ERR(("%s: memory allocation failed\n", __FUNCTION__));
				tlv_data->sde_svc_info.dlen = 0;
				ret = BCME_NOMEM;
				goto fail;
			}
			/* advance read pointer, consider sizeof of Service Update Indicator */
			offset = sizeof(tlv_data->sde_svc_info.dlen) - 1;
			if (offset > len) {
				WL_ERR(("Invalid event buffer len\n"));
				ret = BCME_BUFTOOSHORT;
				goto fail;
			}
			p_attr += offset;
			len -= offset;
			ret = memcpy_s(tlv_data->sde_svc_info.data, tlv_data->sde_svc_info.dlen,
				p_attr, tlv_data->sde_svc_info.dlen);
			if (ret != BCME_OK) {
				WL_ERR(("Failed to copy sde_svc_info\n"));
				goto fail;
			}
		} else {
			/* must be able to handle null msg which is not error */
			tlv_data->sde_svc_info.dlen = 0;
			WL_DBG(("%s: sdea svc info length is zero, null info data\n",
				__FUNCTION__));
		}
	}
	return ret;
fail:
	if (tlv_data->sde_svc_info.data) {
		MFREE(osh, tlv_data->sde_svc_info.data,
				tlv_data->sde_svc_info.dlen);
		tlv_data->sde_svc_info.data = NULL;
	}

	WL_DBG(("Parse SDEA event data, status = %d\n", ret));
	return ret;
}

/*
 * This attribute contains some mandatory fields and some optional fields
 * depending on the content of the service discovery request.
 */
s32
wl_cfgnan_parse_sda_data(osl_t *osh, const uint8 *p_attr,
		uint16 len, nan_event_data_t *tlv_data)
{
	uint8 svc_control = 0, offset = 0;
	s32 ret = BCME_OK;
	const wifi_nan_svc_descriptor_attr_t *nan_svc_desc_attr = NULL;

	/* service descriptor attributes */
	nan_svc_desc_attr = (const wifi_nan_svc_descriptor_attr_t *)p_attr;
	/* attribute ID */
	WL_TRACE(("> attr id: 0x%02x\n", nan_svc_desc_attr->id));

	/* attribute length */
	WL_TRACE(("> attr len: 0x%x\n", nan_svc_desc_attr->len));

	/* service ID */
	ret = memcpy_s(tlv_data->svc_name, sizeof(tlv_data->svc_name),
		nan_svc_desc_attr->svc_hash, NAN_SVC_HASH_LEN);
	if (ret != BCME_OK) {
		WL_ERR(("Failed to copy svc_hash_name:\n"));
		return ret;
	}
	WL_TRACE(("> svc_hash_name: " MACDBG "\n", MAC2STRDBG(tlv_data->svc_name)));

	/* local instance ID */
	tlv_data->local_inst_id = nan_svc_desc_attr->instance_id;
	WL_TRACE(("> local instance id: 0x%02x\n", tlv_data->local_inst_id));

	/* requestor instance ID */
	tlv_data->requestor_id = nan_svc_desc_attr->requestor_id;
	WL_TRACE(("> requestor id: 0x%02x\n", tlv_data->requestor_id));

	/* service control */
	svc_control = nan_svc_desc_attr->svc_control;
	if ((svc_control & NAN_SVC_CONTROL_TYPE_MASK) == NAN_SC_PUBLISH) {
		WL_TRACE(("> Service control type: NAN_SC_PUBLISH\n"));
	} else if ((svc_control & NAN_SVC_CONTROL_TYPE_MASK) == NAN_SC_SUBSCRIBE) {
		WL_TRACE(("> Service control type: NAN_SC_SUBSCRIBE\n"));
	} else if ((svc_control & NAN_SVC_CONTROL_TYPE_MASK) == NAN_SC_FOLLOWUP) {
		WL_TRACE(("> Service control type: NAN_SC_FOLLOWUP\n"));
	}
	offset = sizeof(*nan_svc_desc_attr);
	if (offset > len) {
		WL_ERR(("Invalid event buffer len\n"));
		ret = BCME_BUFTOOSHORT;
		goto fail;
	}
	p_attr += offset;
	len -= offset;

	/*
	 * optional fields:
	 * must be in order following by service descriptor attribute format
	 */

	/* binding bitmap */
	if (svc_control & NAN_SC_BINDING_BITMAP_PRESENT) {
		uint16 bitmap = 0;
		WL_TRACE(("> svc_control: binding bitmap present\n"));

		/* Copy binding bitmap */
		ret = memcpy_s(&bitmap, sizeof(bitmap),
			p_attr, NAN_BINDING_BITMAP_LEN);
		if (ret != BCME_OK) {
			WL_ERR(("Failed to copy bit map\n"));
			return ret;
		}
		WL_TRACE(("> sc binding bitmap: 0x%04x\n", bitmap));

		if (NAN_BINDING_BITMAP_LEN > len) {
			WL_ERR(("Invalid event buffer len\n"));
			ret = BCME_BUFTOOSHORT;
			goto fail;
		}
		p_attr += NAN_BINDING_BITMAP_LEN;
		len -= NAN_BINDING_BITMAP_LEN;
	}

	/* matching filter */
	if (svc_control & NAN_SC_MATCHING_FILTER_PRESENT) {
		WL_TRACE(("> svc_control: matching filter present\n"));

		tlv_data->tx_match_filter.dlen = *p_attr++;
		WL_TRACE(("> matching filter len: 0x%02x\n",
				tlv_data->tx_match_filter.dlen));

		if (!tlv_data->tx_match_filter.dlen ||
				tlv_data->tx_match_filter.dlen > MAX_MATCH_FILTER_LEN) {
			tlv_data->tx_match_filter.dlen = 0;
			WL_ERR(("tx match filter length is invalid\n"));
			ret = -EINVAL;
			goto fail;
		}
		tlv_data->tx_match_filter.data =
			MALLOCZ(osh, tlv_data->tx_match_filter.dlen);
		if (!tlv_data->tx_match_filter.data) {
			WL_ERR(("%s: memory allocation failed\n", __FUNCTION__));
			tlv_data->tx_match_filter.dlen = 0;
			ret = -ENOMEM;
			goto fail;
		}
		ret = memcpy_s(tlv_data->tx_match_filter.data, tlv_data->tx_match_filter.dlen,
				p_attr, tlv_data->tx_match_filter.dlen);
		if (ret != BCME_OK) {
			WL_ERR(("Failed to copy tx match filter data\n"));
			goto fail;
		}
		/* advance read pointer */
		offset = tlv_data->tx_match_filter.dlen;
		if (offset > len) {
			WL_ERR(("Invalid event buffer\n"));
			ret = BCME_BUFTOOSHORT;
			goto fail;
		}
		p_attr += offset;
		len -= offset;
	}

	/* service response filter */
	if (svc_control & NAN_SC_SR_FILTER_PRESENT) {
		WL_TRACE(("> svc_control: service response filter present\n"));

		tlv_data->rx_match_filter.dlen = *p_attr++;
		WL_TRACE(("> sr match filter len: 0x%02x\n",
				tlv_data->rx_match_filter.dlen));

		if (!tlv_data->rx_match_filter.dlen ||
				tlv_data->rx_match_filter.dlen > MAX_MATCH_FILTER_LEN) {
			tlv_data->rx_match_filter.dlen = 0;
			WL_ERR(("%s: sr matching filter length is invalid\n",
					__FUNCTION__));
			ret = BCME_BADLEN;
			goto fail;
		}
		tlv_data->rx_match_filter.data =
			MALLOCZ(osh, tlv_data->rx_match_filter.dlen);
		if (!tlv_data->rx_match_filter.data) {
			WL_ERR(("%s: memory allocation failed\n", __FUNCTION__));
			tlv_data->rx_match_filter.dlen = 0;
			ret = BCME_NOMEM;
			goto fail;
		}

		ret = memcpy_s(tlv_data->rx_match_filter.data, tlv_data->rx_match_filter.dlen,
				p_attr, tlv_data->rx_match_filter.dlen);
		if (ret != BCME_OK) {
			WL_ERR(("Failed to copy rx match filter data\n"));
			goto fail;
		}

		/* advance read pointer */
		offset = tlv_data->rx_match_filter.dlen;
		if (offset > len) {
			WL_ERR(("Invalid event buffer len\n"));
			ret = BCME_BUFTOOSHORT;
			goto fail;
		}
		p_attr += offset;
		len -= offset;
	}

	/* service specific info */
	if (svc_control & NAN_SC_SVC_INFO_PRESENT) {
		WL_TRACE(("> svc_control: svc specific info present\n"));

		tlv_data->svc_info.dlen = *p_attr++;
		WL_TRACE(("> svc info len: 0x%02x\n", tlv_data->svc_info.dlen));

		if (!tlv_data->svc_info.dlen ||
				tlv_data->svc_info.dlen > NAN_MAX_SERVICE_SPECIFIC_INFO_LEN) {
			/* must be able to handle null msg which is not error */
			tlv_data->svc_info.dlen = 0;
			WL_ERR(("sde data length is invalid\n"));
			ret = BCME_BADLEN;
			goto fail;
		}

		if (tlv_data->svc_info.dlen > 0) {
			tlv_data->svc_info.data =
				MALLOCZ(osh, tlv_data->svc_info.dlen);
			if (!tlv_data->svc_info.data) {
				WL_ERR(("%s: memory allocation failed\n", __FUNCTION__));
				tlv_data->svc_info.dlen = 0;
				ret = BCME_NOMEM;
				goto fail;
			}
			ret = memcpy_s(tlv_data->svc_info.data, tlv_data->svc_info.dlen,
					p_attr, tlv_data->svc_info.dlen);
			if (ret != BCME_OK) {
				WL_ERR(("Failed to copy svc info\n"));
				goto fail;
			}

			/* advance read pointer */
			offset = tlv_data->svc_info.dlen;
			if (offset > len) {
				WL_ERR(("Invalid event buffer len\n"));
				ret = BCME_BUFTOOSHORT;
				goto fail;
			}
			p_attr += offset;
			len -= offset;
		} else {
			/* must be able to handle null msg which is not error */
			tlv_data->svc_info.dlen = 0;
			WL_TRACE(("%s: svc info length is zero, null info data\n",
					__FUNCTION__));
		}
	}

	/*
	 * discovery range limited:
	 * If set to 1, the pub/sub msg is limited in range to close proximity.
	 * If set to 0, the pub/sub msg is not limited in range.
	 * Valid only when the message is either of a publish or a sub.
	 */
	if (svc_control & NAN_SC_RANGE_LIMITED) {
		if (((svc_control & NAN_SVC_CONTROL_TYPE_MASK) == NAN_SC_PUBLISH) ||
				((svc_control & NAN_SVC_CONTROL_TYPE_MASK) == NAN_SC_SUBSCRIBE)) {
			WL_TRACE(("> svc_control: range limited present\n"));
		} else {
			WL_TRACE(("range limited is only valid on pub or sub\n"));
		}

		/* TODO: send up */

		/* advance read pointer */
		p_attr++;
	}
	return ret;
fail:
	if (tlv_data->tx_match_filter.data) {
		MFREE(osh, tlv_data->tx_match_filter.data,
				tlv_data->tx_match_filter.dlen);
		tlv_data->tx_match_filter.data = NULL;
	}
	if (tlv_data->rx_match_filter.data) {
		MFREE(osh, tlv_data->rx_match_filter.data,
				tlv_data->rx_match_filter.dlen);
		tlv_data->rx_match_filter.data = NULL;
	}
	if (tlv_data->svc_info.data) {
		MFREE(osh, tlv_data->svc_info.data,
				tlv_data->svc_info.dlen);
		tlv_data->svc_info.data = NULL;
	}

	WL_DBG(("Parse SDA event data, status = %d\n", ret));
	return ret;
}

static s32
wl_cfgnan_parse_sd_attr_data(osl_t *osh, uint16 len, const uint8 *data,
	nan_event_data_t *tlv_data, uint16 type) {
	const uint8 *p_attr = data;
	uint16 offset = 0;
	s32 ret = BCME_OK;
	const wl_nan_event_disc_result_t *ev_disc = NULL;
	const wl_nan_event_replied_t *ev_replied = NULL;
	const wl_nan_ev_receive_t *ev_fup = NULL;

	/*
	 * Mapping wifi_nan_svc_descriptor_attr_t, and svc controls are optional.
	 */
	if (type == WL_NAN_XTLV_SD_DISC_RESULTS) {
		u8 iter;
		ev_disc = (const wl_nan_event_disc_result_t *)p_attr;

		WL_DBG((">> WL_NAN_XTLV_RESULTS: Discovery result\n"));

		tlv_data->pub_id = (wl_nan_instance_id_t)ev_disc->pub_id;
		tlv_data->sub_id = (wl_nan_instance_id_t)ev_disc->sub_id;
		tlv_data->publish_rssi = ev_disc->publish_rssi;
		ret = memcpy_s(&tlv_data->remote_nmi, ETHER_ADDR_LEN,
				&ev_disc->pub_mac, ETHER_ADDR_LEN);
		if (ret != BCME_OK) {
			WL_ERR(("Failed to copy remote nmi\n"));
			goto fail;
		}

		WL_TRACE(("publish id: %d\n", ev_disc->pub_id));
		WL_TRACE(("subscribe d: %d\n", ev_disc->sub_id));
		WL_TRACE(("publish mac addr: " MACDBG "\n",
				MAC2STRDBG(ev_disc->pub_mac.octet)));
		WL_TRACE(("publish rssi: %d\n", (int8)ev_disc->publish_rssi));
		WL_TRACE(("attribute no: %d\n", ev_disc->attr_num));
		WL_TRACE(("attribute len: %d\n", (uint16)ev_disc->attr_list_len));

		/* advance to the service descricptor */
		offset = OFFSETOF(wl_nan_event_disc_result_t, attr_list[0]);
		if (offset > len) {
			WL_ERR(("Invalid event buffer len\n"));
			ret = BCME_BUFTOOSHORT;
			goto fail;
		}
		p_attr += offset;
		len -= offset;

		iter = ev_disc->attr_num;
		while (iter) {
			if ((uint8)*p_attr == NAN_ATTR_SVC_DESCRIPTOR) {
				WL_TRACE(("> attr id: 0x%02x\n", (uint8)*p_attr));
				ret = wl_cfgnan_parse_sda_data(osh, p_attr, len, tlv_data);
				if (unlikely(ret)) {
					WL_ERR(("wl_cfgnan_parse_sda_data failed,"
							"error = %d \n", ret));
					goto fail;
				}
			}

			if ((uint8)*p_attr == NAN_ATTR_SVC_DESC_EXTENSION) {
				WL_TRACE(("> attr id: 0x%02x\n", (uint8)*p_attr));
				ret = wl_cfgnan_parse_sdea_data(osh, p_attr, len, tlv_data);
				if (unlikely(ret)) {
					WL_ERR(("wl_cfgnan_parse_sdea_data failed,"
							"error = %d \n", ret));
					goto fail;
				}
			}
			offset = (sizeof(*p_attr) +
					sizeof(ev_disc->attr_list_len) +
					(p_attr[1] | (p_attr[2] << 8)));
			if (offset > len) {
				WL_ERR(("Invalid event buffer len\n"));
				ret = BCME_BUFTOOSHORT;
				goto fail;
			}
			p_attr += offset;
			len -= offset;
			iter--;
		}
	} else if (type == WL_NAN_XTLV_SD_FUP_RECEIVED) {
		uint8 iter;
		ev_fup = (const wl_nan_ev_receive_t *)p_attr;

		WL_TRACE((">> WL_NAN_XTLV_SD_FUP_RECEIVED: Transmit follow-up\n"));

		tlv_data->local_inst_id = (wl_nan_instance_id_t)ev_fup->local_id;
		tlv_data->requestor_id = (wl_nan_instance_id_t)ev_fup->remote_id;
		tlv_data->fup_rssi = ev_fup->fup_rssi;
		ret = memcpy_s(&tlv_data->remote_nmi, ETHER_ADDR_LEN,
				&ev_fup->remote_addr, ETHER_ADDR_LEN);
		if (ret != BCME_OK) {
			WL_ERR(("Failed to copy remote nmi\n"));
			goto fail;
		}

		WL_TRACE(("local id: %d\n", ev_fup->local_id));
		WL_TRACE(("remote id: %d\n", ev_fup->remote_id));
		WL_TRACE(("peer mac addr: " MACDBG "\n",
				MAC2STRDBG(ev_fup->remote_addr.octet)));
		WL_TRACE(("peer rssi: %d\n", (int8)ev_fup->fup_rssi));
		WL_TRACE(("attribute no: %d\n", ev_fup->attr_num));
		WL_TRACE(("attribute len: %d\n", ev_fup->attr_list_len));

		/* advance to the service descriptor which is attr_list[0] */
		offset = OFFSETOF(wl_nan_ev_receive_t, attr_list[0]);
		if (offset > len) {
			WL_ERR(("Invalid event buffer len\n"));
			ret = BCME_BUFTOOSHORT;
			goto fail;
		}
		p_attr += offset;
		len -= offset;

		iter = ev_fup->attr_num;
		while (iter) {
			if ((uint8)*p_attr == NAN_ATTR_SVC_DESCRIPTOR) {
				WL_TRACE(("> attr id: 0x%02x\n", (uint8)*p_attr));
				ret = wl_cfgnan_parse_sda_data(osh, p_attr, len, tlv_data);
				if (unlikely(ret)) {
					WL_ERR(("wl_cfgnan_parse_sda_data failed,"
							"error = %d \n", ret));
					goto fail;
				}
			}

			if ((uint8)*p_attr == NAN_ATTR_SVC_DESC_EXTENSION) {
				WL_TRACE(("> attr id: 0x%02x\n", (uint8)*p_attr));
				ret = wl_cfgnan_parse_sdea_data(osh, p_attr, len, tlv_data);
				if (unlikely(ret)) {
					WL_ERR(("wl_cfgnan_parse_sdea_data failed,"
							"error = %d \n", ret));
					goto fail;
				}
			}
			offset = (sizeof(*p_attr) +
					sizeof(ev_fup->attr_list_len) +
					(p_attr[1] | (p_attr[2] << 8)));
			if (offset > len) {
				WL_ERR(("Invalid event buffer len\n"));
				ret = BCME_BUFTOOSHORT;
				goto fail;
			}
			p_attr += offset;
			len -= offset;
			iter--;
		}
	} else if (type == WL_NAN_XTLV_SD_SDF_RX) {
		/*
		 * SDF followed by nan2_pub_act_frame_t and wifi_nan_svc_descriptor_attr_t,
		 * and svc controls are optional.
		 */
		const nan2_pub_act_frame_t *nan_pub_af =
			(const nan2_pub_act_frame_t *)p_attr;

		WL_TRACE((">> WL_NAN_XTLV_SD_SDF_RX\n"));

		/* nan2_pub_act_frame_t */
		WL_TRACE(("pub category: 0x%02x\n", nan_pub_af->category_id));
		WL_TRACE(("pub action: 0x%02x\n", nan_pub_af->action_field));
		WL_TRACE(("nan oui: %2x-%2x-%2x\n",
				nan_pub_af->oui[0], nan_pub_af->oui[1], nan_pub_af->oui[2]));
		WL_TRACE(("oui type: 0x%02x\n", nan_pub_af->oui_type));
		WL_TRACE(("oui subtype: 0x%02x\n", nan_pub_af->oui_sub_type));

		offset = sizeof(*nan_pub_af);
		if (offset > len) {
			WL_ERR(("Invalid event buffer len\n"));
			ret = BCME_BUFTOOSHORT;
			goto fail;
		}
		p_attr += offset;
		len -= offset;
	} else if (type == WL_NAN_XTLV_SD_REPLIED) {
		ev_replied = (const wl_nan_event_replied_t *)p_attr;

		WL_TRACE((">> WL_NAN_XTLV_SD_REPLIED: Replied Event\n"));

		tlv_data->pub_id = (wl_nan_instance_id_t)ev_replied->pub_id;
		tlv_data->sub_id = (wl_nan_instance_id_t)ev_replied->sub_id;
		tlv_data->sub_rssi = ev_replied->sub_rssi;
		ret = memcpy_s(&tlv_data->remote_nmi, ETHER_ADDR_LEN,
				&ev_replied->sub_mac, ETHER_ADDR_LEN);
		if (ret != BCME_OK) {
			WL_ERR(("Failed to copy remote nmi\n"));
			goto fail;
		}

		WL_TRACE(("publish id: %d\n", ev_replied->pub_id));
		WL_TRACE(("subscribe d: %d\n", ev_replied->sub_id));
		WL_TRACE(("Subscriber mac addr: " MACDBG "\n",
				MAC2STRDBG(ev_replied->sub_mac.octet)));
		WL_TRACE(("subscribe rssi: %d\n", (int8)ev_replied->sub_rssi));
		WL_TRACE(("attribute no: %d\n", ev_replied->attr_num));
		WL_TRACE(("attribute len: %d\n", (uint16)ev_replied->attr_list_len));

		/* advance to the service descriptor which is attr_list[0] */
		offset = OFFSETOF(wl_nan_event_replied_t, attr_list[0]);
		if (offset > len) {
			WL_ERR(("Invalid event buffer len\n"));
			ret = BCME_BUFTOOSHORT;
			goto fail;
		}
		p_attr += offset;
		len -= offset;
		ret = wl_cfgnan_parse_sda_data(osh, p_attr, len, tlv_data);
		if (unlikely(ret)) {
			WL_ERR(("wl_cfgnan_parse_sdea_data failed,"
				"error = %d \n", ret));
		}
	}

fail:
	return ret;
}

/* Based on each case of tlv type id, fill into tlv data */
int
wl_cfgnan_set_vars_cbfn(void *ctx, const uint8 *data, uint16 type, uint16 len)
{
	nan_parse_event_ctx_t *ctx_tlv_data = ((nan_parse_event_ctx_t *)(ctx));
	nan_event_data_t *tlv_data = ((nan_event_data_t *)(ctx_tlv_data->nan_evt_data));
	int ret = BCME_OK;

	NAN_DBG_ENTER();
	if (!data || !len) {
		WL_ERR(("data length is invalid\n"));
		ret = BCME_ERROR;
		goto fail;
	}

	switch (type) {
	/*
	 * Need to parse service descript attributes including service control,
	 * when Follow up or Discovery result come
	 */
	case WL_NAN_XTLV_SD_FUP_RECEIVED:
	case WL_NAN_XTLV_SD_DISC_RESULTS: {
		ret = wl_cfgnan_parse_sd_attr_data(ctx_tlv_data->cfg->osh,
			len, data, tlv_data, type);
		break;
	}
	case WL_NAN_XTLV_SD_SVC_INFO: {
		tlv_data->svc_info.data =
			MALLOCZ(ctx_tlv_data->cfg->osh, len);
		if (!tlv_data->svc_info.data) {
			WL_ERR(("%s: memory allocation failed\n", __FUNCTION__));
			tlv_data->svc_info.dlen = 0;
			ret = BCME_NOMEM;
			goto fail;
		}
		tlv_data->svc_info.dlen = len;
		ret = memcpy_s(tlv_data->svc_info.data, tlv_data->svc_info.dlen,
				data, tlv_data->svc_info.dlen);
		if (ret != BCME_OK) {
			WL_ERR(("Failed to copy svc info data\n"));
			goto fail;
		}
		break;
	}
	default:
		WL_ERR(("Not available for tlv type = 0x%x\n", type));
		ret = BCME_ERROR;
		break;
	}
fail:
	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfg_nan_check_cmd_len(uint16 nan_iov_len, uint16 data_size,
		uint16 *subcmd_len)
{
	s32 ret = BCME_OK;

	if (subcmd_len != NULL) {
		*subcmd_len = OFFSETOF(bcm_iov_batch_subcmd_t, data) +
				ALIGN_SIZE(data_size, 4);
		if (*subcmd_len > nan_iov_len) {
			WL_ERR(("%s: Buf short, requested:%d, available:%d\n",
					__FUNCTION__, *subcmd_len, nan_iov_len));
			ret = BCME_NOMEM;
		}
	} else {
		WL_ERR(("Invalid subcmd_len\n"));
		ret = BCME_ERROR;
	}
	return ret;
}

int
wl_cfgnan_config_eventmask(struct net_device *ndev, struct bcm_cfg80211 *cfg,
	uint8 event_ind_flag, bool disable_events)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	s32 ret = BCME_OK;
	uint16 nan_buf_size = NAN_IOCTL_BUF_SIZE;
	uint16 subcmd_len;
	uint32 status;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	bcm_iov_batch_subcmd_t *sub_cmd_resp = NULL;
	uint8 event_mask[WL_NAN_EVMASK_EXTN_LEN];
	wl_nan_evmask_extn_t *evmask;
	uint16 evmask_cmd_len;
	uint8 resp_buf[NAN_IOCTL_BUF_SIZE];

	NAN_DBG_ENTER();

	/* same src and dest len here */
	(void)memset_s(event_mask, WL_NAN_EVMASK_EXTN_VER, 0, WL_NAN_EVMASK_EXTN_VER);
	evmask_cmd_len = OFFSETOF(wl_nan_evmask_extn_t, evmask) +
		WL_NAN_EVMASK_EXTN_LEN;
	ret = wl_add_remove_eventmsg(ndev, WLC_E_NAN, true);
	if (unlikely(ret)) {
		WL_ERR((" nan event enable failed, error = %d \n", ret));
		goto fail;
	}

	nan_buf = MALLOCZ(cfg->osh, nan_buf_size);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_buf_size -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);
	sub_cmd = (bcm_iov_batch_subcmd_t*)(uint8 *)(&nan_buf->cmds[0]);

	ret = wl_cfg_nan_check_cmd_len(nan_buf_size,
			evmask_cmd_len, &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		goto fail;
	}

	sub_cmd->id = htod16(WL_NAN_CMD_CFG_EVENT_MASK);
	sub_cmd->len = sizeof(sub_cmd->u.options) + evmask_cmd_len;
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
	evmask = (wl_nan_evmask_extn_t *)sub_cmd->data;
	evmask->ver = WL_NAN_EVMASK_EXTN_VER;
	evmask->len = WL_NAN_EVMASK_EXTN_LEN;
	nan_buf_size -= subcmd_len;
	nan_buf->count = 1;

	if (disable_events) {
		WL_DBG(("Disabling all nan events..except stop event\n"));
		setbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_STOP));
	} else {
		/*
		 * Android framework event mask configuration.
		 */
		nan_buf->is_set = false;
		memset(resp_buf, 0, sizeof(resp_buf));
		ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, nan_buf_size, &status,
				(void*)resp_buf, NAN_IOCTL_BUF_SIZE);
		if (unlikely(ret) || unlikely(status)) {
			WL_ERR(("get nan event mask failed ret %d status %d \n",
				ret, status));
			goto fail;
		}
		sub_cmd_resp = &((bcm_iov_batch_buf_t *)(resp_buf))->cmds[0];
		evmask = (wl_nan_evmask_extn_t *)sub_cmd_resp->data;

		/* check the response buff */
		/* same src and dest len here */
		(void)memcpy_s(&event_mask, WL_NAN_EVMASK_EXTN_LEN,
				(uint8*)&evmask->evmask, WL_NAN_EVMASK_EXTN_LEN);

		if (event_ind_flag) {
			if (CHECK_BIT(event_ind_flag, WL_NAN_EVENT_DIC_MAC_ADDR_BIT)) {
				WL_DBG(("Need to add disc mac addr change event\n"));
			}
			/* BIT2 - Disable nan cluster join indication (OTA). */
			if (CHECK_BIT(event_ind_flag, WL_NAN_EVENT_JOIN_EVENT)) {
				clrbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_MERGE));
			}
		}

		setbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_DISCOVERY_RESULT));
		setbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_RECEIVE));
		setbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_TERMINATED));
		setbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_STOP));
		setbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_TXS));
		setbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_PEER_DATAPATH_IND));
		setbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_DATAPATH_ESTB));
		setbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_DATAPATH_END));
		setbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_RNG_REQ_IND));
		setbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_RNG_TERM_IND));
		setbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_DISC_CACHE_TIMEOUT));
		/* Disable below events by default */
		clrbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_PEER_SCHED_UPD_NOTIF));
		clrbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_RNG_RPT_IND));
		clrbit(event_mask, NAN_EVENT_MAP(WL_NAN_EVENT_DW_END));
	}

	nan_buf->is_set = true;
	evmask = (wl_nan_evmask_extn_t *)sub_cmd->data;
	/* same src and dest len here */
	(void)memcpy_s((uint8*)&evmask->evmask, WL_NAN_EVMASK_EXTN_LEN,
		&event_mask, WL_NAN_EVMASK_EXTN_LEN);

	nan_buf_size = (NAN_IOCTL_BUF_SIZE - nan_buf_size);
	ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, nan_buf_size, &status,
			(void*)resp_buf, NAN_IOCTL_BUF_SIZE);
	if (unlikely(ret) || unlikely(status)) {
		WL_ERR(("set nan event mask failed ret %d status %d \n", ret, status));
		goto fail;
	}
	WL_DBG(("set nan event mask successfull\n"));

fail:
	if (nan_buf) {
		MFREE(cfg->osh, nan_buf, NAN_IOCTL_BUF_SIZE);
	}
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_nan_avail(struct net_device *ndev,
		struct bcm_cfg80211 *cfg, nan_avail_cmd_data *cmd_data, uint8 avail_type)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	s32 ret = BCME_OK;
	uint16 nan_buf_size = NAN_IOCTL_BUF_SIZE;
	uint16 subcmd_len;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	wl_nan_iov_t *nan_iov_data = NULL;
	wl_avail_t *avail = NULL;
	wl_avail_entry_t *entry;	/* used for filling entry structure */
	uint8 *p;	/* tracking pointer */
	uint8 i;
	u32 status;
	int c;
	char ndc_id[ETHER_ADDR_LEN] = { 0x50, 0x6f, 0x9a, 0x01, 0x0, 0x0 };
	dhd_pub_t *dhdp = wl_cfg80211_get_dhdp(ndev);
	char *a = WL_AVAIL_BIT_MAP;
	uint8 resp_buf[NAN_IOCTL_BUF_SIZE];

	NAN_DBG_ENTER();

	/* Do not disturb avail if dam is supported */
	if (FW_SUPPORTED(dhdp, autodam)) {
		WL_DBG(("DAM is supported, avail modification not allowed\n"));
		return ret;
	}

	if (avail_type < WL_AVAIL_LOCAL || avail_type > WL_AVAIL_TYPE_MAX) {
		WL_ERR(("Invalid availability type\n"));
		ret = BCME_USAGE_ERROR;
		goto fail;
	}

	nan_buf = MALLOCZ(cfg->osh, nan_buf_size);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = MALLOCZ(cfg->osh, sizeof(*nan_iov_data));
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data->nan_iov_len = NAN_IOCTL_BUF_SIZE;
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);
	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(*avail), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		goto fail;
	}
	avail = (wl_avail_t *)sub_cmd->data;

	/* populate wl_avail_type */
	avail->flags = avail_type;
	if (avail_type == WL_AVAIL_RANGING) {
		ret = memcpy_s(&avail->addr, ETHER_ADDR_LEN,
			&cmd_data->peer_nmi, ETHER_ADDR_LEN);
		if (ret != BCME_OK) {
			WL_ERR(("Failed to copy peer nmi\n"));
			goto fail;
		}
	}

	sub_cmd->len = sizeof(sub_cmd->u.options) + subcmd_len;
	sub_cmd->id = htod16(WL_NAN_CMD_CFG_AVAIL);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);

	nan_buf->is_set = false;
	nan_buf->count++;
	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_buf_size = (NAN_IOCTL_BUF_SIZE - nan_iov_data->nan_iov_len);

	WL_TRACE(("Read wl nan avail status\n"));

	memset_s(resp_buf, sizeof(resp_buf), 0, sizeof(resp_buf));
	ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, nan_buf_size, &status,
			(void*)resp_buf, NAN_IOCTL_BUF_SIZE);
	if (unlikely(ret)) {
		WL_ERR(("\n Get nan avail failed ret %d, status %d \n", ret, status));
		goto fail;
	}

	if (status == BCME_NOTFOUND) {
		nan_buf->count = 0;
		nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
		nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

		sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);

		avail = (wl_avail_t *)sub_cmd->data;
		p = avail->entry;

		/* populate wl_avail fields */
		avail->length = OFFSETOF(wl_avail_t, entry);
		avail->flags = avail_type;
		avail->num_entries = 0;
		avail->id = 0;
		entry = (wl_avail_entry_t*)p;
		entry->flags = WL_AVAIL_ENTRY_COM;

		/* set default values for optional parameters */
		entry->start_offset = 0;
		entry->u.band = 0;

		if (cmd_data->avail_period) {
			entry->period = cmd_data->avail_period;
		} else {
			entry->period = WL_AVAIL_PERIOD_1024;
		}

		if (cmd_data->duration != NAN_BAND_INVALID) {
			entry->flags |= (3 << WL_AVAIL_ENTRY_USAGE_SHIFT) |
				(cmd_data->duration << WL_AVAIL_ENTRY_BIT_DUR_SHIFT);
		} else {
			entry->flags |= (3 << WL_AVAIL_ENTRY_USAGE_SHIFT) |
				(WL_AVAIL_BIT_DUR_16 << WL_AVAIL_ENTRY_BIT_DUR_SHIFT);
		}
		entry->bitmap_len = 0;

		if (avail_type == WL_AVAIL_LOCAL) {
			entry->flags |= 1 << WL_AVAIL_ENTRY_CHAN_SHIFT;
			/* Check for 5g support, based on that choose 5g channel */
			if (cfg->support_5g) {
				entry->u.channel_info =
					htod32(wf_channel2chspec(WL_AVAIL_CHANNEL_5G,
						WL_AVAIL_BANDWIDTH_5G));
			} else {
				entry->u.channel_info =
					htod32(wf_channel2chspec(WL_AVAIL_CHANNEL_2G,
						WL_AVAIL_BANDWIDTH_2G));
			}
			entry->flags = htod16(entry->flags);
		}

		if (cfg->support_5g) {
			a = WL_5G_AVAIL_BIT_MAP;
		}

		/* point to bitmap value for processing */
		if (cmd_data->bmap) {
			for (c = (WL_NAN_EVENT_CLEAR_BIT-1); c >= 0; c--) {
				i = cmd_data->bmap >> c;
				if (i & 1) {
					setbit(entry->bitmap, (WL_NAN_EVENT_CLEAR_BIT-c-1));
				}
			}
		} else {
			for (i = 0; i < strlen(WL_AVAIL_BIT_MAP); i++) {
				if (*a == '1') {
					setbit(entry->bitmap, i);
				}
				a++;
			}
		}

		/* account for partially filled most significant byte */
		entry->bitmap_len = ((WL_NAN_EVENT_CLEAR_BIT) + NBBY - 1) / NBBY;
		if (avail_type == WL_AVAIL_NDC) {
			ret = memcpy_s(&avail->addr, ETHER_ADDR_LEN,
					ndc_id, ETHER_ADDR_LEN);
			if (ret != BCME_OK) {
				WL_ERR(("Failed to copy ndc id\n"));
				goto fail;
			}
		} else if (avail_type == WL_AVAIL_RANGING) {
			ret = memcpy_s(&avail->addr, ETHER_ADDR_LEN,
					&cmd_data->peer_nmi, ETHER_ADDR_LEN);
			if (ret != BCME_OK) {
				WL_ERR(("Failed to copy peer nmi\n"));
				goto fail;
			}
		}
		/* account for partially filled most significant byte */

		/* update wl_avail and populate wl_avail_entry */
		entry->length = OFFSETOF(wl_avail_entry_t, bitmap) + entry->bitmap_len;
		avail->num_entries++;
		avail->length += entry->length;
		/* advance pointer for next entry */
		p += entry->length;

		/* convert to dongle endianness */
		entry->length = htod16(entry->length);
		entry->start_offset = htod16(entry->start_offset);
		entry->u.channel_info = htod32(entry->u.channel_info);
		entry->flags = htod16(entry->flags);
		/* update avail_len only if
		 * there are avail entries
		 */
		if (avail->num_entries) {
			nan_iov_data->nan_iov_len -= avail->length;
			avail->length = htod16(avail->length);
			avail->flags = htod16(avail->flags);
		}
		avail->length = htod16(avail->length);

		sub_cmd->id = htod16(WL_NAN_CMD_CFG_AVAIL);
		sub_cmd->len = sizeof(sub_cmd->u.options) + avail->length;
		sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);

		nan_buf->is_set = true;
		nan_buf->count++;

		/* Reduce the iov_len size by subcmd_len */
		nan_iov_data->nan_iov_len -= subcmd_len;
		nan_buf_size = (NAN_IOCTL_BUF_SIZE - nan_iov_data->nan_iov_len);

		ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, nan_buf_size, &status,
				(void*)resp_buf, NAN_IOCTL_BUF_SIZE);
		if (unlikely(ret) || unlikely(status)) {
			WL_ERR(("\n set nan avail failed ret %d status %d \n", ret, status));
			ret = status;
			goto fail;
		}
	} else if (status == BCME_OK) {
		WL_DBG(("Avail type [%d] found to be configured\n", avail_type));
	} else {
		WL_ERR(("set nan avail failed ret %d status %d \n", ret, status));
	}

fail:
	if (nan_buf) {
		MFREE(cfg->osh, nan_buf, NAN_IOCTL_BUF_SIZE);
	}
	if (nan_iov_data) {
		MFREE(cfg->osh, nan_iov_data, sizeof(*nan_iov_data));
	}

	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_config_control_flag(struct net_device *ndev, struct bcm_cfg80211 *cfg,
		uint32 flag, uint32 *status, bool set)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	s32 ret = BCME_OK;
	uint16 nan_iov_start, nan_iov_end;
	uint16 nan_buf_size = NAN_IOCTL_BUF_SIZE;
	uint16 subcmd_len;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	bcm_iov_batch_subcmd_t *sub_cmd_resp = NULL;
	wl_nan_iov_t *nan_iov_data = NULL;
	uint32 cfg_ctrl;
	uint8 resp_buf[NAN_IOCTL_BUF_SIZE];

	NAN_DBG_ENTER();
	WL_INFORM_MEM(("%s: Modifying nan ctrl flag %x val %d",
		__FUNCTION__, flag, set));
	nan_buf = MALLOCZ(cfg->osh, nan_buf_size);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = MALLOCZ(cfg->osh, sizeof(*nan_iov_data));
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data->nan_iov_len = nan_iov_start = NAN_IOCTL_BUF_SIZE;
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);
	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(cfg_ctrl), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		goto fail;
	}

	sub_cmd->id = htod16(WL_NAN_CMD_CFG_NAN_CONFIG);
	sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(cfg_ctrl);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);

	nan_buf->is_set = false;
	nan_buf->count++;

	/* Reduce the iov_len size by subcmd_len */
	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_end = nan_iov_data->nan_iov_len;
	nan_buf_size = (nan_iov_start - nan_iov_end);

	memset_s(resp_buf, sizeof(resp_buf), 0, sizeof(resp_buf));
	ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, nan_buf_size, status,
			(void*)resp_buf, NAN_IOCTL_BUF_SIZE);
	if (unlikely(ret) || unlikely(*status)) {
		WL_ERR(("get nan cfg ctrl failed ret %d status %d \n", ret, *status));
		goto fail;
	}
	sub_cmd_resp = &((bcm_iov_batch_buf_t *)(resp_buf))->cmds[0];

	/* check the response buff */
	cfg_ctrl = (*(uint32 *)&sub_cmd_resp->data[0]);
	if (set) {
		cfg_ctrl |= flag;
	} else {
		cfg_ctrl &= ~flag;
	}
	ret = memcpy_s(sub_cmd->data, sizeof(cfg_ctrl),
			&cfg_ctrl, sizeof(cfg_ctrl));
	if (ret != BCME_OK) {
		WL_ERR(("Failed to copy cfg ctrl\n"));
		goto fail;
	}

	nan_buf->is_set = true;
	ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, nan_buf_size, status,
			(void*)resp_buf, NAN_IOCTL_BUF_SIZE);
	if (unlikely(ret) || unlikely(*status)) {
		WL_ERR(("set nan cfg ctrl failed ret %d status %d \n", ret, *status));
		goto fail;
	}
	WL_DBG(("set nan cfg ctrl successfull\n"));
fail:
	if (nan_buf) {
		MFREE(cfg->osh, nan_buf, NAN_IOCTL_BUF_SIZE);
	}
	if (nan_iov_data) {
		MFREE(cfg->osh, nan_iov_data, sizeof(*nan_iov_data));
	}

	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_get_iovars_status(void *ctx, const uint8 *data, uint16 type, uint16 len)
{
	bcm_iov_batch_buf_t *b_resp = (bcm_iov_batch_buf_t *)ctx;
	uint32 status;
	/* if all tlvs are parsed, we should not be here */
	if (b_resp->count == 0) {
		return BCME_BADLEN;
	}

	/*  cbfn params may be used in f/w */
	if (len < sizeof(status)) {
		return BCME_BUFTOOSHORT;
	}

	/* first 4 bytes consists status */
	if (memcpy_s(&status, sizeof(status),
			data, sizeof(uint32)) != BCME_OK) {
		WL_ERR(("Failed to copy status\n"));
		goto exit;
	}

	status = dtoh32(status);

	/* If status is non zero */
	if (status != BCME_OK) {
		printf("cmd type %d failed, status: %04x\n", type, status);
		goto exit;
	}

	if (b_resp->count > 0) {
		b_resp->count--;
	}

	if (!b_resp->count) {
		status = BCME_IOV_LAST_CMD;
	}
exit:
	return status;
}

static int
wl_cfgnan_execute_ioctl(struct net_device *ndev, struct bcm_cfg80211 *cfg,
	bcm_iov_batch_buf_t *nan_buf, uint16 nan_buf_size, uint32 *status,
	uint8 *resp_buf, uint16 resp_buf_size)
{
	int ret = BCME_OK;
	uint16 tlvs_len;
	int res = BCME_OK;
	bcm_iov_batch_buf_t *p_resp = NULL;
	char *iov = "nan";
	int max_resp_len = WLC_IOCTL_MAXLEN;

	WL_DBG(("Enter:\n"));
	if (nan_buf->is_set) {
		ret = wldev_iovar_setbuf(ndev, "nan", nan_buf, nan_buf_size,
			resp_buf, resp_buf_size, NULL);
		p_resp = (bcm_iov_batch_buf_t *)(resp_buf + strlen(iov) + 1);
	} else {
		ret = wldev_iovar_getbuf(ndev, "nan", nan_buf, nan_buf_size,
			resp_buf, resp_buf_size, NULL);
		p_resp = (bcm_iov_batch_buf_t *)(resp_buf);
	}
	if (unlikely(ret)) {
		WL_ERR((" nan execute ioctl failed, error = %d \n", ret));
		goto fail;
	}

	p_resp->is_set = nan_buf->is_set;
	tlvs_len = max_resp_len - OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

	/* Extract the tlvs and print their resp in cb fn */
	res = bcm_unpack_xtlv_buf((void *)p_resp, (const uint8 *)&p_resp->cmds[0],
		tlvs_len, BCM_IOV_CMD_OPT_ALIGN32, wl_cfgnan_get_iovars_status);

	if (res == BCME_IOV_LAST_CMD) {
		res = BCME_OK;
	}
fail:
	*status = res;
	WL_DBG((" nan ioctl ret %d status %d \n", ret, *status));
	return ret;

}

static int
wl_cfgnan_if_addr_handler(void *p_buf, uint16 *nan_buf_size,
		struct ether_addr *if_addr)
{
	/* nan enable */
	s32 ret = BCME_OK;
	uint16 subcmd_len;

	NAN_DBG_ENTER();

	if (p_buf != NULL) {
		bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);

		ret = wl_cfg_nan_check_cmd_len(*nan_buf_size,
				sizeof(*if_addr), &subcmd_len);
		if (unlikely(ret)) {
			WL_ERR(("nan_sub_cmd check failed\n"));
			goto fail;
		}

		/* Fill the sub_command block */
		sub_cmd->id = htod16(WL_NAN_CMD_CFG_IF_ADDR);
		sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(*if_addr);
		sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
		ret = memcpy_s(sub_cmd->data, sizeof(*if_addr),
				(uint8 *)if_addr, sizeof(*if_addr));
		if (ret != BCME_OK) {
			WL_ERR(("Failed to copy if addr\n"));
			goto fail;
		}

		*nan_buf_size -= subcmd_len;
	} else {
		WL_ERR(("nan_iov_buf is NULL\n"));
		ret = BCME_ERROR;
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_get_ver(struct net_device *ndev, struct bcm_cfg80211 *cfg)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	s32 ret = BCME_OK;
	uint16 nan_buf_size = NAN_IOCTL_BUF_SIZE;
	wl_nan_ver_t *nan_ver = NULL;
	uint16 subcmd_len;
	uint32 status;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	bcm_iov_batch_subcmd_t *sub_cmd_resp = NULL;
	uint8 resp_buf[NAN_IOCTL_BUF_SIZE];

	NAN_DBG_ENTER();
	nan_buf = MALLOCZ(cfg->osh, nan_buf_size);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_buf_size -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);
	sub_cmd = (bcm_iov_batch_subcmd_t*)(uint8 *)(&nan_buf->cmds[0]);

	ret = wl_cfg_nan_check_cmd_len(nan_buf_size,
			sizeof(*nan_ver), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		goto fail;
	}

	nan_ver = (wl_nan_ver_t *)sub_cmd->data;
	sub_cmd->id = htod16(WL_NAN_CMD_GLB_NAN_VER);
	sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(*nan_ver);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
	nan_buf_size -= subcmd_len;
	nan_buf->count = 1;

	nan_buf->is_set = false;
	bzero(resp_buf, sizeof(resp_buf));
	nan_buf_size = NAN_IOCTL_BUF_SIZE - nan_buf_size;

	ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, nan_buf_size, &status,
			(void*)resp_buf, NAN_IOCTL_BUF_SIZE);
	if (unlikely(ret) || unlikely(status)) {
		WL_ERR(("get nan ver failed ret %d status %d \n",
				ret, status));
		goto fail;
	}

	sub_cmd_resp = &((bcm_iov_batch_buf_t *)(resp_buf))->cmds[0];
	nan_ver = ((wl_nan_ver_t *)&sub_cmd_resp->data[0]);
	if (!nan_ver) {
		ret = BCME_NOTFOUND;
		WL_ERR(("nan_ver not found: err = %d\n", ret));
		goto fail;
	}
	cfg->nancfg.version = *nan_ver;
	WL_INFORM_MEM(("Nan Version is %d\n", cfg->nancfg.version));

fail:
	if (nan_buf) {
		MFREE(cfg->osh, nan_buf, NAN_IOCTL_BUF_SIZE);
	}
	NAN_DBG_EXIT();
	return ret;

}

static int
wl_cfgnan_set_if_addr(struct bcm_cfg80211 *cfg)
{
	s32 ret = BCME_OK;
	uint16 nan_buf_size = NAN_IOCTL_BUF_SIZE;
	uint32 status;
	uint8 resp_buf[NAN_IOCTL_BUF_SIZE];
	struct ether_addr if_addr;
	uint8 buf[NAN_IOCTL_BUF_SIZE];
	bcm_iov_batch_buf_t *nan_buf = (bcm_iov_batch_buf_t*)buf;
	bool rand_mac = cfg->nancfg.mac_rand;

	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_buf_size -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);
	if (rand_mac) {
		RANDOM_BYTES(if_addr.octet, 6);
		/* restore mcast and local admin bits to 0 and 1 */
		ETHER_SET_UNICAST(if_addr.octet);
		ETHER_SET_LOCALADDR(if_addr.octet);
	} else {
		/* Use primary MAC with the locally administered bit for the
		 * NAN NMI I/F
		 */
		if (wl_get_vif_macaddr(cfg, WL_IF_TYPE_NAN_NMI,
				if_addr.octet) != BCME_OK) {
			ret = -EINVAL;
			WL_ERR(("Failed to get mac addr for NMI\n"));
			goto fail;
		}
	}
	WL_INFORM_MEM(("%s: NMI " MACDBG "\n",
			__FUNCTION__, MAC2STRDBG(if_addr.octet)));
	ret = wl_cfgnan_if_addr_handler(&nan_buf->cmds[0],
			&nan_buf_size, &if_addr);
	if (unlikely(ret)) {
		WL_ERR(("Nan if addr handler sub_cmd set failed\n"));
		goto fail;
	}
	nan_buf->count++;
	nan_buf->is_set = true;
	nan_buf_size = NAN_IOCTL_BUF_SIZE - nan_buf_size;
	memset_s(resp_buf, sizeof(resp_buf), 0, sizeof(resp_buf));
	ret = wl_cfgnan_execute_ioctl(bcmcfg_to_prmry_ndev(cfg), cfg,
			nan_buf, nan_buf_size, &status,
			(void*)resp_buf, NAN_IOCTL_BUF_SIZE);
	if (unlikely(ret) || unlikely(status)) {
		WL_ERR(("nan if addr handler failed ret %d status %d\n",
				ret, status));
		goto fail;
	}
	ret = memcpy_s(cfg->nan_nmi_mac, ETH_ALEN,
			if_addr.octet, ETH_ALEN);
	if (ret != BCME_OK) {
		WL_ERR(("Failed to copy nmi addr\n"));
		goto fail;
	}
	return ret;
fail:
	if (!rand_mac) {
		wl_release_vif_macaddr(cfg, if_addr.octet, WL_IF_TYPE_NAN_NMI);
	}

	return ret;
}

static int
wl_cfgnan_init_handler(void *p_buf, uint16 *nan_buf_size, bool val)
{
	/* nan enable */
	s32 ret = BCME_OK;
	uint16 subcmd_len;

	NAN_DBG_ENTER();

	if (p_buf != NULL) {
		bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);

		ret = wl_cfg_nan_check_cmd_len(*nan_buf_size,
				sizeof(val), &subcmd_len);
		if (unlikely(ret)) {
			WL_ERR(("nan_sub_cmd check failed\n"));
			goto fail;
		}

		/* Fill the sub_command block */
		sub_cmd->id = htod16(WL_NAN_CMD_CFG_NAN_INIT);
		sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(uint8);
		sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
		ret = memcpy_s(sub_cmd->data, sizeof(uint8),
				(uint8*)&val, sizeof(uint8));
		if (ret != BCME_OK) {
			WL_ERR(("Failed to copy init value\n"));
			goto fail;
		}

		*nan_buf_size -= subcmd_len;
	} else {
		WL_ERR(("nan_iov_buf is NULL\n"));
		ret = BCME_ERROR;
		goto fail;
	}

fail:
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_enable_handler(wl_nan_iov_t *nan_iov_data, bool val)
{
	/* nan enable */
	s32 ret = BCME_OK;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	uint16 subcmd_len;

	NAN_DBG_ENTER();

	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(val), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		return ret;
	}

	/* Fill the sub_command block */
	sub_cmd->id = htod16(WL_NAN_CMD_CFG_NAN_ENAB);
	sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(uint8);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
	ret = memcpy_s(sub_cmd->data, sizeof(uint8),
			(uint8*)&val, sizeof(uint8));
	if (ret != BCME_OK) {
		WL_ERR(("Failed to copy enab value\n"));
		return ret;
	}

	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_data->nan_iov_buf += subcmd_len;
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_warmup_time_handler(nan_config_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data)
{
	/* wl nan warm_up_time */
	s32 ret = BCME_OK;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	wl_nan_warmup_time_ticks_t *wup_ticks = NULL;
	uint16 subcmd_len;
	NAN_DBG_ENTER();

	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);
	wup_ticks = (wl_nan_warmup_time_ticks_t *)sub_cmd->data;

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(*wup_ticks), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		return ret;
	}
	/* Fill the sub_command block */
	sub_cmd->id = htod16(WL_NAN_CMD_CFG_WARMUP_TIME);
	sub_cmd->len = sizeof(sub_cmd->u.options) +
		sizeof(*wup_ticks);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
	*wup_ticks = cmd_data->warmup_time;

	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_data->nan_iov_buf += subcmd_len;

	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_election_metric(nan_config_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data, uint32 nan_attr_mask)
{
	s32 ret = BCME_OK;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	wl_nan_election_metric_config_t *metrics = NULL;
	uint16 subcmd_len;
	NAN_DBG_ENTER();

	sub_cmd =
		(bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);
	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(*metrics), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		goto fail;
	}

	metrics = (wl_nan_election_metric_config_t *)sub_cmd->data;

	if (nan_attr_mask & NAN_ATTR_RAND_FACTOR_CONFIG) {
		metrics->random_factor = (uint8)cmd_data->metrics.random_factor;
	}

	if ((!cmd_data->metrics.master_pref) ||
		(cmd_data->metrics.master_pref > NAN_MAXIMUM_MASTER_PREFERENCE)) {
		WL_TRACE(("Master Pref is 0 or greater than 254, hence sending random value\n"));
		/* Master pref for mobile devices can be from 1 - 127 as per Spec AppendixC */
		metrics->master_pref = (RANDOM32()%(NAN_MAXIMUM_MASTER_PREFERENCE/2)) + 1;
	} else {
		metrics->master_pref = (uint8)cmd_data->metrics.master_pref;
	}
	sub_cmd->id = htod16(WL_NAN_CMD_ELECTION_METRICS_CONFIG);
	sub_cmd->len = sizeof(sub_cmd->u.options) +
		sizeof(*metrics);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);

	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_data->nan_iov_buf += subcmd_len;

fail:
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_rssi_proximity(nan_config_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data, uint32 nan_attr_mask)
{
	s32 ret = BCME_OK;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	wl_nan_rssi_notif_thld_t *rssi_notif_thld = NULL;
	uint16 subcmd_len;

	NAN_DBG_ENTER();
	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);

	rssi_notif_thld = (wl_nan_rssi_notif_thld_t *)sub_cmd->data;

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(*rssi_notif_thld), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		return ret;
	}
	if (nan_attr_mask & NAN_ATTR_RSSI_PROXIMITY_2G_CONFIG) {
		rssi_notif_thld->bcn_rssi_2g =
			cmd_data->rssi_attr.rssi_proximity_2dot4g_val;
	} else {
		/* Keeping RSSI threshold value to be -70dBm */
		rssi_notif_thld->bcn_rssi_2g = NAN_DEF_RSSI_NOTIF_THRESH;
	}

	if (nan_attr_mask & NAN_ATTR_RSSI_PROXIMITY_5G_CONFIG) {
		rssi_notif_thld->bcn_rssi_5g =
			cmd_data->rssi_attr.rssi_proximity_5g_val;
	} else {
		/* Keeping RSSI threshold value to be -70dBm */
		rssi_notif_thld->bcn_rssi_5g = NAN_DEF_RSSI_NOTIF_THRESH;
	}

	sub_cmd->id = htod16(WL_NAN_CMD_SYNC_BCN_RSSI_NOTIF_THRESHOLD);
	sub_cmd->len = htod16(sizeof(sub_cmd->u.options) + sizeof(*rssi_notif_thld));
	sub_cmd->u.options = htod32(BCM_XTLV_OPTION_ALIGN32);

	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_data->nan_iov_buf += subcmd_len;

	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_rssi_mid_or_close(nan_config_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data, uint32 nan_attr_mask)
{
	s32 ret = BCME_OK;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	wl_nan_rssi_thld_t *rssi_thld = NULL;
	uint16 subcmd_len;

	NAN_DBG_ENTER();
	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);
	rssi_thld = (wl_nan_rssi_thld_t *)sub_cmd->data;

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(*rssi_thld), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		return ret;
	}

	/*
	 * Keeping RSSI mid value -75dBm for both 2G and 5G
	 * Keeping RSSI close value -60dBm for both 2G and 5G
	 */
	if (nan_attr_mask & NAN_ATTR_RSSI_MIDDLE_2G_CONFIG) {
		rssi_thld->rssi_mid_2g =
			cmd_data->rssi_attr.rssi_middle_2dot4g_val;
	} else {
		rssi_thld->rssi_mid_2g = NAN_DEF_RSSI_MID;
	}

	if (nan_attr_mask & NAN_ATTR_RSSI_MIDDLE_5G_CONFIG) {
		rssi_thld->rssi_mid_5g =
			cmd_data->rssi_attr.rssi_middle_5g_val;
	} else {
		rssi_thld->rssi_mid_5g = NAN_DEF_RSSI_MID;
	}

	if (nan_attr_mask & NAN_ATTR_RSSI_CLOSE_CONFIG) {
		rssi_thld->rssi_close_2g =
			cmd_data->rssi_attr.rssi_close_2dot4g_val;
	} else {
		rssi_thld->rssi_close_2g = NAN_DEF_RSSI_CLOSE;
	}

	if (nan_attr_mask & NAN_ATTR_RSSI_CLOSE_5G_CONFIG) {
		rssi_thld->rssi_close_5g =
			cmd_data->rssi_attr.rssi_close_5g_val;
	} else {
		rssi_thld->rssi_close_5g = NAN_DEF_RSSI_CLOSE;
	}

	sub_cmd->id = htod16(WL_NAN_CMD_ELECTION_RSSI_THRESHOLD);
	sub_cmd->len = htod16(sizeof(sub_cmd->u.options) + sizeof(*rssi_thld));
	sub_cmd->u.options = htod32(BCM_XTLV_OPTION_ALIGN32);

	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_data->nan_iov_buf += subcmd_len;

	NAN_DBG_EXIT();
	return ret;
}

static int
check_for_valid_5gchan(struct net_device *ndev, uint8 chan)
{
	s32 ret = BCME_OK;
	uint bitmap;
	u8 ioctl_buf[WLC_IOCTL_SMLEN];
	uint32 chanspec_arg;
	NAN_DBG_ENTER();

	chanspec_arg = CH20MHZ_CHSPEC(chan);
	chanspec_arg = wl_chspec_host_to_driver(chanspec_arg);
	memset_s(ioctl_buf, WLC_IOCTL_SMLEN, 0, WLC_IOCTL_SMLEN);
	ret = wldev_iovar_getbuf(ndev, "per_chan_info",
			(void *)&chanspec_arg, sizeof(chanspec_arg),
			ioctl_buf, WLC_IOCTL_SMLEN, NULL);
	if (ret != BCME_OK) {
		WL_ERR(("Chaninfo for channel = %d, error %d\n", chan, ret));
		goto exit;
	}

	bitmap = dtoh32(*(uint *)ioctl_buf);
	if (!(bitmap & WL_CHAN_VALID_HW)) {
		WL_ERR(("Invalid channel\n"));
		ret = BCME_BADCHAN;
		goto exit;
	}

	if (!(bitmap & WL_CHAN_VALID_SW)) {
		WL_ERR(("Not supported in current locale\n"));
		ret = BCME_BADCHAN;
		goto exit;
	}
exit:
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_nan_soc_chans(struct net_device *ndev, nan_config_cmd_data_t *cmd_data,
	wl_nan_iov_t *nan_iov_data, uint32 nan_attr_mask)
{
	s32 ret = BCME_OK;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	wl_nan_social_channels_t *soc_chans = NULL;
	uint16 subcmd_len;

	NAN_DBG_ENTER();

	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);
	soc_chans =
		(wl_nan_social_channels_t *)sub_cmd->data;

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(*soc_chans), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		return ret;
	}

	sub_cmd->id = htod16(WL_NAN_CMD_SYNC_SOCIAL_CHAN);
	sub_cmd->len = sizeof(sub_cmd->u.options) +
		sizeof(*soc_chans);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
	if (nan_attr_mask & NAN_ATTR_2G_CHAN_CONFIG) {
		soc_chans->soc_chan_2g = cmd_data->chanspec[1];
	} else {
		soc_chans->soc_chan_2g = NAN_DEF_SOCIAL_CHAN_2G;
	}

	if (cmd_data->support_5g) {
		if (nan_attr_mask & NAN_ATTR_5G_CHAN_CONFIG) {
			soc_chans->soc_chan_5g = cmd_data->chanspec[2];
		} else {
			soc_chans->soc_chan_5g = NAN_DEF_SOCIAL_CHAN_5G;
		}
		ret = check_for_valid_5gchan(ndev, soc_chans->soc_chan_5g);
		if (ret != BCME_OK) {
			ret = check_for_valid_5gchan(ndev, NAN_DEF_SEC_SOCIAL_CHAN_5G);
			if (ret == BCME_OK) {
				soc_chans->soc_chan_5g = NAN_DEF_SEC_SOCIAL_CHAN_5G;
			} else {
				soc_chans->soc_chan_5g = 0;
				ret = BCME_OK;
				WL_ERR(("Current locale doesn't support 5G op"
					"continuing with 2G only operation\n"));
			}
		}
	} else {
		WL_DBG(("5G support is disabled\n"));
	}
	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_data->nan_iov_buf += subcmd_len;

	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_nan_scan_params(struct net_device *ndev, struct bcm_cfg80211 *cfg,
	nan_config_cmd_data_t *cmd_data, uint8 band_index, uint32 nan_attr_mask)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	s32 ret = BCME_OK;
	uint16 nan_iov_start, nan_iov_end;
	uint16 nan_buf_size = NAN_IOCTL_BUF_SIZE;
	uint16 subcmd_len;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	wl_nan_iov_t *nan_iov_data = NULL;
	uint8 resp_buf[NAN_IOCTL_BUF_SIZE];
	wl_nan_scan_params_t *scan_params = NULL;
	uint32 status;

	NAN_DBG_ENTER();

	nan_buf = MALLOCZ(cfg->osh, nan_buf_size);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = MALLOCZ(cfg->osh, sizeof(*nan_iov_data));
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data->nan_iov_len = nan_iov_start = NAN_IOCTL_BUF_SIZE;
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);
	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(*scan_params), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		goto fail;
	}
	scan_params = (wl_nan_scan_params_t *)sub_cmd->data;

	sub_cmd->id = htod16(WL_NAN_CMD_CFG_SCAN_PARAMS);
	sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(*scan_params);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);

	if (!band_index) {
		/* Fw default: Dwell time for 2G is 210 */
		if ((nan_attr_mask & NAN_ATTR_2G_DWELL_TIME_CONFIG) &&
			cmd_data->dwell_time[0]) {
			scan_params->dwell_time = cmd_data->dwell_time[0] +
				NAN_SCAN_DWELL_TIME_DELTA_MS;
		}
		/* Fw default: Scan period for 2G is 10 */
		if (nan_attr_mask & NAN_ATTR_2G_SCAN_PERIOD_CONFIG) {
			scan_params->scan_period = cmd_data->scan_period[0];
		}
	} else {
		if ((nan_attr_mask & NAN_ATTR_5G_DWELL_TIME_CONFIG) &&
			cmd_data->dwell_time[1]) {
			scan_params->dwell_time = cmd_data->dwell_time[1] +
				NAN_SCAN_DWELL_TIME_DELTA_MS;
		}
		if (nan_attr_mask & NAN_ATTR_5G_SCAN_PERIOD_CONFIG) {
			scan_params->scan_period = cmd_data->scan_period[1];
		}
	}
	scan_params->band_index = band_index;
	nan_buf->is_set = true;
	nan_buf->count++;

	/* Reduce the iov_len size by subcmd_len */
	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_end = nan_iov_data->nan_iov_len;
	nan_buf_size = (nan_iov_start - nan_iov_end);

	memset_s(resp_buf, sizeof(resp_buf), 0, sizeof(resp_buf));
	ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, nan_buf_size, &status,
			(void*)resp_buf, NAN_IOCTL_BUF_SIZE);
	if (unlikely(ret) || unlikely(status)) {
		WL_ERR(("set nan scan params failed ret %d status %d \n", ret, status));
		goto fail;
	}
	WL_DBG(("set nan scan params successfull\n"));
fail:
	if (nan_buf) {
		MFREE(cfg->osh, nan_buf, NAN_IOCTL_BUF_SIZE);
	}
	if (nan_iov_data) {
		MFREE(cfg->osh, nan_iov_data, sizeof(*nan_iov_data));
	}

	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_cluster_id(nan_config_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data)
{
	s32 ret = BCME_OK;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	uint16 subcmd_len;

	NAN_DBG_ENTER();

	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			(sizeof(cmd_data->clus_id) - sizeof(uint8)), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		return ret;
	}

	cmd_data->clus_id.octet[0] = 0x50;
	cmd_data->clus_id.octet[1] = 0x6F;
	cmd_data->clus_id.octet[2] = 0x9A;
	cmd_data->clus_id.octet[3] = 0x01;
	WL_TRACE(("cluster_id = " MACDBG "\n", MAC2STRDBG(cmd_data->clus_id.octet)));

	sub_cmd->id = htod16(WL_NAN_CMD_CFG_CID);
	sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(cmd_data->clus_id);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
	ret = memcpy_s(sub_cmd->data, sizeof(cmd_data->clus_id),
			(uint8 *)&cmd_data->clus_id,
			sizeof(cmd_data->clus_id));
	if (ret != BCME_OK) {
		WL_ERR(("Failed to copy clus id\n"));
		return ret;
	}

	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_data->nan_iov_buf += subcmd_len;

	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_hop_count_limit(nan_config_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data)
{
	s32 ret = BCME_OK;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	wl_nan_hop_count_t *hop_limit = NULL;
	uint16 subcmd_len;

	NAN_DBG_ENTER();

	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);
	hop_limit = (wl_nan_hop_count_t *)sub_cmd->data;

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(*hop_limit), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		return ret;
	}

	*hop_limit = cmd_data->hop_count_limit;
	sub_cmd->id = htod16(WL_NAN_CMD_CFG_HOP_LIMIT);
	sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(*hop_limit);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);

	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_data->nan_iov_buf += subcmd_len;

	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_sid_beacon_val(nan_config_cmd_data_t *cmd_data,
	wl_nan_iov_t *nan_iov_data, uint32 nan_attr_mask)
{
	s32 ret = BCME_OK;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	wl_nan_sid_beacon_control_t *sid_beacon = NULL;
	uint16 subcmd_len;

	NAN_DBG_ENTER();

	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(*sid_beacon), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		return ret;
	}

	sid_beacon = (wl_nan_sid_beacon_control_t *)sub_cmd->data;
	sid_beacon->sid_enable = cmd_data->sid_beacon.sid_enable;
	/* Need to have separate flag for sub beacons
	 * sid_beacon->sub_sid_enable = cmd_data->sid_beacon.sub_sid_enable;
	 */
	if (nan_attr_mask & NAN_ATTR_SID_BEACON_CONFIG) {
		/* Limit for number of publish SIDs to be included in Beacons */
		sid_beacon->sid_count = cmd_data->sid_beacon.sid_count;
	}
	if (nan_attr_mask & NAN_ATTR_SUB_SID_BEACON_CONFIG) {
		/* Limit for number of subscribe SIDs to be included in Beacons */
		sid_beacon->sub_sid_count = cmd_data->sid_beacon.sub_sid_count;
	}
	sub_cmd->id = htod16(WL_NAN_CMD_CFG_SID_BEACON);
	sub_cmd->len = sizeof(sub_cmd->u.options) +
		sizeof(*sid_beacon);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);

	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_data->nan_iov_buf += subcmd_len;
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_nan_oui(nan_config_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data)
{
	s32 ret = BCME_OK;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	uint16 subcmd_len;

	NAN_DBG_ENTER();

	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(cmd_data->nan_oui), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		return ret;
	}

	sub_cmd->id = htod16(WL_NAN_CMD_CFG_OUI);
	sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(cmd_data->nan_oui);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
	ret = memcpy_s(sub_cmd->data, sizeof(cmd_data->nan_oui),
			(uint32 *)&cmd_data->nan_oui,
			sizeof(cmd_data->nan_oui));
	if (ret != BCME_OK) {
		WL_ERR(("Failed to copy nan oui\n"));
		return ret;
	}

	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_data->nan_iov_buf += subcmd_len;
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_set_awake_dws(struct net_device *ndev, nan_config_cmd_data_t *cmd_data,
		wl_nan_iov_t *nan_iov_data, struct bcm_cfg80211 *cfg, uint32 nan_attr_mask)
{
	s32 ret = BCME_OK;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	wl_nan_awake_dws_t *awake_dws = NULL;
	uint16 subcmd_len;
	NAN_DBG_ENTER();

	sub_cmd =
		(bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);
	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
			sizeof(*awake_dws), &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		return ret;
	}

	awake_dws = (wl_nan_awake_dws_t *)sub_cmd->data;

	if (nan_attr_mask & NAN_ATTR_2G_DW_CONFIG) {
		awake_dws->dw_interval_2g = cmd_data->awake_dws.dw_interval_2g;
		if (!awake_dws->dw_interval_2g) {
			/* Set 2G awake dw value to fw default value 1 */
			awake_dws->dw_interval_2g = NAN_SYNC_DEF_AWAKE_DW;
		}
	} else {
		/* Set 2G awake dw value to fw default value 1 */
		awake_dws->dw_interval_2g = NAN_SYNC_DEF_AWAKE_DW;
	}

	if (cfg->support_5g) {
		if (nan_attr_mask & NAN_ATTR_5G_DW_CONFIG) {
			awake_dws->dw_interval_5g = cmd_data->awake_dws.dw_interval_5g;
			if (!awake_dws->dw_interval_5g) {
				/* disable 5g beacon ctrls */
				ret = wl_cfgnan_config_control_flag(ndev, cfg,
						WL_NAN_CTRL_DISC_BEACON_TX_5G,
						&(cmd_data->status), 0);
				if (unlikely(ret) || unlikely(cmd_data->status)) {
					WL_ERR((" nan control set config handler,"
							" ret = %d status = %d \n",
							ret, cmd_data->status));
					goto fail;
				}
				ret = wl_cfgnan_config_control_flag(ndev, cfg,
						WL_NAN_CTRL_SYNC_BEACON_TX_5G,
						&(cmd_data->status), 0);
				if (unlikely(ret) || unlikely(cmd_data->status)) {
					WL_ERR((" nan control set config handler,"
							" ret = %d status = %d \n",
							ret, cmd_data->status));
					goto fail;
				}
			}
		} else {
			/* Set 5G awake dw value to fw default value 1 */
			awake_dws->dw_interval_5g = NAN_SYNC_DEF_AWAKE_DW;
			ret = wl_cfgnan_config_control_flag(ndev, cfg,
					WL_NAN_CTRL_DISC_BEACON_TX_5G |
					WL_NAN_CTRL_SYNC_BEACON_TX_5G,
					&(cmd_data->status), TRUE);
			if (unlikely(ret) || unlikely(cmd_data->status)) {
				WL_ERR((" nan control set config handler, ret = %d"
					" status = %d \n", ret, cmd_data->status));
				goto fail;
			}
		}
	}

	sub_cmd->id = htod16(WL_NAN_CMD_SYNC_AWAKE_DWS);
	sub_cmd->len = sizeof(sub_cmd->u.options) +
		sizeof(*awake_dws);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);

	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_data->nan_iov_buf += subcmd_len;

fail:
	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfgnan_start_handler(struct net_device *ndev, struct bcm_cfg80211 *cfg,
	nan_config_cmd_data_t *cmd_data, uint32 nan_attr_mask)
{
	s32 ret = BCME_OK;
	uint16 nan_buf_size = NAN_IOCTL_BUF_SIZE;
	bcm_iov_batch_buf_t *nan_buf = NULL;
	wl_nan_iov_t *nan_iov_data = NULL;
	dhd_pub_t *dhdp = wl_cfg80211_get_dhdp(ndev);
	uint8 resp_buf[NAN_IOCTL_BUF_SIZE];
	int i;
	s32 timeout = 0;
	nan_hal_capabilities_t capabilities;

	NAN_DBG_ENTER();

	/* Protect discovery creation. Ensure proper mutex precedence.
	 * If if_sync & nan_mutex comes together in same context, nan_mutex
	 * should follow if_sync.
	 */
	mutex_lock(&cfg->if_sync);
	NAN_MUTEX_LOCK();

	if (!dhdp->up) {
		WL_ERR(("bus is already down, hence blocking nan start\n"));
		ret = BCME_ERROR;
		NAN_MUTEX_UNLOCK();
		mutex_unlock(&cfg->if_sync);
		goto fail;
	}

#ifdef WL_IFACE_MGMT
	if ((ret = wl_cfg80211_handle_if_role_conflict(cfg, WL_IF_TYPE_NAN_NMI)) != BCME_OK) {
		WL_ERR(("Conflicting iface is present, cant support nan\n"));
		NAN_MUTEX_UNLOCK();
		mutex_unlock(&cfg->if_sync);
		goto fail;
	}
#endif /* WL_IFACE_MGMT */

	WL_INFORM_MEM(("Initializing NAN\n"));
	ret = wl_cfgnan_init(cfg);
	if (ret != BCME_OK) {
		WL_ERR(("failed to initialize NAN[%d]\n", ret));
		NAN_MUTEX_UNLOCK();
		mutex_unlock(&cfg->if_sync);
		goto fail;
	}

	ret = wl_cfgnan_get_ver(ndev, cfg);
	if (ret != BCME_OK) {
		WL_ERR(("failed to Nan IOV version[%d]\n", ret));
		NAN_MUTEX_UNLOCK();
		mutex_unlock(&cfg->if_sync);
		goto fail;
	}

	/* set nmi addr */
	ret = wl_cfgnan_set_if_addr(cfg);
	if (ret != BCME_OK) {
		WL_ERR(("Failed to set nmi address \n"));
		NAN_MUTEX_UNLOCK();
		mutex_unlock(&cfg->if_sync);
		goto fail;
	}
	cfg->nancfg.nan_event_recvd = false;
	NAN_MUTEX_UNLOCK();
	mutex_unlock(&cfg->if_sync);

	for (i = 0; i < NAN_MAX_NDI; i++) {
		/* Create NDI using the information provided by user space */
		if (cfg->nancfg.ndi[i].in_use && !cfg->nancfg.ndi[i].created) {
			ret = wl_cfgnan_data_path_iface_create_delete_handler(ndev, cfg,
				cfg->nancfg.ndi[i].ifname,
				NAN_WIFI_SUBCMD_DATA_PATH_IFACE_CREATE, dhdp->up);
			if (ret) {
				WL_ERR(("failed to create ndp interface [%d]\n", ret));
				goto fail;
			}
			cfg->nancfg.ndi[i].created = true;
		}
	}

	nan_buf = MALLOCZ(cfg->osh, nan_buf_size);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = MALLOCZ(cfg->osh, sizeof(*nan_iov_data));
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data->nan_iov_len = NAN_IOCTL_BUF_SIZE;
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

	if (nan_attr_mask & NAN_ATTR_SYNC_DISC_2G_BEACON_CONFIG) {
		/* config sync/discovery beacons on 2G band */
		/* 2g is mandatory */
		if (!cmd_data->beacon_2g_val) {
			WL_ERR(("Invalid NAN config...2G is mandatory\n"));
			ret = BCME_BADARG;
		}
		ret = wl_cfgnan_config_control_flag(ndev, cfg,
			WL_NAN_CTRL_DISC_BEACON_TX_2G | WL_NAN_CTRL_SYNC_BEACON_TX_2G,
			&(cmd_data->status), TRUE);
		if (unlikely(ret) || unlikely(cmd_data->status)) {
			WL_ERR((" nan control set config handler, ret = %d status = %d \n",
					ret, cmd_data->status));
			goto fail;
		}
	}
	if (nan_attr_mask & NAN_ATTR_SYNC_DISC_5G_BEACON_CONFIG) {
		/* config sync/discovery beacons on 5G band */
		ret = wl_cfgnan_config_control_flag(ndev, cfg,
			WL_NAN_CTRL_DISC_BEACON_TX_5G | WL_NAN_CTRL_SYNC_BEACON_TX_5G,
			&(cmd_data->status), cmd_data->beacon_5g_val);
		if (unlikely(ret) || unlikely(cmd_data->status)) {
			WL_ERR((" nan control set config handler, ret = %d status = %d \n",
					ret, cmd_data->status));
			goto fail;
		}
	}
	/* Setting warm up time */
	cmd_data->warmup_time = 1;
	if (cmd_data->warmup_time) {
		ret = wl_cfgnan_warmup_time_handler(cmd_data, nan_iov_data);
		if (unlikely(ret)) {
			WL_ERR(("warm up time handler sub_cmd set failed\n"));
			goto fail;
		}
		nan_buf->count++;
	}
	/* setting master preference and random factor */
	ret = wl_cfgnan_set_election_metric(cmd_data, nan_iov_data, nan_attr_mask);
	if (unlikely(ret)) {
		WL_ERR(("election_metric sub_cmd set failed\n"));
		goto fail;
	} else {
		nan_buf->count++;
	}

	/* setting nan social channels */
	ret = wl_cfgnan_set_nan_soc_chans(ndev, cmd_data, nan_iov_data, nan_attr_mask);
	if (unlikely(ret)) {
		WL_ERR(("nan social channels set failed\n"));
		goto fail;
	} else {
		/* Storing 5g capability which is reqd for avail chan config. */
		cfg->support_5g = cmd_data->support_5g;
		nan_buf->count++;
	}

	if ((cmd_data->support_2g) && ((cmd_data->dwell_time[0]) ||
			(cmd_data->scan_period[0]))) {
		/* setting scan params */
		ret = wl_cfgnan_set_nan_scan_params(ndev, cfg, cmd_data, 0, nan_attr_mask);
		if (unlikely(ret)) {
			WL_ERR(("scan params set failed for 2g\n"));
			goto fail;
		}
	}

	if ((cmd_data->support_5g) && ((cmd_data->dwell_time[1]) ||
			(cmd_data->scan_period[1]))) {
		/* setting scan params */
		ret = wl_cfgnan_set_nan_scan_params(ndev, cfg, cmd_data,
			cmd_data->support_5g, nan_attr_mask);
		if (unlikely(ret)) {
			WL_ERR(("scan params set failed for 5g\n"));
			goto fail;
		}
	}

	/*
	 * A cluster_low value matching cluster_high indicates a request
	 * to join a cluster with that value.
	 * If the requested cluster is not found the
	 * device will start its own cluster
	 */
	/* For Debug purpose, using clust id compulsion */
	if (!ETHER_ISNULLADDR(&cmd_data->clus_id.octet)) {
		if (cmd_data->clus_id.octet[4] == cmd_data->clus_id.octet[5]) {
			/* device will merge to configured CID only */
			ret = wl_cfgnan_config_control_flag(ndev, cfg,
					WL_NAN_CTRL_MERGE_CONF_CID_ONLY, &(cmd_data->status), true);
			if (unlikely(ret) || unlikely(cmd_data->status)) {
				WL_ERR((" nan control set config handler, ret = %d status = %d \n",
					ret, cmd_data->status));
				goto fail;
			}
		}
		/* setting cluster ID */
		ret = wl_cfgnan_set_cluster_id(cmd_data, nan_iov_data);
		if (unlikely(ret)) {
			WL_ERR(("cluster_id sub_cmd set failed\n"));
			goto fail;
		}
		nan_buf->count++;
	}

	/* setting rssi proximaty values for 2.4GHz and 5GHz */
	ret = wl_cfgnan_set_rssi_proximity(cmd_data, nan_iov_data, nan_attr_mask);
	if (unlikely(ret)) {
		WL_ERR(("2.4GHz/5GHz rssi proximity threshold set failed\n"));
		goto fail;
	} else {
		nan_buf->count++;
	}

	/* setting rssi middle/close values for 2.4GHz and 5GHz */
	ret = wl_cfgnan_set_rssi_mid_or_close(cmd_data, nan_iov_data, nan_attr_mask);
	if (unlikely(ret)) {
		WL_ERR(("2.4GHz/5GHz rssi middle and close set failed\n"));
		goto fail;
	} else {
		nan_buf->count++;
	}

	/* setting hop count limit or threshold */
	if (nan_attr_mask & NAN_ATTR_HOP_COUNT_LIMIT_CONFIG) {
		ret = wl_cfgnan_set_hop_count_limit(cmd_data, nan_iov_data);
		if (unlikely(ret)) {
			WL_ERR(("hop_count_limit sub_cmd set failed\n"));
			goto fail;
		}
		nan_buf->count++;
	}

	/* setting sid beacon val */
	if ((nan_attr_mask & NAN_ATTR_SID_BEACON_CONFIG) ||
		(nan_attr_mask & NAN_ATTR_SUB_SID_BEACON_CONFIG)) {
		ret = wl_cfgnan_set_sid_beacon_val(cmd_data, nan_iov_data, nan_attr_mask);
		if (unlikely(ret)) {
			WL_ERR(("sid_beacon sub_cmd set failed\n"));
			goto fail;
		}
		nan_buf->count++;
	}

	/* setting nan oui */
	if (nan_attr_mask & NAN_ATTR_OUI_CONFIG) {
		ret = wl_cfgnan_set_nan_oui(cmd_data, nan_iov_data);
		if (unlikely(ret)) {
			WL_ERR(("nan_oui sub_cmd set failed\n"));
			goto fail;
		}
		nan_buf->count++;
	}

	/* setting nan awake dws */
	ret = wl_cfgnan_set_awake_dws(ndev, cmd_data,
			nan_iov_data, cfg, nan_attr_mask);
	if (unlikely(ret)) {
		WL_ERR(("nan awake dws set failed\n"));
		goto fail;
	} else {
		nan_buf->count++;
	}

	/* enable events */
	ret = wl_cfgnan_config_eventmask(ndev, cfg, cmd_data->disc_ind_cfg, false);
	if (unlikely(ret)) {
		WL_ERR(("Failed to config disc ind flag in event_mask, ret = %d\n", ret));
		goto fail;
	}

	/* setting nan enable sub_cmd */
	ret = wl_cfgnan_enable_handler(nan_iov_data, true);
	if (unlikely(ret)) {
		WL_ERR(("enable handler sub_cmd set failed\n"));
		goto fail;
	}
	nan_buf->count++;
	nan_buf->is_set = true;

	nan_buf_size -= nan_iov_data->nan_iov_len;
	memset(resp_buf, 0, sizeof(resp_buf));
	/* Reset conditon variable */
	ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, nan_buf_size,
			&(cmd_data->status), (void*)resp_buf, NAN_IOCTL_BUF_SIZE);
	if (unlikely(ret) || unlikely(cmd_data->status)) {
		WL_ERR((" nan start handler, enable failed, ret = %d status = %d \n",
				ret, cmd_data->status));
		goto fail;
	}

	timeout = wait_event_timeout(cfg->nancfg.nan_event_wait,
		cfg->nancfg.nan_event_recvd, msecs_to_jiffies(NAN_START_STOP_TIMEOUT));
	if (!timeout) {
		WL_ERR(("Timed out while Waiting for WL_NAN_EVENT_START event !!!\n"));
		ret = BCME_ERROR;
		goto fail;
	}

	/* If set, auto datapath confirms will be sent by FW */
	ret = wl_cfgnan_config_control_flag(ndev, cfg, WL_NAN_CTRL_AUTO_DPCONF,
		&(cmd_data->status), true);
	if (unlikely(ret) || unlikely(cmd_data->status)) {
		WL_ERR((" nan control set config handler, ret = %d status = %d \n",
				ret, cmd_data->status));
		goto fail;
	}

	/* By default set NAN proprietary rates */
	ret = wl_cfgnan_config_control_flag(ndev, cfg, WL_NAN_CTRL_PROP_RATE,
		&(cmd_data->status), true);
	if (unlikely(ret) || unlikely(cmd_data->status)) {
		WL_ERR((" nan proprietary rate set failed, ret = %d status = %d \n",
				ret, cmd_data->status));
		goto fail;
	}

	/* malloc for ndp peer list */
	if ((ret = wl_cfgnan_get_capablities_handler(ndev, cfg, &capabilities))
			== BCME_OK) {
		cfg->nancfg.max_ndp_count = capabilities.max_ndp_sessions;
		cfg->nancfg.nan_ndp_peer_info = MALLOCZ(cfg->osh,
				cfg->nancfg.max_ndp_count * sizeof(nan_ndp_peer_t));
		if (!cfg->nancfg.nan_ndp_peer_info) {
			WL_ERR(("%s: memory allocation failed\n", __func__));
			ret = BCME_NOMEM;
			goto fail;
		}

	} else {
		WL_ERR(("wl_cfgnan_get_capablities_handler failed, ret = %d\n", ret));
		goto fail;
	}

#ifdef RTT_SUPPORT
	/* Initialize geofence cfg */
	dhd_rtt_initialize_geofence_cfg(cfg->pub);
#endif /* RTT_SUPPORT */

	cfg->nan_enable = true;
	WL_INFORM_MEM(("[NAN] Enable successfull \n"));
	/* disable TDLS on NAN NMI IF create  */
	wl_cfg80211_tdls_config(cfg, TDLS_STATE_NMI_CREATE, false);

fail:
	/* reset conditon variable */
	cfg->nancfg.nan_event_recvd = false;
	if (unlikely(ret) || unlikely(cmd_data->status)) {
		cfg->nan_enable = false;
		mutex_lock(&cfg->if_sync);
		ret = wl_cfg80211_delete_iface(cfg, WL_IF_TYPE_NAN);
		if (ret != BCME_OK) {
			WL_ERR(("failed to delete NDI[%d]\n", ret));
		}
		mutex_unlock(&cfg->if_sync);
	}
	if (nan_buf) {
		MFREE(cfg->osh, nan_buf, NAN_IOCTL_BUF_SIZE);
	}
	if (nan_iov_data) {
		MFREE(cfg->osh, nan_iov_data, sizeof(*nan_iov_data));
	}

	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfgnan_disable(struct bcm_cfg80211 *cfg)
{
	s32 ret = BCME_OK;
	dhd_pub_t *dhdp = (dhd_pub_t *)(cfg->pub);

	NAN_DBG_ENTER();
	if ((cfg->nan_init_state == TRUE) &&
			(cfg->nan_enable == TRUE)) {
		struct net_device *ndev;
		ndev = bcmcfg_to_prmry_ndev(cfg);

		/* We have to remove NDIs so that P2P/Softap can work */
		ret = wl_cfg80211_delete_iface(cfg, WL_IF_TYPE_NAN);
		if (ret != BCME_OK) {
			WL_ERR(("failed to delete NDI[%d]\n", ret));
		}

		WL_INFORM_MEM(("Nan Disable Req, reason = %d\n", cfg->nancfg.disable_reason));
		ret = wl_cfgnan_stop_handler(ndev, cfg);
		if (ret == -ENODEV) {
			WL_ERR(("Bus is down, no need to proceed\n"));
		} else if (ret != BCME_OK) {
			WL_ERR(("failed to stop nan, error[%d]\n", ret));
		}
		ret = wl_cfgnan_deinit(cfg, dhdp->up);
		if (ret != BCME_OK) {
			WL_ERR(("failed to de-initialize NAN[%d]\n", ret));
			if (!dhd_query_bus_erros(dhdp)) {
				ASSERT(0);
			}
		}
		wl_cfgnan_disable_cleanup(cfg);
	}
	NAN_DBG_EXIT();
	return ret;
}

static void
wl_cfgnan_send_stop_event(struct bcm_cfg80211 *cfg)
{
	s32 ret = BCME_OK;
	nan_event_data_t *nan_event_data = NULL;

	NAN_DBG_ENTER();

	if (cfg->nancfg.disable_reason == NAN_USER_INITIATED) {
	    /* do not event to host if command is from host */
	    goto exit;
	}
	nan_event_data = MALLOCZ(cfg->osh, sizeof(nan_event_data_t));
	if (!nan_event_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto exit;
	}
	bzero(nan_event_data, sizeof(nan_event_data_t));

	if (cfg->nancfg.disable_reason == NAN_CONCURRENCY_CONFLICT) {
	   nan_event_data->status = NAN_STATUS_UNSUPPORTED_CONCURRENCY_NAN_DISABLED;
	} else {
	   nan_event_data->status = NAN_STATUS_SUCCESS;
	}

	nan_event_data->status = NAN_STATUS_SUCCESS;
	ret = memcpy_s(nan_event_data->nan_reason, NAN_ERROR_STR_LEN,
			"NAN_STATUS_SUCCESS", strlen("NAN_STATUS_SUCCESS"));
	if (ret != BCME_OK) {
		WL_ERR(("Failed to copy nan reason string, ret = %d\n", ret));
		goto exit;
	}
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 13, 0)) || defined(WL_VENDOR_EXT_SUPPORT)
	ret = wl_cfgvendor_send_nan_event(cfg->wdev->wiphy, bcmcfg_to_prmry_ndev(cfg),
			GOOGLE_NAN_EVENT_DISABLED, nan_event_data);
	if (ret != BCME_OK) {
		WL_ERR(("Failed to send event to nan hal, (%d)\n",
				GOOGLE_NAN_EVENT_DISABLED));
	}
#endif /* (LINUX_VERSION_CODE > KERNEL_VERSION(3, 13, 0)) || defined(WL_VENDOR_EXT_SUPPORT) */
exit:
	if (nan_event_data) {
		MFREE(cfg->osh, nan_event_data, sizeof(nan_event_data_t));
	}
	NAN_DBG_EXIT();
	return;
}

void wl_cfgnan_disable_cleanup(struct bcm_cfg80211 *cfg)
{
	int i = 0;
#ifdef RTT_SUPPORT
	dhd_pub_t *dhdp = (dhd_pub_t *)(cfg->pub);
	rtt_status_info_t *rtt_status = GET_RTTSTATE(dhdp);
	rtt_target_info_t *target_info = NULL;

	/* Delete the geofence rtt target list */
	dhd_rtt_delete_geofence_target_list(dhdp);
	/* Cancel pending retry timer if any */
	if (delayed_work_pending(&rtt_status->rtt_retry_timer)) {
		cancel_delayed_work_sync(&rtt_status->rtt_retry_timer);
	}
	/* Remove if any pending proxd timeout for nan-rtt */
	target_info = &rtt_status->rtt_config.target_info[rtt_status->cur_idx];
	if (target_info && target_info->peer == RTT_PEER_NAN) {
		/* Cancel pending proxd timeout work if any */
		if (delayed_work_pending(&rtt_status->proxd_timeout)) {
			cancel_delayed_work_sync(&rtt_status->proxd_timeout);
		}
	}
	/* Delete if any directed nan rtt session */
	dhd_rtt_delete_nan_session(dhdp);
#endif /* RTT_SUPPORT */
	/* Clear the NDP ID array and dp count */
	for (i = 0; i < NAN_MAX_NDP_PEER; i++) {
		cfg->nancfg.ndp_id[i] = 0;
	}
	cfg->nan_dp_count = 0;
	if (cfg->nancfg.nan_ndp_peer_info) {
		MFREE(cfg->osh, cfg->nancfg.nan_ndp_peer_info,
			cfg->nancfg.max_ndp_count * sizeof(nan_ndp_peer_t));
		cfg->nancfg.nan_ndp_peer_info = NULL;
	}
	return;
}

/*
 * Deferred nan disable work,
 * scheduled with 3sec delay in order to remove any active nan dps
 */
void
wl_cfgnan_delayed_disable(struct work_struct *work)
{
	struct bcm_cfg80211 *cfg = NULL;

	BCM_SET_CONTAINER_OF(cfg, work, struct bcm_cfg80211, nan_disable.work);

	rtnl_lock();
	wl_cfgnan_disable(cfg);
	rtnl_unlock();
}

int
wl_cfgnan_stop_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	s32 ret = BCME_OK;
	uint16 nan_buf_size = NAN_IOCTL_BUF_SIZE;
	wl_nan_iov_t *nan_iov_data = NULL;
	uint32 status;
	uint8 resp_buf[NAN_IOCTL_BUF_SIZE];

	NAN_DBG_ENTER();
	NAN_MUTEX_LOCK();

	if (!cfg->nan_enable) {
		WL_INFORM(("Nan is not enabled\n"));
		ret = BCME_OK;
		goto fail;
	}

	if (cfg->nancfg.disable_reason != NAN_BUS_IS_DOWN) {
		/*
		 * Framework doing cleanup(iface remove) on disable command,
		 * so avoiding event to prevent iface delete calls again
		 */
		WL_INFORM_MEM(("[NAN] Disabling Nan events\n"));
		wl_cfgnan_config_eventmask(ndev, cfg, 0, true);

		nan_buf = MALLOCZ(cfg->osh, nan_buf_size);
		if (!nan_buf) {
			WL_ERR(("%s: memory allocation failed\n", __func__));
			ret = BCME_NOMEM;
			goto fail;
		}

		nan_iov_data = MALLOCZ(cfg->osh, sizeof(*nan_iov_data));
		if (!nan_iov_data) {
			WL_ERR(("%s: memory allocation failed\n", __func__));
			ret = BCME_NOMEM;
			goto fail;
		}

		nan_iov_data->nan_iov_len = NAN_IOCTL_BUF_SIZE;
		nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
		nan_buf->count = 0;
		nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
		nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

		ret = wl_cfgnan_enable_handler(nan_iov_data, false);
		if (unlikely(ret)) {
			WL_ERR(("nan disable handler failed\n"));
			goto fail;
		}
		nan_buf->count++;
		nan_buf->is_set = true;
		nan_buf_size -= nan_iov_data->nan_iov_len;
		memset_s(resp_buf, sizeof(resp_buf),
				0, sizeof(resp_buf));
		ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, nan_buf_size, &status,
				(void*)resp_buf, NAN_IOCTL_BUF_SIZE);
		if (unlikely(ret) || unlikely(status)) {
			WL_ERR(("nan disable failed ret = %d status = %d\n", ret, status));
			goto fail;
		}
		/* Enable back TDLS if connected interface is <= 1 */
		wl_cfg80211_tdls_config(cfg, TDLS_STATE_IF_DELETE, false);
	}

	wl_cfgnan_send_stop_event(cfg);

fail:
	/* Resetting instance ID mask */
	cfg->nancfg.inst_id_start = 0;
	memset(cfg->nancfg.svc_inst_id_mask, 0, sizeof(cfg->nancfg.svc_inst_id_mask));
	memset(cfg->svc_info, 0, NAN_MAX_SVC_INST * sizeof(nan_svc_info_t));
	cfg->nan_enable = false;

	if (nan_buf) {
		MFREE(cfg->osh, nan_buf, NAN_IOCTL_BUF_SIZE);
	}
	if (nan_iov_data) {
		MFREE(cfg->osh, nan_iov_data, sizeof(*nan_iov_data));
	}

	NAN_MUTEX_UNLOCK();
	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfgnan_config_handler(struct net_device *ndev, struct bcm_cfg80211 *cfg,
	nan_config_cmd_data_t *cmd_data, uint32 nan_attr_mask)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	s32 ret = BCME_OK;
	uint16 nan_buf_size = NAN_IOCTL_BUF_SIZE;
	wl_nan_iov_t *nan_iov_data = NULL;
	uint8 resp_buf[NAN_IOCTL_BUF_SIZE];

	NAN_DBG_ENTER();

	/* Nan need to be enabled before configuring/updating params */
	if (cfg->nan_enable) {
		nan_buf = MALLOCZ(cfg->osh, nan_buf_size);
		if (!nan_buf) {
			WL_ERR(("%s: memory allocation failed\n", __func__));
			ret = BCME_NOMEM;
			goto fail;
		}

		nan_iov_data = MALLOCZ(cfg->osh, sizeof(*nan_iov_data));
		if (!nan_iov_data) {
			WL_ERR(("%s: memory allocation failed\n", __func__));
			ret = BCME_NOMEM;
			goto fail;
		}

		nan_iov_data->nan_iov_len = NAN_IOCTL_BUF_SIZE;
		nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
		nan_buf->count = 0;
		nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
		nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

		/* setting sid beacon val */
		if ((nan_attr_mask & NAN_ATTR_SID_BEACON_CONFIG) ||
			(nan_attr_mask & NAN_ATTR_SUB_SID_BEACON_CONFIG)) {
			ret = wl_cfgnan_set_sid_beacon_val(cmd_data, nan_iov_data, nan_attr_mask);
			if (unlikely(ret)) {
				WL_ERR(("sid_beacon sub_cmd set failed\n"));
				goto fail;
			}
			nan_buf->count++;
		}

		/* setting master preference and random factor */
		if (cmd_data->metrics.random_factor ||
			cmd_data->metrics.master_pref) {
			ret = wl_cfgnan_set_election_metric(cmd_data, nan_iov_data,
					nan_attr_mask);
			if (unlikely(ret)) {
				WL_ERR(("election_metric sub_cmd set failed\n"));
				goto fail;
			} else {
				nan_buf->count++;
			}
		}

		/* setting hop count limit or threshold */
		if (nan_attr_mask & NAN_ATTR_HOP_COUNT_LIMIT_CONFIG) {
			ret = wl_cfgnan_set_hop_count_limit(cmd_data, nan_iov_data);
			if (unlikely(ret)) {
				WL_ERR(("hop_count_limit sub_cmd set failed\n"));
				goto fail;
			}
			nan_buf->count++;
		}

		/* setting rssi proximaty values for 2.4GHz and 5GHz */
		ret = wl_cfgnan_set_rssi_proximity(cmd_data, nan_iov_data,
				nan_attr_mask);
		if (unlikely(ret)) {
			WL_ERR(("2.4GHz/5GHz rssi proximity threshold set failed\n"));
			goto fail;
		} else {
			nan_buf->count++;
		}

		/* setting nan awake dws */
		ret = wl_cfgnan_set_awake_dws(ndev, cmd_data, nan_iov_data,
			cfg, nan_attr_mask);
		if (unlikely(ret)) {
			WL_ERR(("nan awake dws set failed\n"));
			goto fail;
		} else {
			nan_buf->count++;
		}

		if (cmd_data->disc_ind_cfg) {
			/* Disable events */
			WL_TRACE(("Disable events based on flag\n"));
			ret = wl_cfgnan_config_eventmask(ndev, cfg,
				cmd_data->disc_ind_cfg, false);
			if (unlikely(ret)) {
				WL_ERR(("Failed to config disc ind flag in event_mask, ret = %d\n",
					ret));
				goto fail;
			}
		}

		if ((cfg->support_5g) && ((cmd_data->dwell_time[1]) ||
				(cmd_data->scan_period[1]))) {
			/* setting scan params */
			ret = wl_cfgnan_set_nan_scan_params(ndev, cfg,
					cmd_data, cfg->support_5g, nan_attr_mask);
			if (unlikely(ret)) {
				WL_ERR(("scan params set failed for 5g\n"));
				goto fail;
			}
		}
		if ((cmd_data->dwell_time[0]) ||
				(cmd_data->scan_period[0])) {
			ret = wl_cfgnan_set_nan_scan_params(ndev, cfg, cmd_data, 0, nan_attr_mask);
			if (unlikely(ret)) {
				WL_ERR(("scan params set failed for 2g\n"));
				goto fail;
			}
		}
		nan_buf->is_set = true;
		nan_buf_size -= nan_iov_data->nan_iov_len;

		if (nan_buf->count) {
			memset_s(resp_buf, sizeof(resp_buf),
				0, sizeof(resp_buf));
			ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, nan_buf_size,
					&(cmd_data->status),
					(void*)resp_buf, NAN_IOCTL_BUF_SIZE);
			if (unlikely(ret) || unlikely(cmd_data->status)) {
				WL_ERR((" nan config handler failed ret = %d status = %d\n",
					ret, cmd_data->status));
				goto fail;
			}
		} else {
			WL_DBG(("No commands to send\n"));
		}

		if ((!cmd_data->bmap) || (cmd_data->avail_params.duration == NAN_BAND_INVALID) ||
				(!cmd_data->chanspec[0])) {
			WL_TRACE(("mandatory arguments are not present to set avail\n"));
			ret = BCME_OK;
		} else {
			cmd_data->avail_params.chanspec[0] = cmd_data->chanspec[0];
			cmd_data->avail_params.bmap = cmd_data->bmap;
			/* 1=local, 2=peer, 3=ndc, 4=immutable, 5=response, 6=counter */
			ret = wl_cfgnan_set_nan_avail(bcmcfg_to_prmry_ndev(cfg),
					cfg, &cmd_data->avail_params, WL_AVAIL_LOCAL);
			if (unlikely(ret)) {
				WL_ERR(("Failed to set avail value with type local\n"));
				goto fail;
			}

			ret = wl_cfgnan_set_nan_avail(bcmcfg_to_prmry_ndev(cfg),
					cfg, &cmd_data->avail_params, WL_AVAIL_NDC);
			if (unlikely(ret)) {
				WL_ERR(("Failed to set avail value with type ndc\n"));
				goto fail;
			}
		}
	} else {
		WL_INFORM(("nan is not enabled\n"));
	}

fail:
	if (nan_buf) {
		MFREE(cfg->osh, nan_buf, NAN_IOCTL_BUF_SIZE);
	}
	if (nan_iov_data) {
		MFREE(cfg->osh, nan_iov_data, sizeof(*nan_iov_data));
	}

	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfgnan_support_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, nan_config_cmd_data_t *cmd_data)
{
	/* TODO: */
	return BCME_OK;
}

int
wl_cfgnan_status_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, nan_config_cmd_data_t *cmd_data)
{
	/* TODO: */
	return BCME_OK;
}

#ifdef WL_NAN_DISC_CACHE
static
nan_svc_info_t *
wl_cfgnan_get_svc_inst(struct bcm_cfg80211 *cfg,
	wl_nan_instance_id svc_inst_id, uint8 ndp_id)
{
	uint8 i, j;
	if (ndp_id) {
		for (i = 0; i < NAN_MAX_SVC_INST; i++) {
			for (j = 0; j < NAN_MAX_SVC_INST; j++) {
				if (cfg->svc_info[i].ndp_id[j] == ndp_id) {
					return &cfg->svc_info[i];
				}
			}
		}
	} else if (svc_inst_id) {
		for (i = 0; i < NAN_MAX_SVC_INST; i++) {
			if (cfg->svc_info[i].svc_id == svc_inst_id) {
				return &cfg->svc_info[i];
			}
		}

	}
	return NULL;
}

nan_ranging_inst_t *
wl_cfgnan_check_for_ranging(struct bcm_cfg80211 *cfg, struct ether_addr *peer)
{
	uint8 i;
	if (peer) {
		for (i = 0; i < NAN_MAX_RANGING_INST; i++) {
			if (!memcmp(peer, &cfg->nan_ranging_info[i].peer_addr,
				ETHER_ADDR_LEN)) {
				return &(cfg->nan_ranging_info[i]);
			}
		}
	}
	return NULL;
}

nan_ranging_inst_t *
wl_cfgnan_get_rng_inst_by_id(struct bcm_cfg80211 *cfg, uint8 rng_id)
{
	uint8 i;
	if (rng_id) {
		for (i = 0; i < NAN_MAX_RANGING_INST; i++) {
			if (cfg->nan_ranging_info[i].range_id == rng_id)
			{
				return &(cfg->nan_ranging_info[i]);
			}
		}
	}
	WL_ERR(("Couldn't find the ranging instance for rng_id %d\n", rng_id));
	return NULL;
}

/*
 * Find ranging inst for given peer,
 * On not found, create one
 * with given range role
 */
nan_ranging_inst_t *
wl_cfgnan_get_ranging_inst(struct bcm_cfg80211 *cfg, struct ether_addr *peer,
	nan_range_role_t range_role)
{
	nan_ranging_inst_t *ranging_inst = NULL;
	uint8 i;

	if (!peer) {
		WL_ERR(("Peer address is NULL"));
		goto done;
	}

	ranging_inst = wl_cfgnan_check_for_ranging(cfg, peer);
	if (ranging_inst) {
		goto done;
	}
	WL_TRACE(("Creating Ranging instance \n"));

	for (i =  0; i < NAN_MAX_RANGING_INST; i++) {
		if (cfg->nan_ranging_info[i].in_use == FALSE) {
			break;
		}
	}

	if (i == NAN_MAX_RANGING_INST) {
		WL_ERR(("No buffer available for the ranging instance"));
		goto done;
	}
	ranging_inst = &cfg->nan_ranging_info[i];
	memcpy(&ranging_inst->peer_addr, peer, ETHER_ADDR_LEN);
	ranging_inst->range_status = NAN_RANGING_REQUIRED;
	ranging_inst->prev_distance_mm = INVALID_DISTANCE;
	ranging_inst->range_role = range_role;
	ranging_inst->in_use = TRUE;

done:
	return ranging_inst;
}
#endif /* WL_NAN_DISC_CACHE */

static int
process_resp_buf(void *iov_resp,
	uint8 *instance_id, uint16 sub_cmd_id)
{
	int res = BCME_OK;
	NAN_DBG_ENTER();

	if (sub_cmd_id == WL_NAN_CMD_DATA_DATAREQ) {
		wl_nan_dp_req_ret_t *dpreq_ret = NULL;
		dpreq_ret = (wl_nan_dp_req_ret_t *)(iov_resp);
		*instance_id = dpreq_ret->ndp_id;
		WL_TRACE(("%s: Initiator NDI: " MACDBG "\n",
			__FUNCTION__, MAC2STRDBG(dpreq_ret->indi.octet)));
	} else if (sub_cmd_id == WL_NAN_CMD_RANGE_REQUEST) {
		wl_nan_range_id *range_id = NULL;
		range_id = (wl_nan_range_id *)(iov_resp);
		*instance_id = *range_id;
		WL_TRACE(("Range id: %d\n", *range_id));
	}
	WL_DBG(("instance_id: %d\n", *instance_id));
	NAN_DBG_EXIT();
	return res;
}

int
wl_cfgnan_cancel_ranging(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, uint8 range_id, uint8 flags, uint32 *status)
{
	bcm_iov_batch_buf_t *nan_buf = NULL;
	s32 ret = BCME_OK;
	uint16 nan_iov_start, nan_iov_end;
	uint16 nan_buf_size = NAN_IOCTL_BUF_SIZE;
	uint16 subcmd_len;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	wl_nan_iov_t *nan_iov_data = NULL;
	uint8 resp_buf[NAN_IOCTL_BUF_SIZE];
	wl_nan_range_cancel_ext_t rng_cncl;
	uint8 size_of_iov;

	NAN_DBG_ENTER();

	if (cfg->nancfg.version >= NAN_RANGE_EXT_CANCEL_SUPPORT_VER) {
		size_of_iov = sizeof(rng_cncl);
	} else {
		size_of_iov = sizeof(range_id);
	}

	memset_s(&rng_cncl, sizeof(rng_cncl), 0, sizeof(rng_cncl));
	rng_cncl.range_id = range_id;
	rng_cncl.flags = flags;

	nan_buf = MALLOCZ(cfg->osh, nan_buf_size);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data = MALLOCZ(cfg->osh, sizeof(*nan_iov_data));
	if (!nan_iov_data) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_iov_data->nan_iov_len = nan_iov_start = NAN_IOCTL_BUF_SIZE;
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_iov_data->nan_iov_buf = (uint8 *)(&nan_buf->cmds[0]);
	nan_iov_data->nan_iov_len -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);
	sub_cmd = (bcm_iov_batch_subcmd_t*)(nan_iov_data->nan_iov_buf);

	ret = wl_cfg_nan_check_cmd_len(nan_iov_data->nan_iov_len,
		size_of_iov, &subcmd_len);
	if (unlikely(ret)) {
		WL_ERR(("nan_sub_cmd check failed\n"));
		goto fail;
	}

	sub_cmd->id = htod16(WL_NAN_CMD_RANGE_CANCEL);
	sub_cmd->len = sizeof(sub_cmd->u.options) + size_of_iov;
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);

	/* Reduce the iov_len size by subcmd_len */
	nan_iov_data->nan_iov_len -= subcmd_len;
	nan_iov_end = nan_iov_data->nan_iov_len;
	nan_buf_size = (nan_iov_start - nan_iov_end);

	if (size_of_iov >= sizeof(rng_cncl)) {
		(void)memcpy_s(sub_cmd->data, nan_iov_data->nan_iov_len,
			&rng_cncl, size_of_iov);
	} else {
		(void)memcpy_s(sub_cmd->data, nan_iov_data->nan_iov_len,
			&range_id, size_of_iov);
	}

	nan_buf->is_set = true;
	nan_buf->count++;
	memset_s(resp_buf, sizeof(resp_buf),
			0, sizeof(resp_buf));
	ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, nan_buf_size, status,
			(void*)resp_buf, NAN_IOCTL_BUF_SIZE);
	if (unlikely(ret) || unlikely(*status)) {
		WL_ERR(("Range ID %d cancel failed ret %d status %d \n", range_id, ret, *status));
		goto fail;
	}
	WL_MEM(("Range cancel with Range ID [%d] successfull\n", range_id));
fail:
	if (nan_buf) {
		MFREE(cfg->osh, nan_buf, NAN_IOCTL_BUF_SIZE);
	}
	if (nan_iov_data) {
		MFREE(cfg->osh, nan_iov_data, sizeof(*nan_iov_data));
	}
	NAN_DBG_EXIT();
	return ret;
}

#ifdef WL_NAN_DISC_CACHE
static int
wl_cfgnan_cache_svc_info(struct bcm_cfg80211 *cfg,
	nan_discover_cmd_data_t *cmd_data, uint16 cmd_id, bool update)
{
	int ret = BCME_OK;
	int i;
	nan_svc_info_t *svc_info;
	uint8 svc_id = (cmd_id == WL_NAN_CMD_SD_SUBSCRIBE) ? cmd_data->sub_id :
		cmd_data->pub_id;

	for (i = 0; i < NAN_MAX_SVC_INST; i++) {
		if (update) {
			if (cfg->svc_info[i].svc_id == svc_id) {
				svc_info = &cfg->svc_info[i];
				break;
			} else {
				continue;
			}
		}
		if (!cfg->svc_info[i].svc_id) {
			svc_info = &cfg->svc_info[i];
			break;
		}
	}
	if (i == NAN_MAX_SVC_INST) {
		WL_ERR(("%s:cannot accomodate ranging session\n", __FUNCTION__));
		ret = BCME_NORESOURCE;
		goto fail;
	}
	if (cmd_data->sde_control_flag & NAN_SDE_CF_RANGING_REQUIRED) {
		WL_TRACE(("%s: updating ranging info, enabling", __FUNCTION__));
		svc_info->status = 1;
		svc_info->ranging_interval = cmd_data->ranging_intvl_msec;
		svc_info->ranging_ind = cmd_data->ranging_indication;
		svc_info->ingress_limit = cmd_data->ingress_limit;
		svc_info->egress_limit = cmd_data->egress_limit;
		svc_info->ranging_required = 1;
	} else {
		WL_TRACE(("%s: updating ranging info, disabling", __FUNCTION__));
		svc_info->status = 0;
		svc_info->ranging_interval = 0;
		svc_info->ranging_ind = 0;
		svc_info->ingress_limit = 0;
		svc_info->egress_limit = 0;
		svc_info->ranging_required = 0;
	}

	/* Reset Range status flags on svc creation/update */
	svc_info->svc_range_status = 0;
	svc_info->flags = cmd_data->flags;

	if (cmd_id == WL_NAN_CMD_SD_SUBSCRIBE) {
		svc_info->svc_id = cmd_data->sub_id;
		if ((cmd_data->flags & WL_NAN_SUB_ACTIVE) &&
			(cmd_data->tx_match.dlen)) {
			ret = memcpy_s(svc_info->tx_match_filter, sizeof(svc_info->tx_match_filter),
				cmd_data->tx_match.data, cmd_data->tx_match.dlen);
			if (ret != BCME_OK) {
				WL_ERR(("Failed to copy tx match filter data\n"));
				goto fail;
			}
			svc_info->tx_match_filter_len = cmd_data->tx_match.dlen;
		}
	} else {
		svc_info->svc_id = cmd_data->pub_id;
	}
	ret = memcpy_s(svc_info->svc_hash, sizeof(svc_info->svc_hash),
			cmd_data->svc_hash.data, WL_NAN_SVC_HASH_LEN);
	if (ret != BCME_OK) {
		WL_ERR(("Failed to copy svc hash\n"));
	}
fail:
	return ret;

}

static bool
wl_cfgnan_clear_svc_from_ranging_inst(struct bcm_cfg80211 *cfg,
	nan_ranging_inst_t *ranging_inst, nan_svc_info_t *svc)
{
	int i = 0;
	bool cleared = FALSE;

	if (svc && ranging_inst->in_use) {
		for (i = 0; i < MAX_SUBSCRIBES; i++) {
			if (svc == ranging_inst->svc_idx[i]) {
				ranging_inst->num_svc_ctx--;
				ranging_inst->svc_idx[i] = NULL;
				cleared = TRUE;
				/*
				 * This list is maintained dupes free,
				 * hence can break
				 */
				break;
			}
		}
	}
	return cleared;
}

static int
wl_cfgnan_clear_svc_from_all_ranging_inst(struct bcm_cfg80211 *cfg, uint8 svc_id)
{
	nan_ranging_inst_t *ranging_inst;
	int i = 0;
	int ret = BCME_OK;

	nan_svc_info_t *svc = wl_cfgnan_get_svc_inst(cfg, svc_id, 0);
	if (!svc) {
		WL_ERR(("\n svc not found \n"));
		ret = BCME_NOTFOUND;
		goto done;
	}
	for (i = 0; i < NAN_MAX_RANGING_INST; i++) {
		ranging_inst = &(cfg->nan_ranging_info[i]);
		wl_cfgnan_clear_svc_from_ranging_inst(cfg, ranging_inst, svc);
	}

done:
	return ret;
}

static int
wl_cfgnan_ranging_clear_publish(struct bcm_cfg80211 *cfg,
	struct ether_addr *peer, uint8 svc_id)
{
	nan_ranging_inst_t *ranging_inst = NULL;
	nan_svc_info_t *svc = NULL;
	bool cleared = FALSE;
	int ret = BCME_OK;

	ranging_inst = wl_cfgnan_check_for_ranging(cfg, peer);
	if (!ranging_inst || !ranging_inst->in_use) {
		goto done;
	}

	WL_INFORM_MEM(("Check clear Ranging for pub update, sub id = %d,"
		" range_id = %d, peer addr = " MACDBG " \n", svc_id,
		ranging_inst->range_id, MAC2STRDBG(peer)));
	svc = wl_cfgnan_get_svc_inst(cfg, svc_id, 0);
	if (!svc) {
		WL_ERR(("\n svc not found, svc_id = %d\n", svc_id));
		ret = BCME_NOTFOUND;
		goto done;
	}

	cleared = wl_cfgnan_clear_svc_from_ranging_inst(cfg, ranging_inst, svc);
	if (!cleared) {
		/* Only if this svc was cleared, any update needed */
		ret = BCME_NOTFOUND;
		goto done;
	}

	wl_cfgnan_terminate_ranging_session(cfg, ranging_inst);

done:
	return ret;
}

#ifdef RTT_SUPPORT
/* API to terminate/clear all directed nan-rtt sessions.
* Can be called from framework RTT stop context
*/
int
wl_cfgnan_terminate_directed_rtt_sessions(struct net_device *ndev,
	struct bcm_cfg80211 *cfg)
{
	nan_ranging_inst_t *ranging_inst;
	int i, ret = BCME_OK;
	uint32 status;

	for (i = 0; i < NAN_MAX_RANGING_INST; i++) {
		ranging_inst = &cfg->nan_ranging_info[i];
		if (ranging_inst->range_id && ranging_inst->range_type == RTT_TYPE_NAN_DIRECTED) {
			if (ranging_inst->range_status == NAN_RANGING_IN_PROGRESS) {
				ret =  wl_cfgnan_cancel_ranging(ndev, cfg, ranging_inst->range_id,
					NAN_RNG_TERM_FLAG_IMMEDIATE, &status);
				if (unlikely(ret) || unlikely(status)) {
					WL_ERR(("nan range cancel failed ret = %d status = %d\n",
						ret, status));
				}
			}
			wl_cfgnan_reset_geofence_ranging(cfg, ranging_inst,
				RTT_SHCED_HOST_DIRECTED_TERM);
		}
	}
	return ret;
}
#endif /* RTT_SUPPORT */

/*
 * suspend ongoing geofence ranging session
 * with a peer if on-going ranging is with given peer
 * If peer NULL,
 * Suspend on-going ranging blindly
 * Do nothing on:
 * If ranging is not in progress
 * If ranging in progress but not with given peer
 */
int
wl_cfgnan_suspend_geofence_rng_session(struct net_device *ndev,
	struct ether_addr *peer, int suspend_reason, u8 cancel_flags)
{
	int ret = BCME_OK;
	uint32 status;
	nan_ranging_inst_t *ranging_inst = NULL;
	struct ether_addr* peer_addr = NULL;
	struct bcm_cfg80211 *cfg = wl_get_cfg(ndev);
#ifdef RTT_SUPPORT
	dhd_pub_t *dhd = (struct dhd_pub *)(cfg->pub);
	rtt_geofence_target_info_t *geofence_target_info;

	geofence_target_info = dhd_rtt_get_geofence_current_target(dhd);
	if (!geofence_target_info) {
		WL_DBG(("No Geofencing Targets, suspend req dropped\n"));
		goto exit;
	}
	peer_addr = &geofence_target_info->peer_addr;

	ranging_inst = wl_cfgnan_check_for_ranging(cfg, peer_addr);
	if (dhd_rtt_get_geofence_rtt_state(dhd) == FALSE) {
		WL_DBG(("Geofencing Ranging not in progress, suspend req dropped\n"));
		goto exit;
	}

	if (peer && memcmp(peer_addr, peer, ETHER_ADDR_LEN)) {
		if (suspend_reason == RTT_GEO_SUSPN_HOST_NDP_TRIGGER ||
			suspend_reason == RTT_GEO_SUSPN_PEER_NDP_TRIGGER) {
			/* NDP and Ranging can coexist with different Peers */
			WL_DBG(("Geofencing Ranging not in progress with given peer,"
				" suspend req dropped\n"));
			goto exit;
		}
	}
#endif /* RTT_SUPPORT */

	ASSERT((ranging_inst != NULL));
	if (ranging_inst) {
		if (ranging_inst->range_status != NAN_RANGING_IN_PROGRESS) {
			WL_DBG(("Ranging Inst with peer not in progress, "
			" suspend req dropped\n"));
			goto exit;
		}
		cancel_flags |= NAN_RNG_TERM_FLAG_IMMEDIATE;
		ret =  wl_cfgnan_cancel_ranging(ndev, cfg,
				ranging_inst->range_id, cancel_flags, &status);
		if (unlikely(ret) || unlikely(status)) {
			WL_ERR(("Geofence Range suspended failed, err = %d, status = %d,"
				" range_id = %d, suspend_reason = %d, " MACDBG " \n",
				ret, status, ranging_inst->range_id,
				suspend_reason, MAC2STRDBG(peer_addr)));
		}
		ranging_inst->range_status = NAN_RANGING_REQUIRED;
		WL_INFORM_MEM(("Geofence Range suspended, range_id = %d,"
			" suspend_reason = %d, " MACDBG " \n", ranging_inst->range_id,
			suspend_reason, MAC2STRDBG(peer_addr)));
#ifdef RTT_SUPPORT
		/* Set geofence RTT in progress state to false */
		dhd_rtt_set_geofence_rtt_state(dhd, FALSE);
#endif /* RTT_SUPPORT */
	}

exit:
	/* Post pending discovery results */
	if (ranging_inst &&
		((suspend_reason == RTT_GEO_SUSPN_HOST_NDP_TRIGGER) ||
		(suspend_reason == RTT_GEO_SUSPN_PEER_NDP_TRIGGER))) {
		wl_cfgnan_disc_result_on_geofence_cancel(cfg, ranging_inst);
	}

	return ret;
}

static void
wl_cfgnan_clear_svc_cache(struct bcm_cfg80211 *cfg,
		wl_nan_instance_id svc_id)
{
	nan_svc_info_t *svc;
	svc = wl_cfgnan_get_svc_inst(cfg, svc_id, 0);
	if (svc) {
		WL_DBG(("clearing cached svc info for svc id %d\n", svc_id));
		memset(svc, 0, sizeof(*svc));
	}
}

/*
 * Terminate given ranging instance
 * if no pending ranging sub service
 */
static void
wl_cfgnan_terminate_ranging_session(struct bcm_cfg80211 *cfg,
	nan_ranging_inst_t *ranging_inst)
{
	int ret = BCME_OK;
	uint32 status;
#ifdef RTT_SUPPORT
	rtt_geofence_target_info_t* geofence_target = NULL;
	dhd_pub_t *dhd = (struct dhd_pub *)(cfg->pub);
	int8 index;
#endif /* RTT_SUPPORT */

	if (ranging_inst->range_id == 0) {
		/* Make sure, range inst is valid in caller */
		return;
	}

	if (ranging_inst->num_svc_ctx != 0) {
		/*
		 * Make sure to remove all svc_insts for range_inst
		 * in order to cancel ranging and remove target in caller
		 */
		return;
	}

	/* Cancel Ranging if in progress for rang_inst */
	if (ranging_inst->range_status == NAN_RANGING_IN_PROGRESS) {
		ret =  wl_cfgnan_cancel_ranging(bcmcfg_to_prmry_ndev(cfg),
				cfg, ranging_inst->range_id,
				NAN_RNG_TERM_FLAG_IMMEDIATE, &status);
		if (unlikely(ret) || unlikely(status)) {
			WL_ERR(("%s:nan range cancel failed ret = %d status = %d\n",
				__FUNCTION__, ret, status));
		} else {
			WL_DBG(("Range cancelled \n"));
			/* Set geofence RTT in progress state to false */
#ifdef RTT_SUPPORT
			dhd_rtt_set_geofence_rtt_state(dhd, FALSE);
#endif /* RTT_SUPPORT */
		}
	}

#ifdef RTT_SUPPORT
	geofence_target = dhd_rtt_get_geofence_target(dhd,
			&ranging_inst->peer_addr, &index);
	if (geofence_target) {
		dhd_rtt_remove_geofence_target(dhd, &geofence_target->peer_addr);
		WL_INFORM_MEM(("Removing Ranging Instance " MACDBG "\n",
			MAC2STRDBG(&(ranging_inst->peer_addr))));
		bzero(ranging_inst, sizeof(nan_ranging_inst_t));
	}
#endif /* RTT_SUPPORT */
}

/*
 * Terminate all ranging sessions
 * with no pending ranging sub service
 */
static void
wl_cfgnan_terminate_all_obsolete_ranging_sessions(
	struct bcm_cfg80211 *cfg)
{
	/* cancel all related ranging instances */
	uint8 i = 0;
	nan_ranging_inst_t *ranging_inst = NULL;

	for (i = 0; i < NAN_MAX_RANGING_INST; i++) {
		ranging_inst = &cfg->nan_ranging_info[i];
		if (ranging_inst->in_use) {
			wl_cfgnan_terminate_ranging_session(cfg, ranging_inst);
		}
	}

	return;
}

/*
 * Store svc_ctx for processing during RNG_RPT
 * Return BCME_OK only when svc is added
 */
static int
wl_cfgnan_update_ranging_svc_inst(nan_ranging_inst_t *ranging_inst,
	nan_svc_info_t *svc)
{
	int ret = BCME_OK;
	int i = 0;

	for (i = 0; i < MAX_SUBSCRIBES; i++) {
		if (ranging_inst->svc_idx[i] == svc) {
			WL_DBG(("SVC Ctx for ranging already present, "
			" Duplication not supported: sub_id: %d\n", svc->svc_id));
			ret = BCME_UNSUPPORTED;
			goto done;
		}
	}
	for (i = 0; i < MAX_SUBSCRIBES; i++) {
		if (ranging_inst->svc_idx[i]) {
			continue;
		} else {
			WL_DBG(("Adding SVC Ctx for ranging..svc_id %d\n", svc->svc_id));
			ranging_inst->svc_idx[i] = svc;
			ranging_inst->num_svc_ctx++;
			ret = BCME_OK;
			goto done;
		}
	}
	if (i == MAX_SUBSCRIBES) {
		WL_ERR(("wl_cfgnan_update_ranging_svc_inst: "
			"No resource to hold Ref SVC ctx..svc_id %d\n", svc->svc_id));
		ret = BCME_NORESOURCE;
		goto done;
	}
done:
	return ret;
}

#ifdef RTT_SUPPORT
int
wl_cfgnan_trigger_geofencing_ranging(struct net_device *dev,
		struct ether_addr *peer_addr)
{
	int ret = BCME_OK;
	int err_at = 0;
	struct bcm_cfg80211 *cfg = wl_get_cfg(dev);
	int8 index = -1;
	dhd_pub_t *dhd = (struct dhd_pub *)(cfg->pub);
	rtt_geofence_target_info_t* geofence_target;
	nan_ranging_inst_t *ranging_inst;
	ranging_inst = wl_cfgnan_check_for_ranging(cfg, peer_addr);

	if (!ranging_inst) {
		WL_INFORM_MEM(("Ranging Entry for peer:" MACDBG ", not found\n",
			MAC2STRDBG(peer_addr)));
		ASSERT(0);
		/* Ranging inst should have been added before adding target */
		dhd_rtt_remove_geofence_target(dhd, peer_addr);
		ret = BCME_ERROR;
		err_at = 1;
		goto exit;
	}

	ASSERT(ranging_inst->range_status !=
		NAN_RANGING_IN_PROGRESS);

	if (ranging_inst->range_status !=
			NAN_RANGING_IN_PROGRESS) {
		WL_DBG(("Trigger range request with first svc in svc list of range inst\n"));
		ret = wl_cfgnan_trigger_ranging(bcmcfg_to_prmry_ndev(cfg),
				cfg, ranging_inst, ranging_inst->svc_idx[0],
				NAN_RANGE_REQ_CMD, TRUE);
		if (ret != BCME_OK) {
			/* Unsupported is for already ranging session for peer */
			if (ret == BCME_BUSY) {
				/* TODO: Attempt again over a timer */
				err_at = 2;
			} else {
				/* Remove target and clean ranging inst */
				geofence_target = dhd_rtt_get_geofence_target(dhd,
						&ranging_inst->peer_addr, &index);
				if (geofence_target) {
					dhd_rtt_remove_geofence_target(dhd,
						&geofence_target->peer_addr);
				}
				bzero(ranging_inst, sizeof(nan_ranging_inst_t));
				err_at = 3;
				goto exit;
			}
		}
	} else {
		/* already in progress..This should not happen */
		ASSERT(0);
		ret = BCME_ERROR;
		err_at = 4;
		goto exit;
	}

exit:
	if (ret) {
		WL_ERR(("wl_cfgnan_trigger_geofencing_ranging: Failed to "
			"trigger ranging, peer: " MACDBG " ret"
			" = (%d), err_at = %d\n", MAC2STRDBG(peer_addr),
			ret, err_at));
	}
	return ret;
}
#endif /* RTT_SUPPORT */

static int
wl_cfgnan_check_disc_result_for_ranging(struct bcm_cfg80211 *cfg,
		nan_event_data_t* nan_event_data)
{
	nan_svc_info_t *svc;
	int ret = BCME_OK;
#ifdef RTT_SUPPORT
	rtt_geofence_target_info_t geofence_target;
	dhd_pub_t *dhd = (struct dhd_pub *)(cfg->pub);
	uint8 index;
#endif /* RTT_SUPPORT */
	bool add_target;

	svc = wl_cfgnan_get_svc_inst(cfg, nan_event_data->sub_id, 0);

	if (svc && svc->ranging_required) {
		nan_ranging_inst_t *ranging_inst;
		ranging_inst = wl_cfgnan_get_ranging_inst(cfg,
				&nan_event_data->remote_nmi,
				NAN_RANGING_ROLE_INITIATOR);
		if (!ranging_inst) {
			ret = BCME_NORESOURCE;
			goto exit;
		}
		ASSERT(ranging_inst->range_role != NAN_RANGING_ROLE_INVALID);

		/* For responder role, range state should be in progress only */
		ASSERT(ranging_inst->range_role == NAN_RANGING_ROLE_INITIATOR ||
			ranging_inst->range_status == NAN_RANGING_IN_PROGRESS);

		/*
		 * On rec disc result with ranging required, add target, if
		 * ranging role is responder (range state has to be in prog always)
		 * Or ranging role is initiator and ranging is not already in prog
		 */
		add_target = ((ranging_inst->range_role ==  NAN_RANGING_ROLE_RESPONDER) ||
			((ranging_inst->range_role ==  NAN_RANGING_ROLE_INITIATOR) &&
			(ranging_inst->range_status != NAN_RANGING_IN_PROGRESS)));
		if (add_target) {
			WL_DBG(("Add Range request to geofence target list\n"));
#ifdef RTT_SUPPORT
			memcpy(&geofence_target.peer_addr, &nan_event_data->remote_nmi,
					ETHER_ADDR_LEN);
			/* check if target is already added */
			if (!dhd_rtt_get_geofence_target(dhd, &nan_event_data->remote_nmi, &index))
			{
				ret = dhd_rtt_add_geofence_target(dhd, &geofence_target);
				if (unlikely(ret)) {
					WL_ERR(("Failed to add geofence Tgt, ret = (%d)\n", ret));
					bzero(ranging_inst, sizeof(*ranging_inst));
					goto exit;
				} else {
					WL_INFORM_MEM(("Geofence Tgt Added:" MACDBG " sub_id:%d\n",
						MAC2STRDBG(&geofence_target.peer_addr),
						svc->svc_id));
				}
				ranging_inst->range_type = RTT_TYPE_NAN_GEOFENCE;
			}
#endif /* RTT_SUPPORT */
			if (wl_cfgnan_update_ranging_svc_inst(ranging_inst, svc)
					!= BCME_OK) {
					goto exit;
			}
#ifdef RTT_SUPPORT
			if (ranging_inst->range_role == NAN_RANGING_ROLE_RESPONDER) {
				/* Adding RTT target while responder, leads to role concurrency */
				dhd_rtt_set_role_concurrency_state(dhd, TRUE);
			}
			else {
				/* Trigger/Reset geofence RTT */
				wl_cfgnan_reset_geofence_ranging(cfg, ranging_inst,
					RTT_SCHED_SUB_MATCH);
			}
#endif /* RTT_SUPPORT */
		} else {
			/* Target already added, check & add svc_inst ref to rang_inst */
			wl_cfgnan_update_ranging_svc_inst(ranging_inst, svc);
		}
		/* Disc event will be given on receving range_rpt event */
		WL_TRACE(("Disc event will given when Range RPT event is recvd"));
	} else {
		ret = BCME_UNSUPPORTED;
	}

exit:
	return ret;
}

bool
wl_cfgnan_ranging_allowed(struct bcm_cfg80211 *cfg)
{
	int i = 0;
	uint8 rng_progress_count = 0;
	nan_ranging_inst_t *ranging_inst = NULL;

	for (i =  0; i < NAN_MAX_RANGING_INST; i++) {
		ranging_inst = &cfg->nan_ranging_info[i];
		if (ranging_inst->range_status == NAN_RANGING_IN_PROGRESS) {
			rng_progress_count++;
		}
	}

	ASSERT(rng_progress_count <= NAN_MAX_RANGING_SSN_ALLOWED);
	if (rng_progress_count == NAN_MAX_RANGING_SSN_ALLOWED) {
		return FALSE;
	}
	return TRUE;
}

uint8
wl_cfgnan_cancel_rng_responders(struct net_device *ndev,
	struct bcm_cfg80211 *cfg)
{
	int i = 0;
	uint8 num_resp_cancelled = 0;
	int status, ret;
	nan_ranging_inst_t *ranging_inst = NULL;

	for (i =  0; i < NAN_MAX_RANGING_INST; i++) {
		ranging_inst = &cfg->nan_ranging_info[i];
		if (ranging_inst->range_status == NAN_RANGING_IN_PROGRESS &&
			ranging_inst->range_role == NAN_RANGING_ROLE_RESPONDER) {
			num_resp_cancelled++;
			WL_ERR((" Cancelling responder\n"));
			ret = wl_cfgnan_cancel_ranging(bcmcfg_to_prmry_ndev(cfg), cfg,
				ranging_inst->range_id, NAN_RNG_TERM_FLAG_IMMEDIATE, &status);
			if (unlikely(ret) || unlikely(status)) {
				WL_ERR(("wl_cfgnan_cancel_rng_responders: Failed to cancel"
					" existing ranging, ret = (%d)\n", ret));
			}
			WL_INFORM_MEM(("Removing Ranging Instance " MACDBG "\n",
				MAC2STRDBG(&(ranging_inst->peer_addr))));
			bzero(ranging_inst, sizeof(*ranging_inst));
		}
	}
	return num_resp_cancelled;
}

#ifdef RTT_SUPPORT
/* ranging reqeust event handler */
static int
wl_cfgnan_handle_ranging_ind(struct bcm_cfg80211 *cfg,
		wl_nan_ev_rng_req_ind_t *rng_ind)
{
	int ret = BCME_OK;
	nan_ranging_inst_t *ranging_inst = NULL;
	uint32 status;
	uint8 cancel_flags = 0;
	bool accept = TRUE;
	nan_ranging_inst_t tmp_rng_inst;
	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);

	WL_DBG(("Trigger range response\n"));

	/* check if we are already having any ranging session with peer.
	* If so below are the policies
	* If we are already a Geofence Initiator or responder w.r.t the peer
	* then silently teardown the current session and accept the REQ.
	* If we are in direct rtt initiator role then reject.
	*/
	ranging_inst = wl_cfgnan_check_for_ranging(cfg, &(rng_ind->peer_m_addr));
	if (ranging_inst) {
		if (ranging_inst->range_type == RTT_TYPE_NAN_GEOFENCE ||
			ranging_inst->range_role == NAN_RANGING_ROLE_RESPONDER) {
			WL_INFORM_MEM(("Already responder/geofence for the Peer, cancel current"
				" ssn and accept new one, range_type = %d, role = %d\n",
				ranging_inst->range_type, ranging_inst->range_role));
			cancel_flags = NAN_RNG_TERM_FLAG_IMMEDIATE |
				NAN_RNG_TERM_FLAG_SILIENT_TEARDOWN;

			if (ranging_inst->range_type == RTT_TYPE_NAN_GEOFENCE &&
				ranging_inst->range_role == NAN_RANGING_ROLE_INITIATOR) {
				wl_cfgnan_suspend_geofence_rng_session(ndev,
					&(rng_ind->peer_m_addr), RTT_GEO_SUSPN_PEER_RTT_TRIGGER,
					cancel_flags);
			} else {
				ret = wl_cfgnan_cancel_ranging(ndev, cfg,
					ranging_inst->range_id, cancel_flags, &status);
				if (unlikely(ret)) {
					WL_ERR(("wl_cfgnan_handle_ranging_ind: Failed to cancel"
						" existing ranging, ret = (%d)\n", ret));
					goto done;
				}
			}
			ranging_inst->range_status = NAN_RANGING_REQUIRED;
			ranging_inst->range_role = NAN_RANGING_ROLE_RESPONDER;
			ranging_inst->range_type = 0;
		} else {
			WL_ERR(("Reject the RNG_REQ_IND in direct rtt initiator role\n"));
			ret = BCME_BUSY;
			goto done;
		}
	} else {
		/* Check if new Ranging session is allowed */
		if (!wl_cfgnan_ranging_allowed(cfg)) {
			WL_ERR(("Cannot allow more ranging sessions \n"));
			ret = BCME_NORESOURCE;
			goto done;
		}

		ranging_inst = wl_cfgnan_get_ranging_inst(cfg, &rng_ind->peer_m_addr,
				NAN_RANGING_ROLE_RESPONDER);
		if (!ranging_inst) {
			WL_ERR(("Failed to create ranging instance \n"));
			ASSERT(0);
			ret = BCME_NORESOURCE;
			goto done;
		}
	}

done:
	if (ret != BCME_OK) {
		/* reject the REQ using temp ranging instance */
		bzero(&tmp_rng_inst, sizeof(tmp_rng_inst));
		ranging_inst = &tmp_rng_inst;
		(void)memcpy_s(&tmp_rng_inst.peer_addr, ETHER_ADDR_LEN,
				&rng_ind->peer_m_addr, ETHER_ADDR_LEN);
		accept = FALSE;
	}

	ranging_inst->range_id = rng_ind->rng_id;

	WL_DBG(("Trigger Ranging at Responder\n"));
	ret = wl_cfgnan_trigger_ranging(ndev, cfg, ranging_inst,
		NULL, NAN_RANGE_REQ_EVNT, accept);
	if (unlikely(ret) || !accept) {
		WL_ERR(("Failed to handle range request, ret = (%d) accept %d\n",
			ret, accept));
		bzero(ranging_inst, sizeof(*ranging_inst));
	}

	return ret;
}
#endif /* RTT_SUPPORT */
/* ranging quest and response iovar handler */
int
wl_cfgnan_trigger_ranging(struct net_device *ndev, struct bcm_cfg80211 *cfg,
		void *ranging_ctxt, nan_svc_info_t *svc,
		uint8 range_cmd, bool accept_req)
{
	s32 ret = BCME_OK;
	bcm_iov_batch_buf_t *nan_buf = NULL;
	wl_nan_range_req_t *range_req = NULL;
	wl_nan_range_resp_t *range_resp = NULL;
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	uint16 nan_buf_size = NAN_IOCTL_BUF_SIZE;
	uint32 status;
	uint8 resp_buf[NAN_IOCTL_BUF_SIZE_MED];
	nan_ranging_inst_t *ranging_inst = (nan_ranging_inst_t *)ranging_ctxt;
	nan_avail_cmd_data cmd_data;

	NAN_DBG_ENTER();

	memset_s(&cmd_data, sizeof(cmd_data),
			0, sizeof(cmd_data));
	ret = memcpy_s(&cmd_data.peer_nmi, ETHER_ADDR_LEN,
			&ranging_inst->peer_addr, ETHER_ADDR_LEN);
	if (ret != BCME_OK) {
		WL_ERR(("Failed to copy ranging peer addr\n"));
		goto fail;
	}

	cmd_data.avail_period = NAN_RANGING_PERIOD;
	ret = wl_cfgnan_set_nan_avail(bcmcfg_to_prmry_ndev(cfg),
			cfg, &cmd_data, WL_AVAIL_LOCAL);
	if (ret != BCME_OK) {
		WL_ERR(("Failed to set avail value with type [WL_AVAIL_LOCAL]\n"));
		goto fail;
	}

	ret = wl_cfgnan_set_nan_avail(bcmcfg_to_prmry_ndev(cfg),
			cfg, &cmd_data, WL_AVAIL_RANGING);
	if (unlikely(ret)) {
		WL_ERR(("Failed to set avail value with type [WL_AVAIL_RANGING]\n"));
		goto fail;
	}

	nan_buf = MALLOCZ(cfg->osh, nan_buf_size);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_buf_size -= OFFSETOF(bcm_iov_batch_buf_t, cmds[0]);

	sub_cmd = (bcm_iov_batch_subcmd_t*)(&nan_buf->cmds[0]);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
	if (range_cmd == NAN_RANGE_REQ_CMD) {
		sub_cmd->id = htod16(WL_NAN_CMD_RANGE_REQUEST);
		sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(wl_nan_range_req_t);
		range_req = (wl_nan_range_req_t *)(sub_cmd->data);
		/* ranging config */
		range_req->peer = ranging_inst->peer_addr;
		if (svc) {
			range_req->interval = svc->ranging_interval;
			/* Limits are in cm from host */
			range_req->ingress = svc->ingress_limit;
			range_req->egress = svc->egress_limit;
		}
		range_req->indication = NAN_RANGING_INDICATE_CONTINUOUS_MASK;
	} else {
		/* range response config */
		sub_cmd->id = htod16(WL_NAN_CMD_RANGE_RESPONSE);
		sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(wl_nan_range_resp_t);
		range_resp = (wl_nan_range_resp_t *)(sub_cmd->data);
		range_resp->range_id = ranging_inst->range_id;
		range_resp->indication = NAN_RANGING_INDICATE_CONTINUOUS_MASK;
		if (accept_req) {
			range_resp->status = NAN_RNG_REQ_ACCEPTED_BY_HOST;
		} else {
			range_resp->status = NAN_RNG_REQ_REJECTED_BY_HOST;
		}
		nan_buf->is_set = true;
	}

	nan_buf_size -= (sub_cmd->len +
			OFFSETOF(bcm_iov_batch_subcmd_t, u.options));
	nan_buf->count++;

	memset_s(resp_buf, sizeof(resp_buf), 0, sizeof(resp_buf));
	ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, nan_buf_size,
			&status,
			(void*)resp_buf, NAN_IOCTL_BUF_SIZE);
	if (unlikely(ret) || unlikely(status)) {
		WL_ERR(("nan ranging failed ret = %d status = %d\n",
				ret, status));
		ret = (ret == BCME_OK) ? status : ret;
		goto fail;
	}
	WL_TRACE(("nan ranging trigger successful\n"));
	if (range_cmd == NAN_RANGE_REQ_CMD) {
		WL_MEM(("Ranging Req Triggered"
			" peer: " MACDBG ", ind : %d, ingress : %d, egress : %d\n",
			MAC2STRDBG(&ranging_inst->peer_addr), range_req->indication,
			range_req->ingress, range_req->egress));
	} else {
		WL_MEM(("Ranging Resp Triggered"
			" peer: " MACDBG ", ind : %d, ingress : %d, egress : %d\n",
			MAC2STRDBG(&ranging_inst->peer_addr), range_resp->indication,
			range_resp->ingress, range_resp->egress));
	}

	/* check the response buff for request */
	if (range_cmd == NAN_RANGE_REQ_CMD) {
		ret = process_resp_buf(resp_buf + WL_NAN_OBUF_DATA_OFFSET,
				&ranging_inst->range_id, WL_NAN_CMD_RANGE_REQUEST);
		WL_INFORM_MEM(("ranging instance returned %d\n", ranging_inst->range_id));
	}
	/* Preventing continuous range requests */
	ranging_inst->range_status = NAN_RANGING_IN_PROGRESS;

fail:
	if (nan_buf) {
		MFREE(cfg->osh, nan_buf, NAN_IOCTL_BUF_SIZE);
	}

	NAN_DBG_EXIT();
	return ret;
}
#endif /* WL_NAN_DISC_CACHE */

static void *wl_nan_bloom_alloc(void *ctx, uint size)
{
	uint8 *buf;
	BCM_REFERENCE(ctx);

	buf = kmalloc(size, GFP_KERNEL);
	if (!buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		buf = NULL;
	}
	return buf;
}

static void wl_nan_bloom_free(void *ctx, void *buf, uint size)
{
	BCM_REFERENCE(ctx);
	BCM_REFERENCE(size);
	if (buf) {
		kfree(buf);
	}
}

static uint wl_nan_hash(void *ctx, uint index, const uint8 *input, uint input_len)
{
	uint8* filter_idx = (uint8*)ctx;
	uint8 i = (*filter_idx * WL_NAN_HASHES_PER_BLOOM) + (uint8)index;
	uint b = 0;

	/* Steps 1 and 2 as explained in Section 6.2 */
	/* Concatenate index to input and run CRC32 by calling hndcrc32 twice */
	GCC_DIAGNOSTIC_PUSH_SUPPRESS_CAST();
	b = hndcrc32(&i, sizeof(uint8), CRC32_INIT_VALUE);
	b = hndcrc32((uint8*)input, input_len, b);
	GCC_DIAGNOSTIC_POP();
	/* Obtain the last 2 bytes of the CRC32 output */
	b &= NAN_BLOOM_CRC32_MASK;

	/* Step 3 is completed by bcmbloom functions */
	return b;
}

static int wl_nan_bloom_create(bcm_bloom_filter_t **bp, uint *idx, uint size)
{
	uint i;
	int err;

	err = bcm_bloom_create(wl_nan_bloom_alloc, wl_nan_bloom_free,
			idx, WL_NAN_HASHES_PER_BLOOM, size, bp);
	if (err != BCME_OK) {
		goto exit;
	}

	/* Populate bloom filter with hash functions */
	for (i = 0; i < WL_NAN_HASHES_PER_BLOOM; i++) {
		err = bcm_bloom_add_hash(*bp, wl_nan_hash, &i);
		if (err) {
			WL_ERR(("bcm_bloom_add_hash failed\n"));
			goto exit;
		}
	}
exit:
	return err;
}

static int
wl_cfgnan_sd_params_handler(struct net_device *ndev,
	nan_discover_cmd_data_t *cmd_data, uint16 cmd_id,
	void *p_buf, uint16 *nan_buf_size)
{
	s32 ret = BCME_OK;
	uint8 *pxtlv, *srf = NULL, *srf_mac = NULL, *srftmp = NULL;
	uint16 buflen_avail;
	bcm_iov_batch_subcmd_t *sub_cmd = (bcm_iov_batch_subcmd_t*)(p_buf);
	wl_nan_sd_params_t *sd_params = (wl_nan_sd_params_t *)sub_cmd->data;
	uint16 srf_size = 0;
	uint bloom_size, a;
	bcm_bloom_filter_t *bp = NULL;
	/* Bloom filter index default, indicates it has not been set */
	uint bloom_idx = 0xFFFFFFFF;
	uint16 bloom_len = NAN_BLOOM_LENGTH_DEFAULT;
	/* srf_ctrl_size = bloom_len + src_control field */
	uint16 srf_ctrl_size = bloom_len + 1;

	dhd_pub_t *dhdp = wl_cfg80211_get_dhdp(ndev);
	struct bcm_cfg80211 *cfg = wl_get_cfg(ndev);
	BCM_REFERENCE(cfg);

	NAN_DBG_ENTER();

	if (cmd_data->period) {
		sd_params->awake_dw = cmd_data->period;
	}
	sd_params->period = 1;

	if (cmd_data->ttl) {
		sd_params->ttl = cmd_data->ttl;
	} else {
		sd_params->ttl = WL_NAN_TTL_UNTIL_CANCEL;
	}

	sd_params->flags = 0;
	sd_params->flags = cmd_data->flags;

	/* Nan Service Based event suppression Flags */
	if (cmd_data->recv_ind_flag) {
		/* BIT0 - If set, host wont rec event "terminated" */
		if (CHECK_BIT(cmd_data->recv_ind_flag, WL_NAN_EVENT_SUPPRESS_TERMINATE_BIT)) {
			sd_params->flags |= WL_NAN_SVC_CTRL_SUPPRESS_EVT_TERMINATED;
		}

		/* BIT1 - If set, host wont receive match expiry evt */
		/* TODO: Exp not yet supported */
		if (CHECK_BIT(cmd_data->recv_ind_flag, WL_NAN_EVENT_SUPPRESS_MATCH_EXP_BIT)) {
			WL_DBG(("Need to add match expiry event\n"));
		}
		/* BIT2 - If set, host wont rec event "receive"  */
		if (CHECK_BIT(cmd_data->recv_ind_flag, WL_NAN_EVENT_SUPPRESS_RECEIVE_BIT)) {
			sd_params->flags |= WL_NAN_SVC_CTRL_SUPPRESS_EVT_RECEIVE;
		}
		/* BIT3 - If set, host wont rec event "replied" */
		if (CHECK_BIT(cmd_data->recv_ind_flag, WL_NAN_EVENT_SUPPRESS_REPLIED_BIT)) {
			sd_params->flags |= WL_NAN_SVC_CTRL_SUPPRESS_EVT_REPLIED;
		}
	}
	if (cmd_id == WL_NAN_CMD_SD_PUBLISH) {
		sd_params->instance_id = cmd_data->pub_id;
		if (cmd_data->service_responder_policy) {
			/* Do not disturb avail if dam is supported */
			if (FW_SUPPORTED(dhdp, autodam)) {
				/* Nan Accept policy: Per service basis policy
				 * Based on this policy(ALL/NONE), responder side
				 * will send ACCEPT/REJECT
				 * If set, auto datapath responder will be sent by FW
				 */
				sd_params->flags |= WL_NAN_SVC_CTRL_AUTO_DPRESP;
			} else  {
				WL_ERR(("svc specifiv auto dp resp is not"
						" supported in non-auto dam fw\n"));
			}
		}
	} else if (cmd_id == WL_NAN_CMD_SD_SUBSCRIBE) {
		sd_params->instance_id = cmd_data->sub_id;
	} else {
		ret = BCME_USAGE_ERROR;
		WL_ERR(("wrong command id = %d \n", cmd_id));
		goto fail;
	}

	if ((cmd_data->svc_hash.dlen == WL_NAN_SVC_HASH_LEN) &&
			(cmd_data->svc_hash.data)) {
		ret = memcpy_s((uint8*)sd_params->svc_hash,
				sizeof(sd_params->svc_hash),
				cmd_data->svc_hash.data,
				cmd_data->svc_hash.dlen);
		if (ret != BCME_OK) {
			WL_ERR(("Failed to copy svc hash\n"));
			goto fail;
		}
#ifdef WL_NAN_DEBUG
		prhex("hashed svc name", cmd_data->svc_hash.data,
				cmd_data->svc_hash.dlen);
#endif /* WL_NAN_DEBUG */
	} else {
		ret = BCME_ERROR;
		WL_ERR(("invalid svc hash data or length = %d\n",
				cmd_data->svc_hash.dlen));
		goto fail;
	}

	/* check if ranging support is present in firmware */
	if ((cmd_data->sde_control_flag & NAN_SDE_CF_RANGING_REQUIRED) &&
		!FW_SUPPORTED(dhdp, nanrange)) {
		WL_ERR(("Service requires ranging but fw doesnt support it\n"));
		ret = BCME_UNSUPPORTED;
		goto fail;
	}

	/* Optional parameters: fill the sub_command block with service descriptor attr */
	sub_cmd->id = htod16(cmd_id);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);
	sub_cmd->len = sizeof(sub_cmd->u.options) +
		OFFSETOF(wl_nan_sd_params_t, optional[0]);
	pxtlv = (uint8*)&sd_params->optional[0];

	*nan_buf_size -= sub_cmd->len;
	buflen_avail = *nan_buf_size;

	if (cmd_data->svc_info.data && cmd_data->svc_info.dlen) {
		WL_TRACE(("optional svc_info present, pack it\n"));
		ret = bcm_pack_xtlv_entry(&pxtlv, nan_buf_size,
				WL_NAN_XTLV_SD_SVC_INFO,
				cmd_data->svc_info.dlen,
				cmd_data->svc_info.data, BCM_XTLV_OPTION_ALIGN32);
		if (unlikely(ret)) {
			WL_ERR(("%s: fail to pack WL_NAN_XTLV_SD_SVC_INFO\n", __FUNCTION__));
			goto fail;
		}
	}

	if (cmd_data->sde_svc_info.data && cmd_data->sde_svc_info.dlen) {
		WL_TRACE(("optional sdea svc_info present, pack it, %d\n",
			cmd_data->sde_svc_info.dlen));
		ret = bcm_pack_xtlv_entry(&pxtlv, nan_buf_size,
				WL_NAN_XTLV_SD_SDE_SVC_INFO,
				cmd_data->sde_svc_info.dlen,
				cmd_data->sde_svc_info.data, BCM_XTLV_OPTION_ALIGN32);
		if (unlikely(ret)) {
			WL_ERR(("%s: fail to pack sdea svc info\n", __FUNCTION__));
			goto fail;
		}
	}

	if (cmd_data->tx_match.dlen) {
		WL_TRACE(("optional tx match filter presnet (len=%d)\n",
				cmd_data->tx_match.dlen));
		ret = bcm_pack_xtlv_entry(&pxtlv, nan_buf_size,
				WL_NAN_XTLV_CFG_MATCH_TX, cmd_data->tx_match.dlen,
				cmd_data->tx_match.data, BCM_XTLV_OPTION_ALIGN32);
		if (unlikely(ret)) {
			WL_ERR(("%s: failed on xtlv_pack for tx match filter\n", __FUNCTION__));
			goto fail;
		}
	}

	if (cmd_data->life_count) {
		WL_TRACE(("optional life count is present, pack it\n"));
		ret = bcm_pack_xtlv_entry(&pxtlv, nan_buf_size, WL_NAN_XTLV_CFG_SVC_LIFE_COUNT,
				sizeof(cmd_data->life_count), &cmd_data->life_count,
				BCM_XTLV_OPTION_ALIGN32);
		if (unlikely(ret)) {
			WL_ERR(("%s: failed to WL_NAN_XTLV_CFG_SVC_LIFE_COUNT\n", __FUNCTION__));
			goto fail;
		}
	}

	if (cmd_data->use_srf) {
		uint8 srf_control = 0;
		/* set include bit */
		if (cmd_data->srf_include == true) {
			srf_control |= 0x2;
		}

		if (!ETHER_ISNULLADDR(&cmd_data->mac_list.list) &&
				(cmd_data->mac_list.num_mac_addr
				 < NAN_SRF_MAX_MAC)) {
			if (cmd_data->srf_type == SRF_TYPE_SEQ_MAC_ADDR) {
				/* mac list */
				srf_size = (cmd_data->mac_list.num_mac_addr
						* ETHER_ADDR_LEN) + NAN_SRF_CTRL_FIELD_LEN;
				WL_TRACE(("srf size = %d\n", srf_size));

				srf_mac = MALLOCZ(cfg->osh, srf_size);
				if (srf_mac == NULL) {
					WL_ERR(("%s: memory allocation failed\n", __FUNCTION__));
					ret = -ENOMEM;
					goto fail;
				}
				ret = memcpy_s(srf_mac, NAN_SRF_CTRL_FIELD_LEN,
						&srf_control, NAN_SRF_CTRL_FIELD_LEN);
				if (ret != BCME_OK) {
					WL_ERR(("Failed to copy srf control\n"));
					goto fail;
				}
				ret = memcpy_s(srf_mac+1, (srf_size - NAN_SRF_CTRL_FIELD_LEN),
						cmd_data->mac_list.list,
						(srf_size - NAN_SRF_CTRL_FIELD_LEN));
				if (ret != BCME_OK) {
					WL_ERR(("Failed to copy srf control mac list\n"));
					goto fail;
				}
				ret = bcm_pack_xtlv_entry(&pxtlv, nan_buf_size,
						WL_NAN_XTLV_CFG_SR_FILTER, srf_size, srf_mac,
						BCM_XTLV_OPTION_ALIGN32);
				if (unlikely(ret)) {
					WL_ERR(("%s: failed to WL_NAN_XTLV_CFG_SR_FILTER\n",
							__FUNCTION__));
					goto fail;
				}
			} else if (cmd_data->srf_type == SRF_TYPE_BLOOM_FILTER) {
				/* Create bloom filter */
				srf = MALLOCZ(cfg->osh, srf_ctrl_size);
				if (srf == NULL) {
					WL_ERR(("%s: memory allocation failed\n", __FUNCTION__));
					ret = -ENOMEM;
					goto fail;
				}
				/* Bloom filter */
				srf_control |= 0x1;
				/* Instance id must be from 1 to 255, 0 is Reserved */
				if (sd_params->instance_id == NAN_ID_RESERVED) {
					WL_ERR(("Invalid instance id: %d\n",
							sd_params->instance_id));
					ret = BCME_BADARG;
					goto fail;
				}
				if (bloom_idx == 0xFFFFFFFF) {
					bloom_idx = sd_params->instance_id % 4;
				} else {
					WL_ERR(("Invalid bloom_idx\n"));
					ret = BCME_BADARG;
					goto fail;

				}
				srf_control |= bloom_idx << 2;

				ret = wl_nan_bloom_create(&bp, &bloom_idx, bloom_len);
				if (unlikely(ret)) {
					WL_ERR(("%s: Bloom create failed\n", __FUNCTION__));
					goto fail;
				}

				srftmp = cmd_data->mac_list.list;
				for (a = 0;
					a < cmd_data->mac_list.num_mac_addr; a++) {
					ret = bcm_bloom_add_member(bp, srftmp, ETHER_ADDR_LEN);
					if (unlikely(ret)) {
						WL_ERR(("%s: Cannot add to bloom filter\n",
								__FUNCTION__));
						goto fail;
					}
					srftmp += ETHER_ADDR_LEN;
				}

				ret = memcpy_s(srf, NAN_SRF_CTRL_FIELD_LEN,
						&srf_control, NAN_SRF_CTRL_FIELD_LEN);
				if (ret != BCME_OK) {
					WL_ERR(("Failed to copy srf control\n"));
					goto fail;
				}
				ret = bcm_bloom_get_filter_data(bp, bloom_len,
						(srf + NAN_SRF_CTRL_FIELD_LEN),
						&bloom_size);
				if (unlikely(ret)) {
					WL_ERR(("%s: Cannot get filter data\n", __FUNCTION__));
					goto fail;
				}
				ret = bcm_pack_xtlv_entry(&pxtlv, nan_buf_size,
						WL_NAN_XTLV_CFG_SR_FILTER, srf_ctrl_size,
						srf, BCM_XTLV_OPTION_ALIGN32);
				if (ret != BCME_OK) {
					WL_ERR(("Failed to pack SR FILTER data, ret = %d\n", ret));
					goto fail;
				}
			} else {
				WL_ERR(("Invalid SRF Type = %d !!!\n",
						cmd_data->srf_type));
				goto fail;
			}
		} else {
			WL_ERR(("Invalid MAC Addr/Too many mac addr = %d !!!\n",
					cmd_data->mac_list.num_mac_addr));
			goto fail;
		}
	}

	if (cmd_data->rx_match.dlen) {
		WL_TRACE(("optional rx match filter is present, pack it\n"));
		ret = bcm_pack_xtlv_entry(&pxtlv, nan_buf_size,
				WL_NAN_XTLV_CFG_MATCH_RX, cmd_data->rx_match.dlen,
				cmd_data->rx_match.data, BCM_XTLV_OPTION_ALIGN32);
		if (unlikely(ret)) {
			WL_ERR(("%s: failed on xtlv_pack for rx match filter\n", __func__));
			goto fail;
		}
	}

	/* Security elements */
	if (cmd_data->csid) {
		WL_TRACE(("Cipher suite type is present, pack it\n"));
		ret = bcm_pack_xtlv_entry(&pxtlv, nan_buf_size,
				WL_NAN_XTLV_CFG_SEC_CSID, sizeof(nan_sec_csid_e),
				(uint8*)&cmd_data->csid, BCM_XTLV_OPTION_ALIGN32);
		if (unlikely(ret)) {
			WL_ERR(("%s: fail to pack on csid\n", __FUNCTION__));
			goto fail;
		}
	}

	if (cmd_data->ndp_cfg.security_cfg) {
		if ((cmd_data->key_type == NAN_SECURITY_KEY_INPUT_PMK) ||
			(cmd_data->key_type == NAN_SECURITY_KEY_INPUT_PASSPHRASE)) {
			if (cmd_data->key.data && cmd_data->key.dlen) {
				WL_TRACE(("optional pmk present, pack it\n"));
				ret = bcm_pack_xtlv_entry(&pxtlv, nan_buf_size,
					WL_NAN_XTLV_CFG_SEC_PMK, cmd_data->key.dlen,
					cmd_data->key.data, BCM_XTLV_OPTION_ALIGN32);
				if (unlikely(ret)) {
					WL_ERR(("%s: fail to pack WL_NAN_XTLV_CFG_SEC_PMK\n",
						__FUNCTION__));
					goto fail;
				}
			}
		} else {
			WL_ERR(("Invalid security key type\n"));
			ret = BCME_BADARG;
			goto fail;
		}
	}

	if (cmd_data->scid.data && cmd_data->scid.dlen) {
		WL_TRACE(("optional scid present, pack it\n"));
		ret = bcm_pack_xtlv_entry(&pxtlv, nan_buf_size, WL_NAN_XTLV_CFG_SEC_SCID,
			cmd_data->scid.dlen, cmd_data->scid.data, BCM_XTLV_OPTION_ALIGN32);
		if (unlikely(ret)) {
			WL_ERR(("%s: fail to pack WL_NAN_XTLV_CFG_SEC_SCID\n", __FUNCTION__));
			goto fail;
		}
	}

	if (cmd_data->sde_control_config) {
		ret = bcm_pack_xtlv_entry(&pxtlv, nan_buf_size,
				WL_NAN_XTLV_SD_SDE_CONTROL,
				sizeof(uint16), (uint8*)&cmd_data->sde_control_flag,
				BCM_XTLV_OPTION_ALIGN32);
		if (ret != BCME_OK) {
			WL_ERR(("%s: fail to pack WL_NAN_XTLV_SD_SDE_CONTROL\n", __FUNCTION__));
			goto fail;
		}
	}

	sub_cmd->len += (buflen_avail - *nan_buf_size);

fail:
	if (srf) {
		MFREE(cfg->osh, srf, srf_ctrl_size);
	}

	if (srf_mac) {
		MFREE(cfg->osh, srf_mac, srf_size);
	}
	NAN_DBG_EXIT();
	return ret;
}

static int
wl_cfgnan_aligned_data_size_of_opt_disc_params(uint16 *data_size, nan_discover_cmd_data_t *cmd_data)
{
	s32 ret = BCME_OK;
	if (cmd_data->svc_info.dlen)
		*data_size += ALIGN_SIZE(cmd_data->svc_info.dlen + NAN_XTLV_ID_LEN_SIZE, 4);
	if (cmd_data->sde_svc_info.dlen)
		*data_size += ALIGN_SIZE(cmd_data->sde_svc_info.dlen + NAN_XTLV_ID_LEN_SIZE, 4);
	if (cmd_data->tx_match.dlen)
		*data_size += ALIGN_SIZE(cmd_data->tx_match.dlen + NAN_XTLV_ID_LEN_SIZE, 4);
	if (cmd_data->rx_match.dlen)
		*data_size += ALIGN_SIZE(cmd_data->rx_match.dlen + NAN_XTLV_ID_LEN_SIZE, 4);
	if (cmd_data->use_srf) {
		if (cmd_data->srf_type == SRF_TYPE_SEQ_MAC_ADDR) {
			*data_size += (cmd_data->mac_list.num_mac_addr * ETHER_ADDR_LEN)
					+ NAN_SRF_CTRL_FIELD_LEN;
		} else { /* Bloom filter type */
			*data_size += NAN_BLOOM_LENGTH_DEFAULT + 1;
		}
		*data_size += ALIGN_SIZE(*data_size + NAN_XTLV_ID_LEN_SIZE, 4);
	}
	if (cmd_data->csid)
		*data_size +=  ALIGN_SIZE(sizeof(nan_sec_csid_e) + NAN_XTLV_ID_LEN_SIZE, 4);
	if (cmd_data->key.dlen)
		*data_size += ALIGN_SIZE(cmd_data->key.dlen + NAN_XTLV_ID_LEN_SIZE, 4);
	if (cmd_data->scid.dlen)
		*data_size += ALIGN_SIZE(cmd_data->scid.dlen + NAN_XTLV_ID_LEN_SIZE, 4);
	if (cmd_data->sde_control_config)
		*data_size += ALIGN_SIZE(sizeof(uint16) + NAN_XTLV_ID_LEN_SIZE, 4);
	if (cmd_data->life_count)
		*data_size += ALIGN_SIZE(sizeof(cmd_data->life_count) + NAN_XTLV_ID_LEN_SIZE, 4);
	return ret;
}

static int
wl_cfgnan_aligned_data_size_of_opt_dp_params(uint16 *data_size, nan_datapath_cmd_data_t *cmd_data)
{
	s32 ret = BCME_OK;
	if (cmd_data->svc_info.dlen)
		*data_size += ALIGN_SIZE(cmd_data->svc_info.dlen + NAN_XTLV_ID_LEN_SIZE, 4);
	if (cmd_data->key.dlen)
		*data_size += ALIGN_SIZE(cmd_data->key.dlen + NAN_XTLV_ID_LEN_SIZE, 4);
	if (cmd_data->csid)
		*data_size += ALIGN_SIZE(sizeof(nan_sec_csid_e) + NAN_XTLV_ID_LEN_SIZE, 4);

	*data_size += ALIGN_SIZE(WL_NAN_SVC_HASH_LEN + NAN_XTLV_ID_LEN_SIZE, 4);
	return ret;
}
int
wl_cfgnan_svc_get_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, uint16 cmd_id, nan_discover_cmd_data_t *cmd_data)
{
	bcm_iov_batch_subcmd_t *sub_cmd = NULL;
	uint32 instance_id;
	s32 ret = BCME_OK;
	bcm_iov_batch_buf_t *nan_buf = NULL;

	uint8 *resp_buf = NULL;
	uint16 data_size = WL_NAN_OBUF_DATA_OFFSET + sizeof(instance_id);

	NAN_DBG_ENTER();

	nan_buf = MALLOCZ(cfg->osh, data_size);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	resp_buf = MALLOCZ(cfg->osh, NAN_IOCTL_BUF_SIZE_LARGE);
	if (!resp_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 1;
	/* check if service is present */
	nan_buf->is_set = false;
	sub_cmd = (bcm_iov_batch_subcmd_t*)(&nan_buf->cmds[0]);
	if (cmd_id == WL_NAN_CMD_SD_PUBLISH) {
		instance_id = cmd_data->pub_id;
	} else if (cmd_id == WL_NAN_CMD_SD_SUBSCRIBE) {
		instance_id = cmd_data->sub_id;
	}  else {
		ret = BCME_USAGE_ERROR;
		WL_ERR(("wrong command id = %u\n", cmd_id));
		goto fail;
	}
	/* Fill the sub_command block */
	sub_cmd->id = htod16(cmd_id);
	sub_cmd->len = sizeof(sub_cmd->u.options) + sizeof(instance_id);
	sub_cmd->u.options = htol32(BCM_XTLV_OPTION_ALIGN32);

	ret = memcpy_s(sub_cmd->data, (data_size - WL_NAN_OBUF_DATA_OFFSET),
			&instance_id, sizeof(instance_id));
	if (ret != BCME_OK) {
		WL_ERR(("Failed to copy instance id, ret = %d\n", ret));
		goto fail;
	}

	ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, data_size,
			&(cmd_data->status), resp_buf, NAN_IOCTL_BUF_SIZE_LARGE);

	if (unlikely(ret) || unlikely(cmd_data->status)) {
		WL_ERR(("nan svc check failed ret = %d status = %d\n", ret, cmd_data->status));
		goto fail;
	} else {
		WL_DBG(("nan svc check successful..proceed to update\n"));
	}

fail:
	if (nan_buf) {
		MFREE(cfg->osh, nan_buf, data_size);
	}

	if (resp_buf) {
		MFREE(cfg->osh, resp_buf, NAN_IOCTL_BUF_SIZE_LARGE);
	}
	NAN_DBG_EXIT();
	return ret;

}

int
wl_cfgnan_svc_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, uint16 cmd_id, nan_discover_cmd_data_t *cmd_data)
{
	s32 ret = BCME_OK;
	bcm_iov_batch_buf_t *nan_buf = NULL;
	uint16 nan_buf_size;
	uint8 *resp_buf = NULL;
	/* Considering fixed params */
	uint16 data_size = WL_NAN_OBUF_DATA_OFFSET +
		OFFSETOF(wl_nan_sd_params_t, optional[0]);

	if (cmd_data->svc_update) {
		ret = wl_cfgnan_svc_get_handler(ndev, cfg, cmd_id, cmd_data);
		if (ret != BCME_OK) {
			WL_ERR(("Failed to update svc handler, ret = %d\n", ret));
			goto fail;
		} else {
			/* Ignoring any other svc get error */
			if (cmd_data->status == WL_NAN_E_BAD_INSTANCE) {
				WL_ERR(("Bad instance status, failed to update svc handler\n"));
				goto fail;
			}
		}
	}

	ret = wl_cfgnan_aligned_data_size_of_opt_disc_params(&data_size, cmd_data);
	if (unlikely(ret)) {
		WL_ERR(("Failed to get alligned size of optional params\n"));
		goto fail;
	}
	nan_buf_size = data_size;
	NAN_DBG_ENTER();

	nan_buf = MALLOCZ(cfg->osh, data_size);
	if (!nan_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}

	resp_buf = MALLOCZ(cfg->osh, data_size + NAN_IOVAR_NAME_SIZE);
	if (!resp_buf) {
		WL_ERR(("%s: memory allocation failed\n", __func__));
		ret = BCME_NOMEM;
		goto fail;
	}
	nan_buf->version = htol16(WL_NAN_IOV_BATCH_VERSION);
	nan_buf->count = 0;
	nan_buf->is_set = true;

	ret = wl_cfgnan_sd_params_handler(ndev, cmd_data, cmd_id,
			&nan_buf->cmds[0], &nan_buf_size);
	if (unlikely(ret)) {
		WL_ERR((" Service discovery params handler failed, ret = %d\n", ret));
		goto fail;
	}

	nan_buf->count++;
	ret = wl_cfgnan_execute_ioctl(ndev, cfg, nan_buf, data_size,
			&(cmd_data->status), resp_buf, data_size + NAN_IOVAR_NAME_SIZE);
	if (cmd_data->svc_update && (cmd_data->status == BCME_DATA_NOTFOUND)) {
		/* return OK if update tlv data is not present
		* which means nothing to update
		*/
		cmd_data->status = BCME_OK;
	}
	if (unlikely(ret) || unlikely(cmd_data->status)) {
		WL_ERR(("nan svc failed ret = %d status = %d\n", ret, cmd_data->status));
		goto fail;
	} else {
		WL_DBG(("nan svc successful\n"));
#ifdef WL_NAN_DISC_CACHE
		ret = wl_cfgnan_cache_svc_info(cfg, cmd_data, cmd_id, cmd_data->svc_update);
		if (ret < 0) {
			WL_ERR(("%s: fail to cache svc info, ret=%d\n",
				__FUNCTION__, ret));
			goto fail;
		}
#endif /* WL_NAN_DISC_CACHE */
	}

fail:
	if (nan_buf) {
		MFREE(cfg->osh, nan_buf, data_size);
	}

	if (resp_buf) {
		MFREE(cfg->osh, resp_buf, data_size + NAN_IOVAR_NAME_SIZE);
	}
	NAN_DBG_EXIT();
	return ret;
}

int
wl_cfgnan_publish_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, nan_discover_cmd_data_t *cmd_data)
{
	int ret = BCME_OK;

	NAN_DBG_ENTER();
	NAN_MUTEX_LOCK();
	/*
	 * proceed only if mandatory arguments are present - subscriber id,
	 * service hash
	 */
	if ((!cmd_data->pub_id) || (!cmd_data->svc_hash.data) ||
		(!cmd_data->svc_hash.dlen)) {
		WL_ERR(("mandatory arguments are not present\n"));
		ret = BCME_BADARG;
		goto fail;
	}

	ret = wl_cfgnan_svc_handler(ndev, cfg, WL_NAN_CMD_SD_PUBLISH, cmd_data);
	if (ret < 0) {
		WL_ERR(("%s: fail to handle pub, ret=%d\n", __FUNCTION__, ret));
		goto fail;
	}
	WL_INFORM_MEM(("[NAN] Service published for instance id:%d\n", cmd_data->pub_id));

fail:
	NAN_MUTEX_UNLOCK();
	NAN_DBG_EXIT();
	return ret;
}
