#include "slice_data_ie.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static
void free_slice_param(slice_params_t* param)
{
  assert(param != NULL);

  if(param->type == SLICE_ALG_SM_V0_NONE){
   // no memory to be freed 
  } else if(param->type == SLICE_ALG_SM_V0_STATIC){
   // no memory to be freed 
  } else if(param->type == SLICE_ALG_SM_V0_NVS){
   // no memory to be freed 
  } else if(param->type == SLICE_ALG_SM_V0_SCN19){
   // no memory to be freed 
  } else if(param->type == SLICE_ALG_SM_V0_EDF){
    if(param->edf.len_over > 0 ){
      assert(param->edf.over != NULL);
      free(param->edf.over);
    }
  } else {
    assert("Unknown slice param");
  }
}

static
void free_ul_dl_slice_conf(ul_dl_slice_conf_t* conf)
{
  assert(conf != NULL);

  if(conf->len_sched_name > 0){
    free(conf->sched_name); 
  }

  if(conf->len_slices > 0){
    assert(conf->slices != NULL);  
   
    for(size_t i = 0; i < conf->len_slices; ++i){

      slice_t* slice = &conf->slices[i];
      if(slice->len_label > 0){
        assert(slice->label != NULL);
        free(slice->label);
      } 

      if(slice->len_sched > 0){
        assert(slice->sched != NULL);
        free(slice->sched);
      }
      free_slice_param(&slice->params);
    }
    free(conf->slices);
  }
}

static
void free_slice_conf(slice_conf_t* conf)
{
 assert(conf != NULL);
 free_ul_dl_slice_conf(&conf->ul);
 free_ul_dl_slice_conf(&conf->dl);
}

static
void free_ue_slice_conf(ue_slice_conf_t* conf)
{
  assert(conf != NULL);
  if(conf->len_ue_slice > 0){
    assert(conf->ues != NULL);
    free(conf->ues);
  }
}

void free_slice_ind_msg(slice_ind_msg_t* msg)
{
  assert(msg != NULL);
  free_slice_conf(&msg->slice_conf);
  free_ue_slice_conf(&msg->ue_slice_conf);
}

static
void free_del_slice(del_slice_conf_t* slice)
{
  assert(slice != NULL);
  if(slice->len_dl > 0){
    assert(slice->dl != NULL);
    free(slice->dl);
  }

  if(slice->len_ul > 0){
    assert(slice->ul != NULL);
    free(slice->ul);
  }
} 

void slice_free_ctrl_msg(slice_ctrl_msg_t* msg)
{
  assert(msg != NULL);

  if(msg->type == SLICE_CTRL_SM_V0_ADD ){
    free_slice_conf(&msg->add_mod_slice);
  } else if(msg->type == SLICE_CTRL_SM_V0_DEL){
    free_del_slice(&msg->del_slice);
  } else if(msg->type == SLICE_CTRL_SM_V0_UE_SLICE_ASSOC){
    free_ue_slice_conf(&msg->ue_slice);
  } else {
    assert(0 != 0 && "Unknown slice control type");
  }
}

void slice_free_ctrl_out(slice_ctrl_out_t* out)
{
  assert(out != NULL);
  if(out->len_diag > 0){
    assert(out->diagnostic != NULL);
    free(out->diagnostic);
  }
}

//////////////////////////////////////
// RIC Indication Header 
/////////////////////////////////////


void free_slice_ind_hdr(slice_ind_hdr_t* src)
{
  assert(src != NULL);
  (void)src;
} 





//////////////////////////////////////
// RIC Indication Message 
/////////////////////////////////////

static
bool eq_static_slice(static_slice_t const* m0, static_slice_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  return m0->pos_high == m1->pos_high 
        && m0->pos_low == m1->pos_low;
}

static
bool eq_nvs_rate(nvs_rate_t const* m0, nvs_rate_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  return m0->mbps_reference == m1->mbps_reference 
    && m0->mbps_required == m1->mbps_required;
}


static
bool eq_nvs_cap(nvs_capacity_t const* m0, nvs_capacity_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  return m0->pct_reserved == m1->pct_reserved;
}

static
bool eq_nvs_slice(nvs_slice_t const* m0, nvs_slice_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if(m0->conf != m1->conf)
    return false;

  bool ret = false;
  if(m0->conf == SLICE_SM_NVS_V0_RATE){
    ret = eq_nvs_rate(&m0->rate, &m1->rate);
  } else if (m0->conf == SLICE_SM_NVS_V0_CAPACITY){
    ret = eq_nvs_cap(&m0->capacity, &m1->capacity);
  } else {
    assert("Unknown type");
  }

  return ret;
}

bool eq_scn19_on_demand(scn19_on_demand_t const* m0, scn19_on_demand_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  return m0->log_delta == m1->log_delta 
        && m0->pct_reserved == m1->pct_reserved
        && m0->tau == m1->tau;
}

static
bool eq_scn19_slice(scn19_slice_t const* m0, scn19_slice_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if(m0->conf != m1->conf)
    return false;

  bool ret = false;
  if(m0->conf == SLICE_SCN19_SM_V0_DYNAMIC){
    ret = eq_nvs_rate(&m0->dynamic, &m1->dynamic); 

    if(ret == false)
      printf("Returning false from scn19 dyn\n");

  } else if(m0->conf == SLICE_SCN19_SM_V0_FIXED ){
    ret = eq_static_slice(&m0->fixed, &m1->fixed); 

    if(ret == false)
      printf("Returning false from scn19 static\n");

  } else if (m0->conf == SLICE_SCN19_SM_V0_ON_DEMAND ){
    ret = eq_scn19_on_demand(&m0->on_demand, &m1->on_demand);

     if(ret == false)
      printf("Returning false from scn19 on demand\n");
  }

     return ret; 
}

static
bool eq_slice_params(slice_params_t const* m0, slice_params_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if(m0->type != m1->type)
    return false;

  bool ret = false;
  if(m0->type == SLICE_ALG_SM_V0_STATIC){
    ret = eq_static_slice(&m0->sta, &m1->sta);
  } else if(m0->type == SLICE_ALG_SM_V0_NVS){
    ret = eq_nvs_slice(&m0->nvs, &m1->nvs); 
  } else if(m0->type == SLICE_ALG_SM_V0_SCN19){
    ret = eq_scn19_slice(&m0->scn19, &m1->scn19);
  } else if(m0->type == SLICE_ALG_SM_V0_EDF){
    assert("Not implemented");
  } else {
    assert("Unknown type");
  }

  return ret;
}

static
bool eq_slice(slice_t const* m0, slice_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if(m0->id != m1->id)
    return false;

  if(m0->len_label != m1->len_label)
    return false;

  if(memcmp(m0->label, m1->label, m0->len_label) != 0)
    return false;

  if(m0->len_sched != m1->len_sched)
    return false;

  return eq_slice_params(&m0->params, &m1->params); 
}

static
bool eq_ul_dl_slice_conf(ul_dl_slice_conf_t const* m0, ul_dl_slice_conf_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if(m0->len_sched_name != m1->len_sched_name)
    return false;

  if(m0->len_slices != m1->len_slices)
    return false;

  if(memcmp(m0->sched_name, m1->sched_name, m0->len_sched_name) != 0)
    return false;

  for(size_t i = 0; i < m0->len_slices; ++i) {
    if(eq_slice(&m0->slices[i], &m1->slices[i] ) == false){
      printf("eq slice returning false \n");
      return false;
    }
  }

  return true;
}

static
bool eq_slice_conf(slice_conf_t const* m0, slice_conf_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  return eq_ul_dl_slice_conf(&m0->ul, &m1->ul) && eq_ul_dl_slice_conf(&m0->dl, &m1->dl); 
}

static
bool eq_ue_slice_assoc(ue_slice_assoc_t const* m0, ue_slice_assoc_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  return m0->dl_id == m1->dl_id
        && m0->rnti == m1->rnti
        && m0->ul_id == m1->ul_id;
}

static
bool eq_ue_slice_conf(ue_slice_conf_t const* m0, ue_slice_conf_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if(m0->len_ue_slice != m1->len_ue_slice)
    return false;

  for(size_t i = 0; i < m0->len_ue_slice; ++i){
    if(eq_ue_slice_assoc(&m0->ues[i], &m1->ues[i]) == false)
      return false;
  }

  return true;
}


bool eq_slice_ind_msg(slice_ind_msg_t const* m0, slice_ind_msg_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  return eq_slice_conf(&m0->slice_conf, &m1->slice_conf) 
         && eq_ue_slice_conf(&m0->ue_slice_conf, &m1->ue_slice_conf)
         && m0->tstamp == m1->tstamp;
}



static
scn19_slice_t cp_scn19_slice(scn19_slice_t const* src)
{
  assert(src != NULL);
  scn19_slice_t dst = {0};

  dst.conf = src->conf;
  if(dst.conf == SLICE_SCN19_SM_V0_DYNAMIC){
    dst.dynamic.mbps_reference = src->dynamic.mbps_reference;
    dst.dynamic.mbps_required = src->dynamic.mbps_required;
  } else if(dst.conf == SLICE_SCN19_SM_V0_FIXED ) {
    dst.fixed.pos_high = src->fixed.pos_high;
    dst.fixed.pos_low = src->fixed.pos_low;
  } else if(dst.conf == SLICE_SCN19_SM_V0_ON_DEMAND ) {
    dst.on_demand.log_delta = src->on_demand.log_delta; 
    dst.on_demand.pct_reserved = src->on_demand.pct_reserved; 
    dst.on_demand.tau = src->on_demand.tau;
  } else {
    assert(0!=0 && "Unknown type");
  }

  return dst;
}

static
slice_params_t cp_slice_params(slice_params_t const* src)
{
  assert(src != NULL);

  slice_params_t dst = {0};

  assert(src->type != SLICE_ALG_SM_V0_EDF && "Not supported yet");
  dst.type = src->type;

  if(src->type == SLICE_ALG_SM_V0_STATIC  ){
    dst.sta.pos_high = src->sta.pos_high;
    dst.sta.pos_low = src->sta.pos_low; 
  } else if(src->type == SLICE_ALG_SM_V0_NVS ){
    if(src->nvs.conf == SLICE_SM_NVS_V0_RATE){
      dst.nvs.conf =  SLICE_SM_NVS_V0_RATE;
      dst.nvs.rate.mbps_reference = src->nvs.rate.mbps_reference;
      dst.nvs.rate.mbps_required = src->nvs.rate.mbps_required;
    } else if(src->nvs.conf == SLICE_SM_NVS_V0_CAPACITY){
      dst.nvs.conf =  SLICE_SM_NVS_V0_CAPACITY;
      dst.nvs.capacity.pct_reserved = src->nvs.capacity.pct_reserved;
    } else {
      assert(0!=0 && "Not implemented or unknown");
    }
  } else if(src->type == SLICE_ALG_SM_V0_SCN19){
      dst.scn19 = cp_scn19_slice(&src->scn19);
  } else {
    assert(0!=0 && "Not implemented or unknown");
  }

//  assert(0!=0 && "Do it properly as the layout does not match" );
//  memcpy(&dst, m0, sizeof(slice_params_t));

  assert(eq_slice_params(&dst, src) == true);

  return dst;
}

static
slice_t cpy_slice(slice_t const* src)
{
  assert(src != NULL);
  slice_t dst = {0}; 

  dst.id = src->id;
  dst.len_label = src->len_label;
  if(dst.len_label > 0){
    dst.label = malloc(src->len_label);
    memcpy(dst.label, src->label, src->len_label);
  }

  dst.len_sched = src->len_sched;
  if(src->len_sched > 0){
    dst.sched = malloc(src->len_sched);
    assert(dst.sched != NULL);
    memcpy(dst.sched, src->sched, src->len_sched);
  }

  dst.params = cp_slice_params(&src->params);

  assert(eq_slice(src, &dst) == true);

  return dst;
}

static
ul_dl_slice_conf_t cpy_ul_dl_slice_conf(ul_dl_slice_conf_t const* src)
{
  assert(src != NULL);
  ul_dl_slice_conf_t dst = {0};

  dst.len_sched_name = src->len_sched_name;
  dst.len_slices = src->len_slices;

  if(src->len_sched_name > 0){
    dst.sched_name = malloc(src->len_sched_name);
    assert(dst.sched_name != NULL && "memory exhausted");
    memcpy(dst.sched_name, src->sched_name, src->len_sched_name);
  }

  dst.slices = calloc(src->len_slices, sizeof(slice_t));
  assert(dst.slices != NULL && "Memory exhausted");

  for(size_t i = 0; i < src->len_slices; ++i){
    dst.slices[i] = cpy_slice(&src->slices[i]); 
  }

  assert(eq_ul_dl_slice_conf(src, &dst) == true);
  return dst;
}

static
slice_conf_t cp_slice_conf(slice_conf_t const* src)
{
  assert(src != NULL);
  slice_conf_t dst = {0};

  dst.dl = cpy_ul_dl_slice_conf(&src->dl); 
  dst.ul = cpy_ul_dl_slice_conf(&src->ul); 

  assert(eq_slice_conf(src, &dst));

  return dst;
}

static
ue_slice_assoc_t cp_ue_slice_assoc(ue_slice_assoc_t const* src)
{
  assert(src != NULL);
  ue_slice_assoc_t dst = {0};

  dst.ul_id = src->ul_id;
  dst.dl_id = src->dl_id;
  dst.rnti = src->rnti;

  return dst;
}

static
ue_slice_conf_t cp_ue_slice_conf(ue_slice_conf_t const* src)
{
  assert(src != NULL);
  ue_slice_conf_t dst = {0}; 

  dst.len_ue_slice = src->len_ue_slice; 
  if(src->len_ue_slice > 0){
    dst.ues = calloc(src->len_ue_slice, sizeof(ue_slice_assoc_t)); 
    assert(dst.ues != NULL);
  }

  for(size_t i =0; i < src->len_ue_slice; ++i){
    dst.ues[i] = cp_ue_slice_assoc(&src->ues[i]);
  }

  return dst;
}

slice_ind_msg_t cp_slice_ind_msg(slice_ind_msg_t* src)
{
  assert(src != NULL);

  slice_ind_msg_t out = {0};

  out.slice_conf = cp_slice_conf(&src->slice_conf);
  out.ue_slice_conf = cp_ue_slice_conf(&src->ue_slice_conf);
  out.tstamp = src->tstamp;

  assert(eq_slice_ind_msg(src, &out) );

  return out;
}


//////////////////////////////////////
// RIC Call Process ID 
/////////////////////////////////////

void free_slice_call_proc_id( slice_call_proc_id_t* src)
{
  if(src == NULL) 
    return;

  assert(0!=0 && "Not implemented");
}

//////////////////////////////////////
// RIC Control Header 
/////////////////////////////////////

void free_slice_ctrl_hdr(slice_ctrl_hdr_t* src)
{
  assert(src != NULL);
  (void)src;
} 

slice_ctrl_hdr_t cp_slice_ctrl_hdr(slice_ctrl_hdr_t* src)
{
  assert(src != NULL);
  slice_ctrl_hdr_t dst = {.dummy = src->dummy};
  return dst; 
}

bool eq_slice_ctrl_hdr(slice_ctrl_hdr_t const* m0, slice_ctrl_hdr_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  return m0->dummy == m1->dummy;
}


//////////////////////////////////////
// RIC Control Message 
/////////////////////////////////////

static
void free_del_slice_conf(del_slice_conf_t* conf)
{
  assert(conf != NULL);
  
  if(conf->len_dl > 0){
    assert(conf->dl != NULL);
    free(conf->dl);
  }

  if(conf->len_ul > 0){
    assert(conf->ul != NULL);
    free(conf->ul);
  }

}

void free_slice_ctrl_msg(slice_ctrl_msg_t* src)
{
  assert(src != NULL);

  if(src->type == SLICE_CTRL_SM_V0_ADD){
    free_slice_conf(&src->add_mod_slice);
  }else if(src->type == SLICE_CTRL_SM_V0_DEL){
    free_del_slice_conf(&src->del_slice);
  } else if(src->type == SLICE_CTRL_SM_V0_UE_SLICE_ASSOC){
    free_ue_slice_conf(&src->ue_slice);
  } else {
    assert(0!=0 && "Unknown type");
  }
} 

static
del_slice_conf_t cp_del_slice(del_slice_conf_t const* src)
{
  assert(src != NULL);

  del_slice_conf_t dst = {0};

  if(src->len_dl > 0){
    dst.len_dl = src->len_dl;
    dst.dl = calloc(src->len_dl, sizeof(uint32_t));
    assert(dst.dl != NULL && "Memory exhausted");
  }

  if(src->len_ul > 0){
    dst.len_ul = src->len_ul;
    dst.ul = calloc(dst.len_ul, sizeof(uint32_t)); 
    assert(dst.ul != NULL && "Memory exhausted");
  }

  return dst;
}

slice_ctrl_msg_t cp_slice_ctrl_msg(slice_ctrl_msg_t* src)
{
  assert(src != NULL);

  slice_ctrl_msg_t dst = {0};
  dst.type = src->type;

  if(src->type == SLICE_CTRL_SM_V0_ADD){
   dst.add_mod_slice = cp_slice_conf(&src->add_mod_slice);
  } else if(src->type == SLICE_CTRL_SM_V0_DEL){
    dst.del_slice = cp_del_slice(&src->del_slice);
  } else if(src->type == SLICE_CTRL_SM_V0_UE_SLICE_ASSOC){
    dst.ue_slice = cp_ue_slice_conf(&src->ue_slice);
  } else {
    assert(0!=0 && "Unknown type");
  }

  return dst;
}

static 
bool eq_del_slice(del_slice_conf_t const* m0, del_slice_conf_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if(m0->len_dl != m1->len_dl) return false;

  for(size_t i = 0; i < m0->len_dl; ++i){
    if(m0->dl[i] != m1->dl[i]) return false;
  }

  if(m0->len_ul != m1->len_ul) return false;

  for(size_t i = 0; i < m0->len_ul; ++i){
    if(m0->ul[i] != m1->ul[i]) return false;
  }

  return true;
}

bool eq_slice_ctrl_msg(slice_ctrl_msg_t const* m0, slice_ctrl_msg_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  bool ans= false;
  if(m0->type == SLICE_CTRL_SM_V0_ADD){
    ans = eq_slice_conf(&m0->add_mod_slice, &m1->add_mod_slice);    
  } else if(m0->type == SLICE_CTRL_SM_V0_DEL){
    ans = eq_del_slice ( &m0->del_slice, &m1->del_slice); 
  } else if(m0->type == SLICE_CTRL_SM_V0_UE_SLICE_ASSOC){
    ans = eq_ue_slice_conf(&m0->ue_slice, &m1->ue_slice);
  } else {
    assert(0!=0 && "Unknown type");
  }

  return ans;
}

//////////////////////////////////////
// RIC Control Outcome 
/////////////////////////////////////


void free_slice_ctrl_out(slice_ctrl_out_t* src)
{
  assert(src != NULL);

  if(src->len_diag > 0){
    assert(src->diagnostic != NULL);
    free(src->diagnostic);
  }
} 

slice_ctrl_out_t cp_slice_ctrl_out(slice_ctrl_out_t* src)
{
  assert(src != NULL);
  slice_ctrl_out_t dst = {.len_diag = src->len_diag }; 
  if(src->len_diag > 0){
    dst.diagnostic = malloc(src->len_diag);
    assert(dst.diagnostic != NULL);
    memcpy(dst.diagnostic, src->diagnostic, src->len_diag);
  }

  return dst;
}

bool eq_slice_ctrl_out(slice_ctrl_out_t* m0, slice_ctrl_out_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if(m0->len_diag != m1->len_diag) return false;

  if(m0->len_diag > 0){
   return (memcmp(m0->diagnostic, m1->diagnostic, m0->len_diag) == 0);
  }

  return true;
}



