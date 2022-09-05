#ifndef MCS_ENCRYPTIOIN_ASN_H
#define MCS_ENCRYPTIOIN_ASN_H

#include "../../../util/byte_array.h"
#include "../ie/mcs_control_ie.h"

// Used for static polymorphism. 
// See mcs_enc_generic.h file
typedef struct{

} mcs_enc_asn_t;

byte_array_t mcs_enc_event_trigger_asn(mcs_event_trigger_t const* event_trigger);

byte_array_t mcs_enc_action_def_asn(mcs_action_def_t const*);

byte_array_t mcs_enc_ind_hdr_asn(mcs_ind_hdr_t const*); 

byte_array_t mcs_enc_ind_msg_asn(mcs_ind_msg_t const*); 

byte_array_t mcs_enc_call_proc_id_asn(mcs_call_proc_id_t const*); 

byte_array_t mcs_enc_ctrl_hdr_asn(mcs_ctrl_hdr_t const*); 

byte_array_t mcs_enc_ctrl_msg_asn(mcs_ctrl_msg_t const*); 

byte_array_t mcs_enc_ctrl_out_asn(mcs_ctrl_out_t const*); 

byte_array_t mcs_enc_func_def_asn(mcs_func_def_t const*);

#endif

