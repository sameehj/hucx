/**
 * Copyright (C) Mellanox Technologies Ltd. 2001-2016.  ALL RIGHTS RESERVED.
 * Copyright (C) Huawei Technologies Co.,Ltd. 2021. ALL RIGHTS RESERVED.
 *
 * See file LICENSE for terms.
 */


#ifndef UCP_EP_INL_
#define UCP_EP_INL_

#include "ucp_ep.h"
#include "ucp_worker.h"
#include "ucp_worker.inl"
#include "ucp_context.h"

#include <ucp/wireup/wireup.h>
#include <ucs/arch/bitops.h>
#include <ucs/datastruct/ptr_map.inl>


static inline ucp_ep_config_t *ucp_ep_config(ucp_ep_h ep)
{
    ucs_assert(ep->cfg_index != UCP_WORKER_CFG_INDEX_NULL);
    return &ep->worker->ep_config[ep->cfg_index];
}

static inline ucp_lane_index_t ucp_ep_get_am_lane(ucp_ep_h ep)
{
    ucs_assert(ucp_ep_config(ep)->key.am_lane != UCP_NULL_LANE);
    return ep->am_lane;
}

static inline ucp_lane_index_t ucp_ep_get_wireup_msg_lane(ucp_ep_h ep)
{
    ucp_lane_index_t lane = ucp_ep_config(ep)->key.wireup_msg_lane;
    return (lane == UCP_NULL_LANE) ? ucp_ep_get_am_lane(ep) : lane;
}

static inline ucp_lane_index_t ucp_ep_get_tag_lane(ucp_ep_h ep)
{
    ucs_assert(ucp_ep_config(ep)->key.tag_lane != UCP_NULL_LANE);
    return ucp_ep_config(ep)->key.tag_lane;
}

static inline int ucp_ep_is_tag_offload_enabled(ucp_ep_config_t *config)
{
    ucp_lane_index_t lane = config->key.tag_lane;

    if (lane != UCP_NULL_LANE) {
        ucs_assert(config->key.lanes[lane].rsc_index != UCP_NULL_RESOURCE);
        return 1;
    }
    return 0;
}

static inline uct_ep_h ucp_ep_get_am_uct_ep(ucp_ep_h ep)
{
    return ep->uct_eps[ucp_ep_get_am_lane(ep)];
}

static inline uct_ep_h ucp_ep_get_tag_uct_ep(ucp_ep_h ep)
{
    return ep->uct_eps[ucp_ep_get_tag_lane(ep)];
}

static inline ucp_rsc_index_t ucp_ep_get_rsc_index(ucp_ep_h ep, ucp_lane_index_t lane)
{
    ucs_assert(lane < UCP_MAX_LANES); /* to suppress coverity */
    return ucp_ep_config(ep)->key.lanes[lane].rsc_index;
}

static inline uint8_t ucp_ep_get_path_index(ucp_ep_h ep, ucp_lane_index_t lane)
{
    return ucp_ep_config(ep)->key.lanes[lane].path_index;
}

static inline uct_iface_attr_t *ucp_ep_get_iface_attr(ucp_ep_h ep, ucp_lane_index_t lane)
{
    return ucp_worker_iface_get_attr(ep->worker, ucp_ep_get_rsc_index(ep, lane));
}

static inline uct_iface_attr_t *ucp_ep_get_am_iface_attr(ucp_ep_h ep)
{
    return ucp_ep_get_iface_attr(ep, ucp_ep_get_am_lane(ep));
}

static inline size_t ucp_ep_get_max_bcopy(ucp_ep_h ep, ucp_lane_index_t lane)
{
    return ucp_ep_get_iface_attr(ep, lane)->cap.am.max_bcopy;
}

static inline size_t ucp_ep_get_max_zcopy(ucp_ep_h ep, ucp_lane_index_t lane)
{
    return ucp_ep_get_iface_attr(ep, lane)->cap.am.max_zcopy;
}

static inline size_t ucp_ep_get_max_iov(ucp_ep_h ep, ucp_lane_index_t lane)
{
    return ucp_ep_get_iface_attr(ep, lane)->cap.am.max_iov;
}

static inline ucp_lane_index_t ucp_ep_num_lanes(ucp_ep_h ep)
{
    return ucp_ep_config(ep)->key.num_lanes;
}

static inline int ucp_ep_is_lane_p2p(ucp_ep_h ep, ucp_lane_index_t lane)
{
    return ucp_ep_config(ep)->p2p_lanes & UCS_BIT(lane);
}

static inline ucp_md_index_t ucp_ep_md_index(ucp_ep_h ep, ucp_lane_index_t lane)
{
    return ucp_ep_config(ep)->md_index[lane];
}

static inline uct_md_h ucp_ep_md(ucp_ep_h ep, ucp_lane_index_t lane)
{
    ucp_context_h context = ep->worker->context;
    return context->tl_mds[ucp_ep_md_index(ep, lane)].md;
}

static inline const uct_md_attr_t* ucp_ep_md_attr(ucp_ep_h ep, ucp_lane_index_t lane)
{
    ucp_context_h context = ep->worker->context;
    return &context->tl_mds[ucp_ep_md_index(ep, lane)].attr;
}

static inline uct_md_h ucp_ep_get_am_uct_md(ucp_ep_h ep)
{
    return ucp_ep_md(ep, ucp_ep_get_am_lane(ep));
}

static inline const uct_md_attr_t* ucp_ep_get_am_uct_md_attr(ucp_ep_h ep)
{
    return ucp_ep_md_attr(ep, ucp_ep_get_am_lane(ep));
}

static UCS_F_ALWAYS_INLINE ucp_ep_ext_gen_t* ucp_ep_ext_gen(ucp_ep_h ep)
{
    return (ucp_ep_ext_gen_t*)ucs_strided_elem_get(ep, 0, 1);
}

static UCS_F_ALWAYS_INLINE ucp_ep_ext_proto_t* ucp_ep_ext_proto(ucp_ep_h ep)
{
    return (ucp_ep_ext_proto_t*)ucs_strided_elem_get(ep, 0, 2);
}

static UCS_F_ALWAYS_INLINE ucp_ep_h ucp_ep_from_ext_gen(ucp_ep_ext_gen_t *ep_ext)
{
    return (ucp_ep_h)ucs_strided_elem_get(ep_ext, 1, 0);
}

static UCS_F_ALWAYS_INLINE ucp_ep_h ucp_ep_from_ext_proto(ucp_ep_ext_proto_t *ep_ext)
{
    return (ucp_ep_h)ucs_strided_elem_get(ep_ext, 2, 0);
}

static UCS_F_ALWAYS_INLINE ucp_ep_flush_state_t* ucp_ep_flush_state(ucp_ep_h ep)
{
    ucs_assert(ep->flags & UCP_EP_FLAG_FLUSH_STATE_VALID);
    ucs_assert(!(ep->flags & UCP_EP_FLAG_ON_MATCH_CTX));
    ucs_assert(!(ep->flags & UCP_EP_FLAG_LISTENER));
    ucs_assert(!(ep->flags & UCP_EP_FLAG_CLOSE_REQ_VALID));
    return &ucp_ep_ext_gen(ep)->flush_state;
}

static UCS_F_ALWAYS_INLINE ucp_ep_ext_control_t* ucp_ep_ext_control(ucp_ep_h ep)
{
    ucs_assert(ucp_ep_ext_gen(ep)->control_ext != NULL);
    return ucp_ep_ext_gen(ep)->control_ext;
}

static UCS_F_ALWAYS_INLINE ucs_ptr_map_key_t ucp_ep_remote_id(ucp_ep_h ep)
{
#if UCS_ENABLE_ASSERT
    if (!(ep->flags & UCP_EP_FLAG_REMOTE_ID)) {
        /* Let remote side assert if it gets invalid key */
        return UCP_EP_ID_INVALID;
    }
#endif
    return ucp_ep_ext_control(ep)->remote_ep_id;
}

static UCS_F_ALWAYS_INLINE ucs_ptr_map_key_t ucp_ep_local_id(ucp_ep_h ep)
{
    ucs_assert(ucp_ep_ext_control(ep)->local_ep_id != UCP_EP_ID_INVALID);
    return ucp_ep_ext_control(ep)->local_ep_id;
}

/*
 * Make sure we have a valid dest_ep_ptr value, so protocols which require a
 * reply from remote side could be used.
 */
static UCS_F_ALWAYS_INLINE ucs_status_t
ucp_ep_resolve_remote_id(ucp_ep_h ep, ucp_lane_index_t lane)
{
    if (ep->flags & UCP_EP_FLAG_REMOTE_ID) {
        return UCS_OK;
    }

    return ucp_wireup_connect_remote(ep, lane);
}

static inline void ucp_ep_update_remote_id(ucp_ep_h ep,
                                           ucs_ptr_map_key_t remote_id)
{
    if (ep->flags & UCP_EP_FLAG_REMOTE_ID) {
        ucs_assertv(remote_id == ucp_ep_ext_control(ep)->remote_ep_id,
                    "ep=%p rkey=0x%" PRIxPTR " ep->remote_id=0x%" PRIxPTR,
                    ep, remote_id, ucp_ep_ext_control(ep)->remote_ep_id);
    }

    ucs_assert(remote_id != UCP_EP_ID_INVALID);
    ucs_trace("ep %p: set remote_id to 0x%" PRIxPTR, ep, remote_id);
    ep->flags                           |= UCP_EP_FLAG_REMOTE_ID;
    ucp_ep_ext_control(ep)->remote_ep_id = remote_id;
}

static inline const char* ucp_ep_peer_name(ucp_ep_h ep)
{
#if ENABLE_DEBUG_DATA
    return ep->peer_name;
#else
    return UCP_WIREUP_EMPTY_PEER_NAME;
#endif
}

static inline void ucp_ep_flush_state_reset(ucp_ep_h ep)
{
    ucp_ep_flush_state_t *flush_state = &ucp_ep_ext_gen(ep)->flush_state;

    ucs_assert(!(ep->flags & (UCP_EP_FLAG_ON_MATCH_CTX |
                              UCP_EP_FLAG_LISTENER)));
    ucs_assert(!(ep->flags & UCP_EP_FLAG_FLUSH_STATE_VALID) ||
               ((flush_state->send_sn == 0) &&
                (flush_state->cmpl_sn == 0) &&
                ucs_queue_is_empty(&flush_state->reqs)));

    flush_state->send_sn = 0;
    flush_state->cmpl_sn = 0;
    ucs_queue_head_init(&flush_state->reqs);
    ep->flags |= UCP_EP_FLAG_FLUSH_STATE_VALID;
}

static inline void ucp_ep_flush_state_invalidate(ucp_ep_h ep)
{
    ucs_assert(ucs_queue_is_empty(&ucp_ep_flush_state(ep)->reqs));
    ep->flags &= ~UCP_EP_FLAG_FLUSH_STATE_VALID;
}

/* get index of the local component which can reach a remote memory domain */
static inline ucp_rsc_index_t
ucp_ep_config_get_dst_md_cmpt(const ucp_ep_config_key_t *key,
                              ucp_md_index_t dst_md_index)
{
    unsigned idx = ucs_popcount(key->reachable_md_map & UCS_MASK(dst_md_index));

    return key->dst_md_cmpts[idx];
}

static inline int
ucp_ep_config_key_has_cm_lane(const ucp_ep_config_key_t *config_key)
{
    return config_key->cm_lane != UCP_NULL_LANE;
}

static inline int ucp_ep_has_cm_lane(ucp_ep_h ep)
{
    return (ep->cfg_index != UCP_WORKER_CFG_INDEX_NULL) &&
           ucp_ep_config_key_has_cm_lane(&ucp_ep_config(ep)->key);
}

static UCS_F_ALWAYS_INLINE ucp_lane_index_t ucp_ep_get_cm_lane(ucp_ep_h ep)
{
    return ucp_ep_config(ep)->key.cm_lane;
}

static inline int
ucp_ep_config_connect_p2p(ucp_worker_h worker,
                          const ucp_ep_config_key_t *ep_config_key,
                          ucp_rsc_index_t rsc_index)
{
    /* The EP with CM lane has to be connected to remote EP, so prefer native
     * UCT p2p capability. */
    return ucp_ep_config_key_has_cm_lane(ep_config_key) ?
           ucp_worker_is_tl_p2p(worker, rsc_index) :
           !ucp_worker_is_tl_2iface(worker, rsc_index);
}

static UCS_F_ALWAYS_INLINE int ucp_ep_use_indirect_id(ucp_ep_h ep)
{
    UCS_STATIC_ASSERT(sizeof(ep->flags) <= sizeof(int));
    return ep->flags & UCP_EP_FLAG_INDIRECT_ID;
}
#endif
