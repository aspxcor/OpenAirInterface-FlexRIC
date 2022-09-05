#include "mcs_dec_plain.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// static inline
// size_t next_pow2(size_t x)
// {
//   static_assert(sizeof(x) == 8, "Need this size to work correctly");
//   x -= 1;
// 	x |= (x >> 1);
// 	x |= (x >> 2);
// 	x |= (x >> 4);
// 	x |= (x >> 8);
// 	x |= (x >> 16);
// 	x |= (x >> 32);
	
// 	return x + 1;
// }

mcs_event_trigger_t mcs_dec_event_trigger_plain(size_t len, uint8_t const ev_tr[len])
{
  mcs_event_trigger_t ev = {0};
  memcpy(&ev.ms, ev_tr, sizeof(ev.ms));
  return ev;
}

mcs_action_def_t mcs_dec_action_def_plain(size_t len, uint8_t const action_def[len])
{
  assert(0!=0 && "Not implemented");
  assert(action_def != NULL);
  mcs_action_def_t act_def;// = {0};
  return act_def;
}

mcs_ind_hdr_t mcs_dec_ind_hdr_plain(size_t len, uint8_t const ind_hdr[len])
{
  assert(len == sizeof(mcs_ind_hdr_t)); 
  mcs_ind_hdr_t ret;
  memcpy(&ret, ind_hdr, len);
  return ret;
}

static
uint8_t* end;

static 
uint8_t* begin;


static
bool it_within_layout(uint8_t const* it)
{
  return it >= begin && it < end;  
}


static inline
size_t fill_static(static_mcs_t* sta, uint8_t const* it)
{
  assert(it != NULL);
  assert(sta != NULL);
  assert(it_within_layout(it) == true);

  memcpy(&sta->mcs_value, it, sizeof(sta->mcs_value));
  it += sizeof(sta->mcs_value);
  size_t sz = sizeof(sta->mcs_value);

  return sz;
}


static inline
size_t fill_params(mcs_params_t* par, uint8_t const* it)
{
  assert(par != NULL);
  assert(it != NULL);

  memcpy(&par->id, it, sizeof(par->id) );
  it += sizeof(par->id);
  size_t sz = sizeof(par->id);

  memcpy(&par->type, it, sizeof(par->type) );
  it += sizeof(par->type);
  sz += sizeof(par->type);

  if(par->type == MCS_ALG_SM_V0_STATIC ){
    sz += fill_static(&par->sta, it);  
  } else if(par->type == MCS_ALG_SM_V0_ORIGIN ){
    // do nothing
  } else {
    assert(0!=0 && "Unknown parameter type");
  }

  return sz;
}

static inline
size_t fill_ul_dl_mcs_conf(ul_dl_mcs_conf_t* conf, uint8_t const* it)
{
  assert(conf != NULL);
  assert(it != NULL);

  memcpy(&conf->len_mcss, it, sizeof(conf->len_mcss));
  it += sizeof(conf->len_mcss);
  size_t sz = sizeof(conf->len_mcss);

  if(conf->len_mcss > 0){
    conf->mcss = calloc(conf->len_mcss, sizeof(mcs_params_t));
    assert(conf->mcss != NULL && "Memory exhausted");
  }

  for(size_t i = 0; i < conf->len_mcss; ++i){
    size_t const tmp = fill_params(&conf->mcss[i], it);
    it += tmp;
    sz += tmp;
  }

  return sz;
}

static
size_t fill_mcs_conf(mcs_conf_t* conf,  uint8_t const* it)
{
  assert(conf != NULL);
  assert(it != NULL);
  
  size_t sz = fill_ul_dl_mcs_conf(&conf->dl, it); 
  it += sz;

  sz += fill_ul_dl_mcs_conf(&conf->ul, it); 

  return sz;
}

static inline
size_t fill_ue_mcs_assoc(ue_mcs_assoc_t* assoc,uint8_t const* it)
{
  assert(it != NULL);
  assert(assoc != NULL);

  memcpy(&assoc->dl_id, it, sizeof(assoc->dl_id));
  it += sizeof(assoc->dl_id);
  size_t sz = sizeof(assoc->dl_id);

  memcpy(&assoc->ul_id, it, sizeof(assoc->ul_id));
  it += sizeof(assoc->ul_id);
  sz += sizeof(assoc->ul_id);

  memcpy(&assoc->rnti, it, sizeof(assoc->rnti));
  it += sizeof(assoc->rnti);
  sz += sizeof(assoc->rnti);

  return sz;
}



static inline
size_t fill_ue_mcs_conf(ue_mcs_conf_t* slc, uint8_t const* it)
{
  assert(it != NULL);
  assert(slc != NULL);

  memcpy(&slc->len_ue_mcs, it, sizeof(slc->len_ue_mcs));
  it += sizeof(slc->len_ue_mcs);
  size_t sz = sizeof(slc->len_ue_mcs);

  if(slc->len_ue_mcs > 0){
    slc->ues = calloc(slc->len_ue_mcs, sizeof(ue_mcs_assoc_t));
    assert(slc->ues != NULL && "memory exhausted");
  }

  for(size_t i = 0; i < slc->len_ue_mcs; ++i){
    size_t const tmp = fill_ue_mcs_assoc(&slc->ues[i], it);
    it += tmp;
    sz += tmp;
  }

  return sz;
}



mcs_ind_msg_t mcs_dec_ind_msg_plain(size_t len, uint8_t const ind_msg[len])
{

  mcs_ind_msg_t ind = {0};

  uint8_t const* it = ind_msg;
  begin = (uint8_t*)it;
  end = begin + len;
  size_t sz = fill_mcs_conf(&ind.mcs_conf, it);
  it += sz;

  sz = fill_ue_mcs_conf(&ind.ue_mcs_conf, it);
  it += sz;

  memcpy(&ind.tstamp, it, sizeof(ind.tstamp));
  it += sizeof(ind.tstamp);

  assert(ind_msg + len == it && "Data layout mismacth");

  return ind;
}

mcs_call_proc_id_t mcs_dec_call_proc_id_plain(size_t len, uint8_t const call_proc_id[len])
{
  assert(0!=0 && "Not implemented");
  assert(call_proc_id != NULL);
}

mcs_ctrl_hdr_t mcs_dec_ctrl_hdr_plain(size_t len, uint8_t const ctrl_hdr[len])
{
  assert(len == sizeof(mcs_ctrl_hdr_t)); 
  mcs_ctrl_hdr_t ret;
  memcpy(&ret, ctrl_hdr, len);
  return ret;
}

static
size_t fill_del_mcs(del_mcs_conf_t* conf, uint8_t const* it)
{
  assert(conf != NULL);
  assert(it != NULL);
  
  memcpy(&conf->len_dl, it, sizeof(conf->len_dl));
  it += sizeof(conf->len_dl);
  size_t sz = sizeof(conf->len_dl);

  if(conf->len_dl > 0){
    conf->dl = calloc(conf->len_dl, sizeof(uint32_t));
    assert(conf->dl != NULL && "memory exhausted");
  }

  for(size_t i = 0; i < conf->len_dl; ++i){
    memcpy(&conf->dl[i], it, sizeof(uint32_t));
    it += sizeof(uint32_t);
    sz += sizeof(uint32_t);
  }


  memcpy(&conf->len_ul, it, sizeof(conf->len_ul));
  it += sizeof(conf->len_ul);
  sz += sizeof(conf->len_ul);


  if(conf->len_ul > 0){
    conf->ul = calloc(conf->len_ul, sizeof(uint32_t));
    assert(conf->ul != NULL && "memory exhausted");
  }

  for(size_t i = 0; i < conf->len_ul; ++i){
    memcpy(&conf->ul[i], it, sizeof(uint32_t));
    it += sizeof(uint32_t);
    sz += sizeof(uint32_t);
  }

  return sz;
}

mcs_ctrl_msg_t mcs_dec_ctrl_msg_plain(size_t len, uint8_t const ctrl_msg[len])
{
  mcs_ctrl_msg_t ctrl = {0}; 
  
  uint8_t const* it = ctrl_msg; 
  begin = (uint8_t*)ctrl_msg;
  end = (uint8_t*)ctrl_msg + len;

  memcpy(&ctrl.type, it, sizeof(ctrl.type));
  it += sizeof(ctrl.type);
  size_t sz = sizeof(ctrl.type);

  if(ctrl.type == MCS_CTRL_SM_V0_ADD){
    sz += fill_mcs_conf(&ctrl.add_mod_mcs, it); 
  } else if (ctrl.type == MCS_CTRL_SM_V0_DEL){
    sz += fill_del_mcs(&ctrl.del_mcs, it); 
  } else if(ctrl.type == MCS_CTRL_SM_V0_UE_MCS_ASSOC){
    sz += fill_ue_mcs_conf(&ctrl.ue_mcs, it);
  } else {
    assert(0!=0 && "Unknonw type");
  }

  assert(sz == len && "Data layout mismatch");

  return ctrl;
}

mcs_ctrl_out_t mcs_dec_ctrl_out_plain(size_t len, uint8_t const ctrl_out[len]) 
{
  assert(ctrl_out!= NULL);

  mcs_ctrl_out_t ret = {0}; 

  uint8_t* it = (uint8_t*)ctrl_out;
  memcpy(&ret.len_diag, it, sizeof(ret.len_diag));
  it += sizeof(ret.len_diag);

  if(ret.len_diag > 0){
    ret.diagnostic = malloc(ret.len_diag);
    assert(ret.diagnostic != NULL && "memory exhausted");
    memcpy(ret.diagnostic, it, ret.len_diag);
  }

  return ret;
}

mcs_func_def_t mcs_dec_func_def_plain(size_t len, uint8_t const func_def[len])
{
  assert(0!=0 && "Not implemented");
  assert(func_def != NULL);
}

