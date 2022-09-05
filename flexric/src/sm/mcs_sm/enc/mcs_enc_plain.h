/* file: mcs message encoding in the form of plain
usage: flexRIC for control downlink mcs value
author: Yuxiang Lin
date: 2022.6.7 */

#ifndef MCS_ENCRYPTION_PLAIN_H
#define MCS_ENCRYPTION_PLAIN_H 

#include "../../../util/byte_array.h"
#include "../ie/mcs_control_ie.h"


// Used for static polymorphism. 
// View mcs_enc_generic file
typedef struct{

} mcs_enc_plain_t;


byte_array_t mcs_enc_event_trigger_plain(mcs_event_trigger_t const* event_trigger);

byte_array_t mcs_enc_action_def_plain(mcs_action_def_t const*);

byte_array_t mcs_enc_ind_hdr_plain(mcs_ind_hdr_t const*); 

byte_array_t mcs_enc_ind_msg_plain(mcs_ind_msg_t const*); 

byte_array_t mcs_enc_call_proc_id_plain(mcs_call_proc_id_t const*); 

byte_array_t mcs_enc_ctrl_hdr_plain(mcs_ctrl_hdr_t const*); 

byte_array_t mcs_enc_ctrl_msg_plain(mcs_ctrl_msg_t const*); 

byte_array_t mcs_enc_ctrl_out_plain(mcs_ctrl_out_t const*); 

byte_array_t mcs_enc_func_def_plain(mcs_func_def_t const*);

#endif

