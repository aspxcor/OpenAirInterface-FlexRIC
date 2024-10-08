/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */


#include "../src/ric/near_ric_api.h"
#include "../src/agent/e2_agent_api.h"

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static
int64_t time_now_us(void)
{
  struct timespec tms;

  /* The C11 way */
  /* if (! timespec_get(&tms, TIME_UTC))  */

  /* POSIX.1-2008 way */
  if (clock_gettime(CLOCK_REALTIME,&tms)) {
    return -1;
  }
  /* seconds, multiplied with 1 million */
  int64_t micros = tms.tv_sec * 1000000;
  /* Add full microseconds */
  micros += tms.tv_nsec/1000;
  /* round up if necessary */
  if (tms.tv_nsec % 1000 >= 500) {
    ++micros;
  }
  return micros;
}



static
void fill_static_slice(static_slice_t* sta)
{
  assert(sta != NULL);

  sta->pos_high = abs(rand()%25);
  sta->pos_low = abs(rand()%25);
}


static
void fill_nvs_slice(nvs_slice_t* nvs)
{
  assert(nvs != NULL);

  const uint32_t type = abs(rand() % SLICE_SM_NVS_V0_END);

  if(type == SLICE_SM_NVS_V0_RATE ){
    nvs->conf = SLICE_SM_NVS_V0_RATE; 
    nvs->rate.mbps_reference = 0.8; 
//      10.0*((float)rand()/(float)RAND_MAX); 
    nvs->rate.mbps_required = 10.0;
    //*((float)rand()/(float)RAND_MAX); 
  } else if(type ==SLICE_SM_NVS_V0_CAPACITY ){
    nvs->conf = SLICE_SM_NVS_V0_CAPACITY; 
    nvs->capacity.pct_reserved = 15.0;
    //*((float)rand()/(float)RAND_MAX);
  } else {
    assert(0!=0 && "Unknown type");
  }

}

static
void fill_scn19_slice(scn19_slice_t* scn19)
{
  assert(scn19 != NULL);

  const uint32_t type = abs(rand()% SLICE_SCN19_SM_V0_END);

  if(type == SLICE_SCN19_SM_V0_DYNAMIC ){
    scn19->conf = SLICE_SCN19_SM_V0_DYNAMIC ;
    scn19->dynamic.mbps_reference = 10.0 * fabs((float)rand()/(float)RAND_MAX); 
    scn19->dynamic.mbps_required = 8.0 * fabs((float)rand()/(float)RAND_MAX); 
  } else if(type == SLICE_SCN19_SM_V0_FIXED ) {
    scn19->conf = SLICE_SCN19_SM_V0_FIXED; 
    scn19->fixed.pos_high = abs(rand()%14);
    scn19->fixed.pos_low = abs(rand()%10);
  } else if(type ==SLICE_SCN19_SM_V0_ON_DEMAND){
    scn19->conf = SLICE_SCN19_SM_V0_ON_DEMAND;
    scn19->on_demand.log_delta_byte = abs(rand()%121);
    scn19->on_demand.log_delta = 1.0 * fabs((float)rand()/RAND_MAX);
    scn19->on_demand.tau = abs(rand()%256);
    scn19->on_demand.pct_reserved = fabs((float)rand()/(float)RAND_MAX);
  } else {
    assert(0 != 0 && "Unknown type!!");
  }

}

static 
void fill_edf_slice(edf_slice_t* edf)
{
  assert(edf != NULL);

  int mod = 32;
  edf->deadline = abs(rand()%mod);
  edf->guaranteed_prbs = abs(rand()%mod);
  edf->max_replenish = abs(rand()%mod);

  edf->len_over = abs(rand()%mod);

  if(edf->len_over > 0){
    edf->over = calloc(edf->len_over, sizeof(uint32_t));
    assert(edf->over != NULL && "Memory exhausted");
  }

  for(uint32_t i = 0; i < edf->len_over; ++i){
    edf->over[i] = abs(rand()%mod);
  }
}

static
void fill_ul_dl_slice(ul_dl_slice_conf_t* slice)
{
  assert(slice != NULL);

  char const* name = "MY SLICE";
  slice->len_sched_name = strlen(name);
  slice->sched_name = malloc(strlen(name));
  assert(slice->sched_name != NULL && "memory exhausted");
  memcpy(slice->sched_name, name, strlen(name));

  slice->len_slices = 1; 
  //slice->len_slices = abs(rand()%8);

  if(slice->len_slices > 0){
    slice->slices = calloc(slice->len_slices, sizeof(slice_t));
    assert(slice->slices != NULL && "memory exhausted");
  }

  for(uint32_t i = 0; i < slice->len_slices; ++i){
    slice->slices[i].id = abs(rand()%1024);
    slice_t* s = &slice->slices[i];

    const char* label = "This is my label";
    s->len_label = strlen(label);
    s->label = malloc(s->len_label);
    assert(s->label != NULL && "Memory exhausted");
    memcpy(s->label, label, s->len_label );

    const char* sched_str = "Scheduler string";
    s->len_sched = strlen(sched_str); 
    s->sched = malloc(s->len_sched);
    assert(s->sched != NULL && "Memory exhausted");
    memcpy(s->sched, sched_str, s->len_sched);

    uint32_t type = abs(rand()% SLICE_ALG_SM_V0_END);

    if(type ==  SLICE_ALG_SM_V0_EDF || type == SLICE_ALG_SM_V0_NONE )
      type = SLICE_ALG_SM_V0_STATIC; 

 //   type = SLICE_ALG_SM_V0_STATIC; 
 //   type = SLICE_ALG_SM_V0_SCN19;

    if(type == SLICE_ALG_SM_V0_NONE ){
      s->params.type =SLICE_ALG_SM_V0_NONE; 
    } else if (type == SLICE_ALG_SM_V0_STATIC ){
      s->params.type = SLICE_ALG_SM_V0_STATIC; 
      fill_static_slice(&s->params.sta);
    } else if (type == SLICE_ALG_SM_V0_NVS){
      s->params.type =  SLICE_ALG_SM_V0_NVS; 
      fill_nvs_slice(&s->params.nvs);
    } else if (type == SLICE_ALG_SM_V0_SCN19) {
      s->params.type = SLICE_ALG_SM_V0_SCN19; 
      fill_scn19_slice(&s->params.scn19);
    } else if (type == SLICE_ALG_SM_V0_EDF){
      s->params.type =  SLICE_ALG_SM_V0_EDF; 
      fill_edf_slice(&s->params.edf);
    } else {
      assert(0 != 0 && "Unknown type encountered");
    }
  }
}

static
void fill_slice_conf(slice_conf_t* conf)
{
  assert(conf != NULL);

  fill_ul_dl_slice(&conf->ul);
  fill_ul_dl_slice(&conf->dl);
}

static
void fill_ue_slice_conf(ue_slice_conf_t* conf)
{
  assert(conf != NULL);
  conf->len_ue_slice = abs(rand()%10);
  if(conf->len_ue_slice > 0){
    conf->ues = calloc(conf->len_ue_slice, sizeof(ue_slice_assoc_t));
    assert(conf->ues);
  }

  for(uint32_t i = 0; i < conf->len_ue_slice; ++i){
    conf->ues[i].rnti = abs(rand()%1024);  
    conf->ues[i].dl_id = abs(rand()%16); 
    conf->ues[i].ul_id = abs(rand()%16); 
  }

}

static
void fill_slice_ind_data(slice_ind_data_t* ind_msg)
{
  assert(ind_msg != NULL);

  srand(time(0));

  fill_slice_conf(&ind_msg->msg.slice_conf);
  fill_ue_slice_conf(&ind_msg->msg.ue_slice_conf);
  ind_msg->msg.tstamp = time_now_us();
}

static
void fill_static_mcs(static_mcs_t* sta)
{
  assert(sta != NULL);

  sta->mcs_value = abs(rand()%25);
}


static
void fill_ul_dl_mcs(ul_dl_mcs_conf_t* mcs)
{
  assert(mcs != NULL);

  mcs->len_mcss = 1; 
  //mcs->len_mcss = abs(rand()%8);

  if(mcs->len_mcss > 0){
    mcs->mcss = calloc(mcs->len_mcss, sizeof(mcs_params_t));
    assert(mcs->mcss != NULL && "memory exhausted");
  }

  for(uint32_t i = 0; i < mcs->len_mcss; ++i){
    mcs->mcss[i].id = abs(rand()%1024);
    mcs_params_t* s = &mcs->mcss[i];

    uint32_t type = abs(rand()% MCS_ALG_SM_V0_END);

    if(type == MCS_ALG_SM_V0_ORIGIN ){
      s->type =MCS_ALG_SM_V0_ORIGIN; 
    } else if (type == MCS_ALG_SM_V0_STATIC ){
      s->type = MCS_ALG_SM_V0_STATIC; 
      fill_static_mcs(&s->sta);
    } else {
      assert(0 != 0 && "Unknown type encountered");
    }
  }
}

static
void fill_mcs_conf(mcs_conf_t* conf)
{
  assert(conf != NULL);

  fill_ul_dl_mcs(&conf->ul);
  fill_ul_dl_mcs(&conf->dl);
}

static
void fill_ue_mcs_conf(ue_mcs_conf_t* conf)
{
  assert(conf != NULL);
  conf->len_ue_mcs = abs(rand()%10);
  if(conf->len_ue_mcs > 0){
    conf->ues = calloc(conf->len_ue_mcs, sizeof(ue_mcs_assoc_t));
    assert(conf->ues);
  }

  for(uint32_t i = 0; i < conf->len_ue_mcs; ++i){
    conf->ues[i].rnti = abs(rand()%1024);  
    conf->ues[i].dl_id = abs(rand()%16); 
    conf->ues[i].ul_id = abs(rand()%16); 
  }

}

void fill_mcs_ind_data(mcs_ind_data_t* ind_msg)
{
  assert(ind_msg != NULL);

  srand(time(0));

  fill_mcs_conf(&ind_msg->msg.mcs_conf);
  fill_ue_mcs_conf(&ind_msg->msg.ue_mcs_conf);

  ind_msg->msg.tstamp = time_now_us();
}


static
void fill_mac_ind_data(mac_ind_data_t* ind_msg)
{
    mac_ind_msg_t* ind = &ind_msg->msg; // data->mac_stats.msg;
    ind->ue_stats = calloc(5, sizeof(mac_ue_stats_impl_t));
    assert(ind->ue_stats != NULL);

    ind->tstamp = time_now_us();
    ind->len_ue_stats = 5;
    assert(ind->ue_stats != NULL);
    for(int i =0 ; i < 5; ++i){
      ind->ue_stats[i].dl_aggr_tbs = rand()%5000;
      ind->ue_stats[i].ul_aggr_tbs  = rand()%5000;
      ind->ue_stats[i].dl_aggr_bytes_sdus  = rand()%5000;
      ind->ue_stats[i].ul_aggr_bytes_sdus   = rand()%5000;
      ind->ue_stats[i].pusch_snr  = rand()%5000;
      ind->ue_stats[i].pucch_snr  = rand()%5000;
      ind->ue_stats[i].rnti  = rand()%5000;
      ind->ue_stats[i].dl_aggr_prb   = rand()%5000;
      ind->ue_stats[i].ul_aggr_prb  = rand()%5000;
      ind->ue_stats[i].dl_aggr_sdus  = rand()%5000;
      ind->ue_stats[i].ul_aggr_sdus  = rand()%5000;
      ind->ue_stats[i].dl_aggr_retx_prb   = rand()%5000;
      ind->ue_stats[i].wb_cqi = rand()%5000;
      ind->ue_stats[i].dl_mcs1 = rand()%5000;
      ind->ue_stats[i].ul_mcs1 = rand()%5000;
      ind->ue_stats[i].dl_mcs2 = rand()%5000;
      ind->ue_stats[i].ul_mcs2 = rand()%5000;
      ind->ue_stats[i].phr  = rand()%5000;
    }
}


void read_RAN(sm_ag_if_rd_t* data)
{
  assert(data->type == MAC_STATS_V0 || data->type == RLC_STATS_V0 ||  data->type == PDCP_STATS_V0 || data->type == SLICE_STATS_V0 || data->type == MCS_STATS_V0);

  srand(time(0));

  if(data->type == MAC_STATS_V0 ){
     fill_mac_ind_data(&data->mac_stats);
  } else if(data->type == RLC_STATS_V0) {

    data->rlc_stats.msg.tstamp = time_now_us();
    data->rlc_stats.msg.len = 5;
    data->rlc_stats.msg.rb = calloc(5, sizeof(rlc_radio_bearer_stats_t) );

    rlc_ind_msg_t* rlc = &data->rlc_stats.msg;
    for(int i = 0 ; i < 5; ++i){
      rlc->rb[i].txpdu_pkts= rand()%5000;
      rlc->rb[i].txpdu_bytes= rand()%5000;
      rlc->rb[i].txpdu_wt_ms= rand()%5000;
      rlc->rb[i].txpdu_dd_pkts= rand()%5000;
      rlc->rb[i].txpdu_dd_bytes= rand()%5000;
      rlc->rb[i].txpdu_retx_pkts= rand()%5000;
      rlc->rb[i].txpdu_retx_bytes= rand()%5000;
      rlc->rb[i].txpdu_segmented= rand()%5000;
      rlc->rb[i].txpdu_status_pkts= rand()%5000;
      rlc->rb[i].txpdu_status_bytes= rand()%5000;
      rlc->rb[i].txbuf_occ_bytes= rand()%5000;
      rlc->rb[i].txbuf_occ_pkts= rand()%5000;
      rlc->rb[i].rxpdu_pkts= rand()%5000;
      rlc->rb[i].rxpdu_bytes= rand()%5000;
      rlc->rb[i].rxpdu_dup_pkts= rand()%5000;
      rlc->rb[i].rxpdu_dup_bytes= rand()%5000;
      rlc->rb[i].rxpdu_dd_pkts= rand()%5000;
      rlc->rb[i].rxpdu_dd_bytes= rand()%5000;
      rlc->rb[i].rxpdu_ow_pkts= rand()%5000;
      rlc->rb[i].rxpdu_ow_bytes= rand()%5000;
      rlc->rb[i].rxpdu_status_pkts= rand()%5000;
      rlc->rb[i].rxpdu_status_bytes= rand()%5000;
      rlc->rb[i].rxbuf_occ_bytes= rand()%5000;
      rlc->rb[i].rxbuf_occ_pkts= rand()%5000;
      rlc->rb[i].txsdu_pkts= rand()%5000;
      rlc->rb[i].txsdu_bytes= rand()%5000;
      rlc->rb[i].rxsdu_pkts= rand()%5000;
      rlc->rb[i].rxsdu_dd_pkts= rand()%5000;
      rlc->rb[i].rxsdu_dd_bytes= rand()%5000;
      rlc->rb[i].rnti= rand()%5000;
      rlc->rb[i].mode= rand()%5000;
      rlc->rb[i].rbid= rand()%5000;
    }
  } else if (data->type == PDCP_STATS_V0 ){

    pdcp_ind_msg_t* ind_msg = &data->pdcp_stats.msg; 
    ind_msg->len = 5; //abs(rand()%10);
    ind_msg->tstamp =  time_now_us();

    if(ind_msg->len > 0){
      ind_msg->rb = calloc(ind_msg->len, sizeof(pdcp_radio_bearer_stats_t));
      assert(ind_msg->rb != NULL && "Memory exhausted!");
    }

    int const mod = 1024;
    for(uint32_t i = 0; i < ind_msg->len; ++i){
      pdcp_radio_bearer_stats_t* rb = &ind_msg->rb[i];

      rb->txpdu_bytes = abs(rand()%mod);    /* aggregated bytes of tx packets */
      rb->txpdu_pkts = rb->txpdu_bytes != 0 ? rb->txpdu_bytes - rand()%rb->txpdu_bytes : 0;     /* aggregated number of tx packets */
      rb->txpdu_sn= abs(rand()%mod);       /* current sequence number of last tx packet (or TX_NEXT) */

      rb->rxpdu_bytes = abs(rand()%mod);    /* aggregated bytes of rx packets */
      rb->rxpdu_pkts = rb->rxpdu_bytes != 0 ? rb->rxpdu_bytes - rand()%rb->rxpdu_bytes : 0;     /* aggregated number of rx packets */
      rb->rxpdu_sn= abs(rand()%mod);       /* current sequence number of last rx packet (or  RX_NEXT) */
      rb->rxpdu_oo_pkts= abs(rand()%mod);       /* aggregated number of out-of-order rx pkts  (or RX_REORD) */
      rb->rxpdu_oo_bytes= abs(rand()%mod); /* aggregated amount of out-of-order rx bytes */
      rb->rxpdu_dd_pkts= abs(rand()%mod);  /* aggregated number of duplicated discarded packets (or dropped packets because of other reasons such as integrity failure) (or RX_DELIV) */
      rb->rxpdu_dd_bytes= abs(rand()%mod); /* aggregated amount of discarded packets' bytes */
      rb->rxpdu_ro_count= abs(rand()%mod); /* this state variable indicates the COUNT value following the COUNT value associated with the PDCP Data PDU which triggered t-Reordering. (RX_REORD) */
      rb->txsdu_bytes = abs(rand()%mod);    /* number of bytes of SDUs delivered */
      rb->txsdu_pkts = rb->txsdu_bytes != 0 ? rb->txsdu_bytes - rand()% rb->txsdu_bytes : 0;     /* number of SDUs delivered */

      rb->rxsdu_bytes = abs(rand()%mod);    /* number of bytes of SDUs received */
      rb->rxsdu_pkts =  rb->rxsdu_bytes != 0 ? rb->rxsdu_bytes - rand()%rb->rxsdu_bytes : 0;     /* number of SDUs received */

      rb->rnti= abs(rand()%mod);
      rb->mode= abs(rand()%3);               /* 0: PDCP AM, 1: PDCP UM, 2: PDCP TM */
      rb->rbid= abs(rand()%11);

    }
  } else if(data->type == SLICE_STATS_V0 ){
    
    fill_slice_ind_data(&data->slice_stats);

  } else if (data->type == MCS_STATS_V0)
  {
    
  } else {
    assert("Invalid data type");
  }
}

sm_ag_if_ans_t write_RAN(sm_ag_if_wr_t const* data)
{
  assert(data != NULL);
  if(data->type == MAC_CTRL_REQ_V0){
    //printf("Control message called in the RAN \n");
    sm_ag_if_ans_t ans = {.type = MAC_AGENT_IF_CTRL_ANS_V0};
    ans.mac.ans = MAC_CTRL_OUT_OK;
    return ans;
  } else {
    assert(0 != 0 && "Not supported function ");
  }
  sm_ag_if_ans_t ans = {0};
  return ans;
}

int main()
{
  // Init the Agent
  const int mcc = 208; 
  const int mnc = 92; 
  const int mnc_digit_len = 2;
  const int nb_id = 42;
  sm_io_ag_t io = {.read = read_RAN, .write = write_RAN};
 
  init_agent_api( mcc, mnc, mnc_digit_len, nb_id, io );
  sleep(1);

  // Init the RIC
  init_near_ric_api();
  sleep(1);

  const uint16_t MAC_ran_func_id = 142;
  const char* cmd = "5_ms";
  report_service_near_ric_api(MAC_ran_func_id, cmd );
  sleep(2);

  const char* cmd2 = "Hello";
  control_service_near_ric_api(MAC_ran_func_id, cmd2 );  
  sleep(2);

//  load_sm_near_ric_api("../test/so/librlc_sm.so");

  const uint16_t RLC_ran_func_id = 143;
  report_service_near_ric_api(RLC_ran_func_id, cmd);
  sleep(2);

  const uint16_t PDCP_ran_func_id = 144;
  report_service_near_ric_api(PDCP_ran_func_id, cmd);
  sleep(2);

  const uint16_t SLICE_ran_func_id = 145;
  report_service_near_ric_api(SLICE_ran_func_id, cmd);
  sleep(2);

  rm_report_service_near_ric_api(MAC_ran_func_id, cmd);
  rm_report_service_near_ric_api(RLC_ran_func_id, cmd);
  rm_report_service_near_ric_api(PDCP_ran_func_id, cmd);
  rm_report_service_near_ric_api(SLICE_ran_func_id, cmd);

  sleep(1);

  // Stop the Agent
  stop_agent_api();

  // Stop the RIC
  stop_near_ric_api();

  printf("Test communicating E2-Agent and Near-RIC run SUCCESSFULLY\n");

}
