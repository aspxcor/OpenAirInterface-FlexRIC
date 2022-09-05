#ifndef MCS_DECRYPTION_GENERIC
#define MCS_DECRYPTION_GENERIC 

#include "mcs_dec_asn.h"
#include "mcs_dec_fb.h"
#include "mcs_dec_plain.h"

/////////////////////////////////////////////////////////////////////
// 9 Information Elements that are interpreted by the SM according
// to ORAN-WG3.E2SM-v01.00.00 Technical Specification
/////////////////////////////////////////////////////////////////////


#define mcs_dec_event_trigger(T,U,V) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_dec_event_trigger_plain, \
                           mcs_enc_asn_t*: mcs_dec_event_trigger_asn,\
                           mcs_enc_fb_t*: mcs_dec_event_trigger_fb,\
                           default: mcs_dec_event_trigger_plain) (U,V)

#define mcs_dec_action_def(T,U,V) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_dec_action_def_plain, \
                           mcs_enc_asn_t*: mcs_dec_action_def_asn, \
                           mcs_enc_fb_t*: mcs_dec_action_def_fb, \
                           default:  mcs_dec_action_def_plain) (U,V)

#define mcs_dec_ind_hdr(T,U,V) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_dec_ind_hdr_plain , \
                           mcs_enc_asn_t*: mcs_dec_ind_hdr_asn, \
                           mcs_enc_fb_t*: mcs_dec_ind_hdr_fb, \
                           default:  mcs_dec_ind_hdr_plain) (U,V)

#define mcs_dec_ind_msg(T,U,V) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_dec_ind_msg_plain , \
                           mcs_enc_asn_t*: mcs_dec_ind_msg_asn, \
                           mcs_enc_fb_t*: mcs_dec_ind_msg_fb, \
                           default:  mcs_dec_ind_msg_plain) (U,V)

#define mcs_dec_call_proc_id(T,U,V) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_dec_call_proc_id_plain , \
                           mcs_enc_asn_t*: mcs_dec_call_proc_id_asn, \
                           mcs_enc_fb_t*: mcs_dec_call_proc_id_fb, \
                           default:  mcs_dec_call_proc_id_plain) (U,V)

#define mcs_dec_ctrl_hdr(T,U,V) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_dec_ctrl_hdr_plain , \
                           mcs_enc_asn_t*: mcs_dec_ctrl_hdr_asn, \
                           mcs_enc_fb_t*: mcs_dec_ctrl_hdr_fb, \
                           default: mcs_dec_ctrl_hdr_plain) (U,V)

#define mcs_dec_ctrl_msg(T,U,V) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_dec_ctrl_msg_plain , \
                           mcs_enc_asn_t*: mcs_dec_ctrl_msg_asn, \
                           mcs_enc_fb_t*: mcs_dec_ctrl_msg_fb, \
                           default:  mcs_dec_ctrl_msg_plain) (U,V)

#define mcs_dec_ctrl_out(T,U,V) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_dec_ctrl_out_plain , \
                           mcs_enc_asn_t*: mcs_dec_ctrl_out_asn, \
                           mcs_enc_fb_t*: mcs_dec_ctrl_out_fb, \
                           default:  mcs_dec_ctrl_out_plain) (U,V)

#define mcs_dec_func_def(T,U,V) _Generic ((T), \
                           mcs_enc_plain_t*: mcs_dec_func_def_plain, \
                           mcs_enc_asn_t*: mcs_dec_func_def_asn, \
                           mcs_enc_fb_t*:  mcs_dec_func_def_fb, \
                           default:  mcs_dec_func_def_plain) (U,V)

#endif

