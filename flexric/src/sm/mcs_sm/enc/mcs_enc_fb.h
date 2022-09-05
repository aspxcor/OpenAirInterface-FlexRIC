#ifndef MCS_ENCRYPTION_FLATBUFFERS_H
#define MCS_ENCRYPTION_FLATBUFFERS_H

#include "../../../util/byte_array.h"
#include "../ie/mcs_control_ie.h"


// Used for static polymorphism. 
// View mcs_enc_generic file
typedef struct{

} mcs_enc_fb_t;

byte_array_t mcs_enc_event_trigger_fb(mcs_event_trigger_t const* event_trigger);

byte_array_t mcs_enc_action_def_fb(mcs_action_def_t const*);

byte_array_t mcs_enc_ind_hdr_fb(mcs_ind_hdr_t const*); 

byte_array_t mcs_enc_ind_msg_fb(mcs_ind_msg_t const*); 

byte_array_t mcs_enc_call_proc_id_fb(mcs_call_proc_id_t const*); 

byte_array_t mcs_enc_ctrl_hdr_fb(mcs_ctrl_hdr_t const*); 

byte_array_t mcs_enc_ctrl_msg_fb(mcs_ctrl_msg_t const*); 

byte_array_t mcs_enc_ctrl_out_fb(mcs_ctrl_out_t const*); 

byte_array_t mcs_enc_func_def_fb(mcs_func_def_t const*);

#endif

