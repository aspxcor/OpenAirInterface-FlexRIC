/* file: mcs control information elements defination
usage: flexRIC for control downlink mcs value
author: Yuxiang Lin
date: 2022.6.6 */

#include "mcs_control_ie.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// free mcs control indication messages' memory to avoid memory leak
static
void free_ul_dl_mcs_conf(ul_dl_mcs_conf_t* conf)
{
  assert(conf != NULL);

  if(conf->len_mcss > 0){
    free(conf->mcss); 
  }
}

static
void free_mcs_conf(mcs_conf_t* conf)
{
 assert(conf != NULL);
 free_ul_dl_mcs_conf(&conf->ul);
 free_ul_dl_mcs_conf(&conf->dl);
}

static
void free_ue_mcs_conf(ue_mcs_conf_t* conf)
{
  assert(conf != NULL);
  if(conf->len_ue_mcs > 0){
    assert(conf->ues != NULL);
    free(conf->ues);
  }
}

void free_mcs_ind_msg(mcs_ind_msg_t* msg)
{
    assert(msg != NULL);
    free_mcs_conf(&msg->mcs_conf);
    free_ue_mcs_conf(&msg->ue_mcs_conf);
}

static
void free_del_mcs(del_mcs_conf_t* mcs)
{
  assert(mcs != NULL);
  if(mcs->len_dl > 0){
    assert(mcs->dl != NULL);
    free(mcs->dl);
  }

  if(mcs->len_ul > 0){
    assert(mcs->ul != NULL);
    free(mcs->ul);
  }
} 

void mcs_free_ctrl_msg(mcs_ctrl_msg_t* msg)
{
  assert(msg != NULL);

  if(msg->type == MCS_CTRL_SM_V0_ADD ){
    free_mcs_conf(&msg->add_mod_mcs);
  } else if(msg->type == MCS_CTRL_SM_V0_DEL){
    free_del_mcs(&msg->del_mcs);
  } else if(msg->type == MCS_CTRL_SM_V0_UE_MCS_ASSOC){
    free_ue_mcs_conf(&msg->ue_mcs);
  } else {
    assert(0 != 0 && "Unknown mcs control type");
  }
}

void mcs_free_ctrl_out(mcs_ctrl_out_t* out)
{
  assert(out != NULL);
  if(out->len_diag > 0){
    assert(out->diagnostic != NULL);
    free(out->diagnostic);
  }
}

void free_mcs_ind_hdr(mcs_ind_hdr_t* src)
{
  assert(src != NULL);
  (void)src;
} 


// identify whether two mcs control messages are equal
static
bool eq_static_mcs(static_mcs_t const* m0, static_mcs_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  return m0->mcs_value == m1->mcs_value;
}

static
bool eq_mcs_params(mcs_params_t const* m0, mcs_params_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if(m0->type != m1->type)
    return false;
  if(m0->id != m1->id)
    return false;

  bool ret = false;
  if(m0->type == MCS_ALG_SM_V0_STATIC){
    ret = eq_static_mcs(&m0->sta, &m1->sta);
  } else if(m0->type == MCS_ALG_SM_V0_ORIGIN){
    ret = true; 
  } else {
    assert("Unknown type");
  }

  return ret;
}

static
bool eq_ul_dl_mcs_conf(ul_dl_mcs_conf_t const* m0, ul_dl_mcs_conf_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if(m0->len_mcss != m1->len_mcss)
    return false;

  for(size_t i = 0; i < m0->len_mcss; ++i) {
    if(eq_mcs_params(&m0->mcss[i], &m1->mcss[i] ) == false){
      printf("eq mcs returning false \n");
      return false;
    }
  }

  return true;
}

static
bool eq_mcs_conf(mcs_conf_t const* m0, mcs_conf_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  return eq_ul_dl_mcs_conf(&m0->ul, &m1->ul) && eq_ul_dl_mcs_conf(&m0->dl, &m1->dl); 
}

static
bool eq_ue_mcs_assoc(ue_mcs_assoc_t const* m0, ue_mcs_assoc_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  return m0->dl_id == m1->dl_id
        && m0->rnti == m1->rnti
        && m0->ul_id == m1->ul_id;
}

static
bool eq_ue_mcs_conf(ue_mcs_conf_t const* m0, ue_mcs_conf_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if(m0->len_ue_mcs != m1->len_ue_mcs)
    return false;

  for(size_t i = 0; i < m0->len_ue_mcs; ++i){
    if(eq_ue_mcs_assoc(&m0->ues[i], &m1->ues[i]) == false)
      return false;
  }

  return true;
}


bool eq_mcs_ind_msg(mcs_ind_msg_t const* m0, mcs_ind_msg_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  return eq_mcs_conf(&m0->mcs_conf, &m1->mcs_conf) 
         && eq_ue_mcs_conf(&m0->ue_mcs_conf, &m1->ue_mcs_conf)
         && m0->tstamp == m1->tstamp;
}

// copy the mcs control indication message
static
mcs_params_t cp_mcs_params(mcs_params_t const* src)
{
  assert(src != NULL);

  mcs_params_t dst = {0};
  dst.id = src->id;
  dst.type = src->type;

  if(src->type == MCS_ALG_SM_V0_STATIC  ){
    dst.sta.mcs_value = src->sta.mcs_value;
  } else if(src->type == MCS_ALG_SM_V0_ORIGIN ){
    // copy nothing
  } else {
    assert("Unknown type");
  }

//  assert(0!=0 && "Do it properly as the layout does not match" );
//  memcpy(&dst, m0, sizeof(mcs_params_t));

  assert(eq_mcs_params(&dst, src) == true);

  return dst;
}

static
ul_dl_mcs_conf_t cpy_ul_dl_mcs_conf(ul_dl_mcs_conf_t const* src)
{
  assert(src != NULL);
  ul_dl_mcs_conf_t dst = {0};

  dst.len_mcss = src->len_mcss;

  dst.mcss = calloc(src->len_mcss, sizeof(mcs_params_t));
  assert(dst.mcss != NULL && "Memory exhausted");

  for(size_t i = 0; i < src->len_mcss; ++i){
    dst.mcss[i] = cp_mcs_params(&src->mcss[i]); 
  }

  assert(eq_ul_dl_mcs_conf(src, &dst) == true);
  return dst;
}

static
mcs_conf_t cp_mcs_conf(mcs_conf_t const* src)
{
  assert(src != NULL);
  mcs_conf_t dst = {0};

  dst.dl = cpy_ul_dl_mcs_conf(&src->dl); 
  dst.ul = cpy_ul_dl_mcs_conf(&src->ul); 

  assert(eq_mcs_conf(src, &dst));

  return dst;
}

static
ue_mcs_assoc_t cp_ue_mcs_assoc(ue_mcs_assoc_t const* src)
{
  assert(src != NULL);
  ue_mcs_assoc_t dst = {0};

  dst.ul_id = src->ul_id;
  dst.dl_id = src->dl_id;
  dst.rnti = src->rnti;

  return dst;
}

static
ue_mcs_conf_t cp_ue_mcs_conf(ue_mcs_conf_t const* src)
{
  assert(src != NULL);
  ue_mcs_conf_t dst = {0}; 

  dst.len_ue_mcs = src->len_ue_mcs; 
  if(src->len_ue_mcs > 0){
    dst.ues = calloc(src->len_ue_mcs, sizeof(ue_mcs_assoc_t)); 
    assert(dst.ues != NULL);
  }

  for(size_t i =0; i < src->len_ue_mcs; ++i){
    dst.ues[i] = cp_ue_mcs_assoc(&src->ues[i]);
  }

  return dst;
}

mcs_ind_msg_t cp_mcs_ind_msg(mcs_ind_msg_t* src)
{
  assert(src != NULL);

  mcs_ind_msg_t out = {0};

  out.mcs_conf = cp_mcs_conf(&src->mcs_conf);
  out.ue_mcs_conf = cp_ue_mcs_conf(&src->ue_mcs_conf);
  out.tstamp = src->tstamp;

  assert(eq_mcs_ind_msg(src, &out) );

  return out;
}


//////////////////////////////////////
// RIC Call Process ID 
/////////////////////////////////////

void free_mcs_call_proc_id( mcs_call_proc_id_t* src)
{
  if(src == NULL) 
    return;

  assert(0!=0 && "Not implemented");
}

//////////////////////////////////////
// RIC Control Header 
/////////////////////////////////////

void free_mcs_ctrl_hdr(mcs_ctrl_hdr_t* src)
{
  assert(src != NULL);
  (void)src;
} 

mcs_ctrl_hdr_t cp_mcs_ctrl_hdr(mcs_ctrl_hdr_t* src)
{
  assert(src != NULL);
  mcs_ctrl_hdr_t dst = {.dummy = src->dummy};
  return dst; 
}

bool eq_mcs_ctrl_hdr(mcs_ctrl_hdr_t const* m0, mcs_ctrl_hdr_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  return m0->dummy == m1->dummy;
}


//////////////////////////////////////
// RIC Control Message 
/////////////////////////////////////

static
void free_del_mcs_conf(del_mcs_conf_t* conf)
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

void free_mcs_ctrl_msg(mcs_ctrl_msg_t* src)
{
  assert(src != NULL);

  if(src->type == MCS_CTRL_SM_V0_ADD){
    free_mcs_conf(&src->add_mod_mcs);
  }else if(src->type == MCS_CTRL_SM_V0_DEL){
    free_del_mcs_conf(&src->del_mcs);
  } else if(src->type == MCS_CTRL_SM_V0_UE_MCS_ASSOC){
    free_ue_mcs_conf(&src->ue_mcs);
  } else {
    assert(0!=0 && "Unknown type");
  }
} 

static
del_mcs_conf_t cp_del_mcs(del_mcs_conf_t const* src)
{
  assert(src != NULL);

  del_mcs_conf_t dst = {0};

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

mcs_ctrl_msg_t cp_mcs_ctrl_msg(mcs_ctrl_msg_t* src)
{
  assert(src != NULL);

  mcs_ctrl_msg_t dst = {0};
  dst.type = src->type;

  if(src->type == MCS_CTRL_SM_V0_ADD){
   dst.add_mod_mcs = cp_mcs_conf(&src->add_mod_mcs);
  } else if(src->type == MCS_CTRL_SM_V0_DEL){
    dst.del_mcs = cp_del_mcs(&src->del_mcs);
  } else if(src->type == MCS_CTRL_SM_V0_UE_MCS_ASSOC){
    dst.ue_mcs = cp_ue_mcs_conf(&src->ue_mcs);
  } else {
    assert(0!=0 && "Unknown type");
  }

  return dst;
}

static 
bool eq_del_mcs(del_mcs_conf_t const* m0, del_mcs_conf_t const* m1)
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

bool eq_mcs_ctrl_msg(mcs_ctrl_msg_t const* m0, mcs_ctrl_msg_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  bool ans= false;
  if(m0->type == MCS_CTRL_SM_V0_ADD){
    ans = eq_mcs_conf(&m0->add_mod_mcs, &m1->add_mod_mcs);    
  } else if(m0->type == MCS_CTRL_SM_V0_DEL){
    ans = eq_del_mcs ( &m0->del_mcs, &m1->del_mcs); 
  } else if(m0->type == MCS_CTRL_SM_V0_UE_MCS_ASSOC){
    ans = eq_ue_mcs_conf(&m0->ue_mcs, &m1->ue_mcs);
  } else {
    assert(0!=0 && "Unknown type");
  }

  return ans;
}

//////////////////////////////////////
// RIC Control Outcome 
/////////////////////////////////////


void free_mcs_ctrl_out(mcs_ctrl_out_t* src)
{
  assert(src != NULL);
  //printf("For debug!\n");

  if(src->len_diag > 0){
    assert(src->diagnostic != NULL);
    //printf("length: %d",src->len_diag);
    //printf("diag: %s", src->diagnostic);
    //free(src->diagnostic);
    //printf("For debug2!\n");
  }
  //printf("For debug3!\n");
} 

mcs_ctrl_out_t cp_mcs_ctrl_out(mcs_ctrl_out_t* src)
{
  assert(src != NULL);
  mcs_ctrl_out_t dst = {.len_diag = src->len_diag }; 
  if(src->len_diag > 0){
    dst.diagnostic = malloc(src->len_diag);
    assert(dst.diagnostic != NULL);
    memcpy(dst.diagnostic, src->diagnostic, src->len_diag);
  }

  return dst;
}

bool eq_mcs_ctrl_out(mcs_ctrl_out_t* m0, mcs_ctrl_out_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if(m0->len_diag != m1->len_diag) return false;

  if(m0->len_diag > 0){
   return (memcmp(m0->diagnostic, m1->diagnostic, m0->len_diag) == 0);
  }

  return true;
}