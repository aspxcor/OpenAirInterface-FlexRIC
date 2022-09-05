#ifndef MCS_DECRYPTION_ASN_H
#define MCS_DECRYPTION_ASN_H

#include <stddef.h>
#include "../ie/mcs_control_ie.h"


mcs_event_trigger_t mcs_dec_event_trigger_asn(size_t len, uint8_t const ev_tr[len]);

mcs_action_def_t mcs_dec_action_def_asn(size_t len, uint8_t const action_def[len]);

mcs_ind_hdr_t mcs_dec_ind_hdr_asn(size_t len, uint8_t const ind_hdr[len]); 

mcs_ind_msg_t mcs_dec_ind_msg_asn(size_t len, uint8_t const ind_msg[len]); 

mcs_call_proc_id_t mcs_dec_call_proc_id_asn(size_t len, uint8_t const call_proc_id[len]);

mcs_ctrl_hdr_t mcs_dec_ctrl_hdr_asn(size_t len, uint8_t const ctrl_hdr[len]); 

mcs_ctrl_msg_t mcs_dec_ctrl_msg_asn(size_t len, uint8_t const ctrl_msg[len]); 

mcs_ctrl_out_t mcs_dec_ctrl_out_asn(size_t len, uint8_t const ctrl_out[len]); 

mcs_func_def_t mcs_dec_func_def_asn(size_t len, uint8_t const func_def[len]);

#endif

