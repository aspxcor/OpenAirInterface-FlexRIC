#ifndef MCS_ENCRYPTION_GENERIC
#define MCS_ENCRYPTION_GENERIC 

#include "mcs_enc_asn.h"
#include "mcs_enc_fb.h"
#include "mcs_enc_plain.h"

/////////////////////////////////////////////////////////////////////
// 9 Information Elements that are interpreted by the SM according
// to ORAN-WG3.E2SM-v01.00.00 Technical Specification
/////////////////////////////////////////////////////////////////////


#define mcs_enc_event_trigger(T,U) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_enc_event_trigger_plain, \
                           mcs_enc_asn_t*: mcs_enc_event_trigger_asn,\
                           mcs_enc_fb_t*: mcs_enc_event_trigger_fb,\
                           default: mcs_enc_event_trigger_plain) (U)

#define mcs_enc_action_def(T,U) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_enc_action_def_plain, \
                           mcs_enc_asn_t*: mcs_enc_action_def_asn, \
                           mcs_enc_fb_t*: mcs_enc_action_def_fb, \
                           default:  mcs_enc_action_def_plain) (U)

#define mcs_enc_ind_hdr(T,U) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_enc_ind_hdr_plain , \
                           mcs_enc_asn_t*: mcs_enc_ind_hdr_asn, \
                           mcs_enc_fb_t*: mcs_enc_ind_hdr_fb, \
                           default:  mcs_enc_ind_hdr_plain) (U)

#define mcs_enc_ind_msg(T,U) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_enc_ind_msg_plain , \
                           mcs_enc_asn_t*: mcs_enc_ind_msg_asn, \
                           mcs_enc_fb_t*: mcs_enc_ind_msg_fb, \
                           default:  mcs_enc_ind_msg_plain) (U)

#define mcs_enc_call_proc_id(T,U) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_enc_call_proc_id_plain , \
                           mcs_enc_asn_t*: mcs_enc_call_proc_id_asn, \
                           mcs_enc_fb_t*: mcs_enc_call_proc_id_fb, \
                           default:  mcs_enc_call_proc_id_plain) (U)

#define mcs_enc_ctrl_hdr(T,U) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_enc_ctrl_hdr_plain , \
                           mcs_enc_asn_t*: mcs_enc_ctrl_hdr_asn, \
                           mcs_enc_fb_t*: mcs_enc_ctrl_hdr_fb, \
                           default:  mcs_enc_ctrl_hdr_plain) (U)

#define mcs_enc_ctrl_msg(T,U) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_enc_ctrl_msg_plain , \
                           mcs_enc_asn_t*: mcs_enc_ctrl_msg_asn, \
                           mcs_enc_fb_t*: mcs_enc_ctrl_msg_fb, \
                           default:  mcs_enc_ctrl_msg_plain) (U)

#define mcs_enc_ctrl_out(T,U) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_enc_ctrl_out_plain , \
                           mcs_enc_asn_t*: mcs_enc_ctrl_out_asn, \
                           mcs_enc_fb_t*: mcs_enc_ctrl_out_fb, \
                           default:  mcs_enc_ctrl_out_plain) (U)

#define mcs_enc_func_def(T,U) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_enc_func_def_plain, \
                           mcs_enc_asn_t*: mcs_enc_func_def_asn, \
                           mcs_enc_fb_t*:  mcs_enc_func_def_fb, \
                           default:  mcs_enc_func_def_plain) (U)

#endif

