/* file: mcs message encoding in the form of plain
usage: flexRIC for control downlink mcs value
author: Yuxiang Lin
date: 2022.6.7 */

#include "mcs_enc_plain.h"

#include <assert.h>
#include <stdlib.h>

byte_array_t mcs_enc_event_trigger_plain(mcs_event_trigger_t const* event_trigger)
{
  assert(event_trigger != NULL);
  byte_array_t  ba = {0};
 
  ba.len = sizeof(event_trigger->ms);
  ba.buf = malloc(ba.len);
  assert(ba.buf != NULL && "Memory exhausted");

  memcpy(ba.buf, &event_trigger->ms, ba.len);

  return ba;
}

byte_array_t mcs_enc_action_def_plain(mcs_action_def_t const* action_def)
{
  assert(0!=0 && "Not implemented");

  assert(action_def != NULL);
  byte_array_t  ba = {0};
  return ba;
}

byte_array_t mcs_enc_ind_hdr_plain(mcs_ind_hdr_t const* ind_hdr)
{
  assert(ind_hdr != NULL);

  byte_array_t ba = {0};

  ba.len = sizeof(mcs_ind_hdr_t);
  ba.buf = malloc(sizeof(mcs_ind_msg_t));
  assert(ba.buf != NULL && "memory exhausted");
  memcpy(ba.buf, ind_hdr, sizeof(mcs_ind_hdr_t));

  return ba;
}

static inline
size_t cal_static(static_mcs_t const* sta)
{
  assert(sta != NULL);
  return sizeof(sta->mcs_value);
}

static
size_t cal_params(mcs_params_t* par)
{
  assert(par != NULL);

  size_t sz = 0;
  if(par->type == MCS_ALG_SM_V0_STATIC){
    sz += cal_static(&par->sta);
  } else if (par->type == MCS_ALG_SM_V0_ORIGIN){
    // do nothing
  } else {
    assert(0 != 0 && "Unknown slicing type");
    // edf_mcs_t edf;
    //  mcs_SM_V0_EDF = 4
  }

  return sizeof(par->id) + sizeof(par->type) + sz;
}

static
size_t cal_ul_dl(ul_dl_mcs_conf_t const* slc)
{
  assert(slc != NULL);
  size_t sz = sizeof(slc->len_mcss);
  for(size_t i = 0; i < slc->len_mcss; ++i){
    sz += cal_params(&slc->mcss[i]);
  }

  return sz;
}

static
size_t cal_mcs_conf(mcs_conf_t const* slc)
{
  assert(slc != NULL);
  size_t ul = cal_ul_dl(&slc->ul);
  size_t dl = cal_ul_dl(&slc->dl);

  return ul+dl;
}

static
size_t cal_ue_mcs_assoc(ue_mcs_assoc_t* assoc)
{
  assert(assoc != NULL);
  return sizeof(assoc->dl_id) + sizeof(assoc->ul_id) + sizeof(assoc->rnti);
}

static
size_t cal_ue_mcs_conf(ue_mcs_conf_t const* slc)
{
  assert(slc != NULL);

  size_t sz = sizeof(slc->len_ue_mcs);
  for(size_t i = 0; i < slc->len_ue_mcs; ++i){
    sz += cal_ue_mcs_assoc(&slc->ues[i]);
  }

  return sz;
}

static
size_t cal_ind_msg_payload(mcs_ind_msg_t const* ind_msg)
{
  assert(ind_msg != NULL);

  size_t sz_conf = cal_mcs_conf(&ind_msg->mcs_conf);
  size_t sz_ues = cal_ue_mcs_conf(&ind_msg->ue_mcs_conf);
  size_t sz_tstamp = sizeof(int64_t);

  return sz_conf + sz_ues + sz_tstamp;
}

static
uint8_t* end;

// uint8_t* it is fed as formal parameters
static inline
size_t fill_static(uint8_t* it, static_mcs_t* sta)
{
  assert(it != NULL);
  assert(sta != NULL);

  assert(it < end);

  memcpy(it, &sta->mcs_value, sizeof(sta->mcs_value));
  it += sizeof(sta->mcs_value);
  size_t sz = sizeof(sta->mcs_value);

  return sz;
}

static
size_t fill_params(uint8_t* it, mcs_params_t* par)
{
  assert(it != NULL);
  assert(par != NULL);

  memcpy(it, &par->id, sizeof(par->id) );
  it += sizeof(par->id);
  size_t sz = sizeof(par->id);

  memcpy(it, &par->type, sizeof(par->type) );
  it += sizeof(par->type);
  sz += sizeof(par->type);

  if(par->type == MCS_ALG_SM_V0_STATIC ){
    sz += fill_static(it, &par->sta);  
  } else if(par->type == MCS_ALG_SM_V0_ORIGIN ){
    // do nothing
  } else {
    assert(0!=0 && "Unknown parameter type");
  }

  return sz; 
}

static
size_t fill_ul_dl_mcs_conf(uint8_t* it, ul_dl_mcs_conf_t const* conf)
{
  assert(it != NULL);
  assert(conf != NULL);

  memcpy(it, &conf->len_mcss, sizeof(conf->len_mcss));
  it += sizeof(conf->len_mcss);
  size_t sz = sizeof(conf->len_mcss);

  for(size_t i = 0; i < conf->len_mcss; ++i){
    size_t const tmp = fill_params(it, &conf->mcss[i]);
    it += tmp;
    sz += tmp;
  }
  return sz;
}

static
size_t fill_mcs_conf(uint8_t* it, mcs_conf_t const* conf)
{
  size_t sz = fill_ul_dl_mcs_conf(it, &conf->dl);
  it += sz;

  sz += fill_ul_dl_mcs_conf(it, &conf->ul);
  return sz;
}

static inline
size_t fill_ue_mcs_assoc(uint8_t* it, ue_mcs_assoc_t const* assoc)
{
  assert(it != NULL);
  assert(assoc != NULL);

  assert(it < end);

  memcpy(it, &assoc->dl_id, sizeof(assoc->dl_id));
  it += sizeof(assoc->dl_id);
  size_t sz = sizeof(assoc->dl_id);

  memcpy(it, &assoc->ul_id, sizeof(assoc->ul_id));
  it += sizeof(assoc->ul_id);
  sz += sizeof(assoc->ul_id);

  memcpy(it, &assoc->rnti, sizeof(assoc->rnti));
  it += sizeof(assoc->rnti);
  sz += sizeof(assoc->rnti);

  return sz;
}

static inline
size_t fill_ue_mcs_conf(uint8_t* it, ue_mcs_conf_t const* slc)
{
  assert(it != NULL);
  assert(slc != NULL);

  memcpy(it, &slc->len_ue_mcs, sizeof(slc->len_ue_mcs));
  it += sizeof(slc->len_ue_mcs);
  size_t sz = sizeof(slc->len_ue_mcs);

  for(size_t i = 0; i < slc->len_ue_mcs; ++i){
    size_t const tmp = fill_ue_mcs_assoc(it, &slc->ues[i]);
    
    it += tmp;
    sz += tmp;
  }

  return sz;
}


byte_array_t mcs_enc_ind_msg_plain(mcs_ind_msg_t const* ind_msg)
{
  assert(ind_msg != NULL);

  byte_array_t ba = {0};

  size_t sz = cal_ind_msg_payload(ind_msg);

  ba.buf = malloc(sz); 
  assert(ba.buf != NULL && "Memory exhausted");
  end = ba.buf + sz;

  uint8_t* it = ba.buf;
  size_t pos1 = fill_mcs_conf(it, &ind_msg->mcs_conf); 
  it += pos1;
  size_t pos2 = fill_ue_mcs_conf(it, &ind_msg->ue_mcs_conf);

  it += pos2;
  // tstamp
  memcpy(it, &ind_msg->tstamp, sizeof(ind_msg->tstamp));
  it += sizeof(ind_msg->tstamp);
  assert(it == ba.buf + sz && "Mismatch of data layout");

  ba.len = sz;
  return ba;
}

byte_array_t mcs_enc_call_proc_id_plain(mcs_call_proc_id_t const* call_proc_id)
{
  assert(0!=0 && "Not implemented");

  assert(call_proc_id != NULL);
  byte_array_t  ba = {0};
  return ba;
}

byte_array_t mcs_enc_ctrl_hdr_plain(mcs_ctrl_hdr_t const* ctrl_hdr)
{
  assert(ctrl_hdr != NULL);

  byte_array_t ba = {0};

  ba.len = sizeof(mcs_ind_hdr_t);
  ba.buf = malloc(sizeof(mcs_ind_msg_t));
  assert(ba.buf != NULL && "memory exhausted");
  memcpy(ba.buf, ctrl_hdr, sizeof(mcs_ctrl_hdr_t));

  return ba;
}

//////////////////////////////////////
// RIC Control Message 
/////////////////////////////////////

static
size_t cal_del_mcs(del_mcs_conf_t const* del)
{
  assert(del != 0);
  
  size_t sz = sizeof(del->len_dl);
  sz += del->len_dl*sizeof(uint32_t);

  sz += sizeof(del->len_ul);
  sz += del->len_ul*sizeof(uint32_t);

  return sz;
}

static
size_t cal_ctrl_msg_payload(mcs_ctrl_msg_t const* ctrl_msg)
{
  assert(ctrl_msg != NULL);

  size_t sz = sizeof(ctrl_msg->type);

  if(ctrl_msg->type == MCS_CTRL_SM_V0_ADD){
    sz += cal_mcs_conf(&ctrl_msg->add_mod_mcs);
  } else if(ctrl_msg->type == MCS_CTRL_SM_V0_DEL ){
    sz += cal_del_mcs(&ctrl_msg->del_mcs);
  } else if(ctrl_msg->type == MCS_CTRL_SM_V0_UE_MCS_ASSOC) {
    sz += cal_ue_mcs_conf(&ctrl_msg->ue_mcs);
  } else {
    assert(0!=0 && "Unknown type");
  }

  return sz;
}

static
size_t fill_del_mcs_conf(uint8_t* it, del_mcs_conf_t const* conf)
{
  assert(it != NULL);
  assert(conf != NULL);

  memcpy(it, &conf->len_dl, sizeof(conf->len_dl));
  it += sizeof(conf->len_dl);
  size_t sz = sizeof(conf->len_dl);

  for(size_t i = 0; i < conf->len_dl; ++i){
    memcpy(it, &conf->dl[i], sizeof(uint32_t)); 
    it += sizeof(uint32_t);
    sz += sizeof(uint32_t);
  }

  memcpy(it, &conf->len_ul, sizeof(conf->len_ul));
  it += sizeof(conf->len_ul);
  sz += sizeof(conf->len_ul);

  for(size_t i = 0; i < conf->len_ul; ++i){
    memcpy(it, &conf->ul[i], sizeof(uint32_t)); 
    it += sizeof(uint32_t);
    sz += sizeof(uint32_t);
  }

  return sz;
}

static
size_t fill_mcs_ctrl_msg(uint8_t* it, mcs_ctrl_msg_t const* msg )
{
  assert(it != NULL);
  assert(msg != NULL);

  memcpy(it, &msg->type, sizeof(msg->type));
  it += sizeof(msg->type);
  size_t sz = sizeof(msg->type);

  if(msg->type == MCS_CTRL_SM_V0_ADD ){
    sz += fill_mcs_conf(it, &msg->add_mod_mcs);
  } else if(msg->type == MCS_CTRL_SM_V0_DEL  ){
    sz += fill_del_mcs_conf(it, &msg->del_mcs);
  } else if(msg->type == MCS_CTRL_SM_V0_UE_MCS_ASSOC ) {
    sz += fill_ue_mcs_conf(it, &msg->ue_mcs);
  } else {
    assert(0 != 0 && "Unknown type");
  } 

  return sz;
}

byte_array_t mcs_enc_ctrl_msg_plain(mcs_ctrl_msg_t const* ctrl_msg)
{
  assert(ctrl_msg != NULL);

  byte_array_t ba = {0};
 
  size_t const sz = cal_ctrl_msg_payload(ctrl_msg);

  ba.buf = malloc(sz);
  assert(ba.buf != NULL && "Memory exhausted");
  ba.len = sz;

  end = ba.buf + sz;

  uint8_t* it = ba.buf;
  size_t const pos = fill_mcs_ctrl_msg(it, ctrl_msg);

  assert(pos == sz && "Mismatch of data layout");

  return ba;
}

byte_array_t mcs_enc_ctrl_out_plain(mcs_ctrl_out_t const* ctrl) 
{
  assert(ctrl != NULL );
  byte_array_t ba = {0};

  ba.len = sizeof(ctrl->len_diag) + ctrl->len_diag;

  ba.buf = malloc(ba.len);
  assert(ba.buf != NULL && "Memory exhausted");
  uint8_t* it = ba.buf;

  memcpy(it, &ctrl->len_diag, sizeof(ctrl->len_diag));
  it += sizeof(ctrl->len_diag);

  memcpy(it, ctrl->diagnostic, ctrl->len_diag);
  it += ctrl->len_diag;

  assert(it == ba.buf + ba.len);

  return ba;
}

byte_array_t mcs_enc_func_def_plain(mcs_func_def_t const* func)
{
  assert(0!=0 && "Not implemented");

  assert(func != NULL);
  byte_array_t  ba = {0};
  return ba;
}

