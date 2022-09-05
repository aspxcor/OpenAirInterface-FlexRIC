/* file: mcs control information elements defination
usage: flexRIC for control downlink mcs value
author: Yuxiang Lin
date: 2022.6.6 */

#ifndef MCS_CONTROL_INFORMATION_ELEMENTS_H
#define MCS_CONTROL_INFORMATION_ELEMENTS_H

/*
 * 9 Information Elements (IE) , RIC Event Trigger Definition, RIC Action Definition, RIC Indication Header, RIC Indication Message, RIC Call Process ID, RIC Control Header, RIC Control Message, RIC Control Outcome and RAN Function Definition defined by ORAN-WG3.E2SM-v01.00.00 at Section 5
 */

#include <stdbool.h>
#include <stdint.h>

//////////////////////////////////////
// RIC Event Trigger Definition
/////////////////////////////////////

typedef struct {
  uint32_t ms;
} mcs_event_trigger_t;

void free_mcs_event_trigger(mcs_event_trigger_t* src); 

mcs_event_trigger_t cp_mcs_event_trigger( mcs_event_trigger_t* src);

bool eq_mcs_event_trigger(mcs_event_trigger_t* m0, mcs_event_trigger_t* m1);



//////////////////////////////////////
// RIC Action Definition 
/////////////////////////////////////

typedef struct {

} mcs_action_def_t;

void free_mcs_action_def(mcs_action_def_t* src); 

mcs_action_def_t cp_mcs_action_def(mcs_action_def_t* src);

bool eq_mcs_action_def(mcs_event_trigger_t* m0,  mcs_event_trigger_t* m1);



//////////////////////////////////////
// RIC Indication Header 
/////////////////////////////////////

typedef struct{
  uint32_t dummy;  
} mcs_ind_hdr_t;

void free_mcs_ind_hdr(mcs_ind_hdr_t* src); 

mcs_ind_hdr_t cp_mcs_ind_hdr(mcs_ind_hdr_t* src);

bool eq_mcs_ind_hdr(mcs_ind_hdr_t* m0, mcs_ind_hdr_t* m1);



//////////////////////////////////////
// RIC Indication Message 
/////////////////////////////////////


typedef enum{
  MCS_ALG_SM_V0_ORIGIN = 0,
  MCS_ALG_SM_V0_STATIC = 1,

  MCS_ALG_SM_V0_END
} mcs_algorithm_e;


typedef struct{

} origin_mcs_t;

typedef struct{
  uint32_t mcs_value;
} static_mcs_t;


typedef struct{
  uint32_t id;
  mcs_algorithm_e type;
  union{
    origin_mcs_t ori;
    static_mcs_t sta;
  };
} mcs_params_t;

typedef struct{
  uint32_t len_mcss;
  mcs_params_t* mcss;
} ul_dl_mcs_conf_t;

typedef struct{
  ul_dl_mcs_conf_t dl;
  ul_dl_mcs_conf_t ul;
} mcs_conf_t;

typedef struct{
  uint32_t dl_id;
  uint32_t ul_id;
  uint16_t rnti;
} ue_mcs_assoc_t; 

typedef struct{
  uint32_t len_ue_mcs;
  ue_mcs_assoc_t* ues; 
} ue_mcs_conf_t;


typedef struct {
  mcs_conf_t mcs_conf;
  ue_mcs_conf_t ue_mcs_conf;
  int64_t tstamp;
} mcs_ind_msg_t;

void free_mcs_ind_msg(mcs_ind_msg_t* src); 

mcs_ind_msg_t cp_mcs_ind_msg(mcs_ind_msg_t* src);

bool eq_mcs_ind_msg(mcs_ind_msg_t const* m0, mcs_ind_msg_t const* m1);



//////////////////////////////////////
// RIC Call Process ID 
/////////////////////////////////////

typedef struct {
  uint32_t dummy;
} mcs_call_proc_id_t;

void free_mcs_call_proc_id( mcs_call_proc_id_t* src); 

mcs_call_proc_id_t cp_mcs_call_proc_id( mcs_call_proc_id_t* src);

bool eq_mcs_call_proc_id(mcs_call_proc_id_t* m0, mcs_call_proc_id_t* m1);




//////////////////////////////////////
// RIC Control Header 
/////////////////////////////////////

typedef struct {
  uint32_t dummy;
} mcs_ctrl_hdr_t;

void free_mcs_ctrl_hdr( mcs_ctrl_hdr_t* src); 

mcs_ctrl_hdr_t cp_mcs_ctrl_hdr(mcs_ctrl_hdr_t* src);

bool eq_mcs_ctrl_hdr(mcs_ctrl_hdr_t const* m0, mcs_ctrl_hdr_t const* m1);


//////////////////////////////////////
// RIC Control Message 
/////////////////////////////////////

typedef enum{
  MCS_CTRL_SM_V0_ADD,
  MCS_CTRL_SM_V0_DEL,
  MCS_CTRL_SM_V0_UE_MCS_ASSOC,

  MCS_CTRL_SM_V0_END
} mcs_ctrl_msg_e ;

typedef struct{
 uint32_t len_dl;
 uint32_t* dl;
 uint32_t len_ul;
 uint32_t* ul;
} del_mcs_conf_t; 

typedef struct {
  mcs_ctrl_msg_e type; 
  union{
    mcs_conf_t add_mod_mcs;
    del_mcs_conf_t del_mcs;
    ue_mcs_conf_t ue_mcs;
  };
} mcs_ctrl_msg_t;

void free_mcs_ctrl_msg( mcs_ctrl_msg_t* src); 

mcs_ctrl_msg_t cp_mcs_ctrl_msg(mcs_ctrl_msg_t* src);

bool eq_mcs_ctrl_msg(mcs_ctrl_msg_t const* m0, mcs_ctrl_msg_t const* m1);


//////////////////////////////////////
// RIC Control Outcome 
/////////////////////////////////////

typedef struct {
  uint32_t len_diag;
  char* diagnostic; // human-readable diagnostic for northbound
} mcs_ctrl_out_t;

void free_mcs_ctrl_out(mcs_ctrl_out_t* src); 

mcs_ctrl_out_t cp_mcs_ctrl_out(mcs_ctrl_out_t* src);

bool eq_mcs_ctrl_out(mcs_ctrl_out_t* m0, mcs_ctrl_out_t* m1);



//////////////////////////////////////
// RAN Function Definition 
/////////////////////////////////////

typedef struct {
  uint32_t len_supported_alg;
  mcs_algorithm_e* supported_alg;
} mcs_func_def_t;

void free_mcs_func_def( mcs_func_def_t* src); 

mcs_func_def_t cp_mcs_func_def(mcs_func_def_t* src);

bool eq_mcs_func_def(mcs_func_def_t* m0, mcs_func_def_t* m1);



void mcs_free_ind_msg(mcs_ind_msg_t* msg);

void mcs_free_ctrl_msg(mcs_ctrl_msg_t* msg);

void mcs_free_ctrl_out(mcs_ctrl_out_t* out);

/////////////////////////////////////////////////
//////////////////////////////////////////////////
/////////////////////////////////////////////////


/*
 * O-RAN defined 5 Procedures: RIC Subscription, RIC Indication, RIC Control, E2 Setup and RIC Service Update 
 * */


///////////////
/// RIC Subscription
///////////////

typedef struct{
  mcs_event_trigger_t et; 
  mcs_action_def_t* ad;
} mcs_sub_data_t;

///////////////
// RIC Indication
///////////////

typedef struct{
  mcs_ind_hdr_t hdr;
  mcs_ind_msg_t msg;
  mcs_call_proc_id_t* proc_id;
} mcs_ind_data_t;

///////////////
// RIC Control
///////////////

typedef struct{
  mcs_ctrl_hdr_t hdr;
  mcs_ctrl_msg_t msg;
} mcs_ctrl_req_data_t;

typedef struct{
  mcs_ctrl_out_t* out;
} mcs_ctrl_out_data_t;

///////////////
// E2 Setup
///////////////

typedef struct{
  mcs_func_def_t func_def;
} mcs_e2_setup_data_t;

///////////////
// RIC Service Update
///////////////

typedef struct{
  mcs_func_def_t func_def;
} mcs_ric_service_update_t;


#endif