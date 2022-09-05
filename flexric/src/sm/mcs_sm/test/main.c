#include "../../mcs_sm/mcs_sm_agent.h"
#include "../../mcs_sm/mcs_sm_ric.h"
#include "../../../util/alg_ds/alg/defer.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>


// For testing purposes
static
mcs_ind_data_t cp;

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


void free_ag_mcs(void)
{


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

  cp.msg = cp_mcs_ind_msg(&ind_msg->msg);
}

static
mcs_ctrl_req_data_t cp_ctrl;


static
void ctrl_mcs(mcs_ctrl_req_data_t const* ctrl)
{
  assert(ctrl != NULL);

  bool const ans_hdr = eq_mcs_ctrl_hdr(&cp_ctrl.hdr, &ctrl->hdr);
  assert(ans_hdr == true);

  bool const ans_msg = eq_mcs_ctrl_msg(&cp_ctrl.msg, &ctrl->msg);
  assert(ans_msg == true);
}

//
// Functions 


/////
// AGENT
////

static
void read_RAN(sm_ag_if_rd_t* read)
{
  assert(read != NULL);
  assert(read->type == MCS_STATS_V0);

  fill_mcs_ind_data(&read->mcs_stats);
}


static 
sm_ag_if_ans_t write_RAN(sm_ag_if_wr_t const* data)
{
  assert(data != NULL);
  assert(data->type == MCS_CTRL_REQ_V0);

  if(data->type ==  MCS_CTRL_REQ_V0){
    printf("mcs Control called \n");
    ctrl_mcs(&data->mcs_req_ctrl);
  } else {
    assert(0!=0 && "Unknown data type");
  }

  sm_ag_if_ans_t ans = {.type = MCS_AGENT_IF_CTRL_ANS_V0}; 
 
  const char* str = "THIS IS ANS STRING";
  ans.mcs.len_diag = strlen(str);
  ans.mcs.diagnostic = malloc(strlen(str));
  assert(ans.mcs.diagnostic != NULL && "Memory exhausted");

  return ans;
}


static
void fill_mcs_del(del_mcs_conf_t* conf)
{
  assert(conf != NULL);

  uint32_t const len_dl = rand()%5;
  conf->len_dl = len_dl;
  if(conf->len_dl > 0)
    conf->dl = calloc(len_dl, sizeof(uint32_t));

  uint32_t const len_ul = rand()%5;
  conf->len_ul = len_ul;
  if(conf->len_ul > 0)
    conf->ul = calloc(len_ul, sizeof(uint32_t));
}

/////////////////////////////
// Check Functions
// //////////////////////////

static
void check_eq_ran_function(sm_agent_t const* ag, sm_ric_t const* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);
  assert(ag->ran_func_id == ric->ran_func_id);
}

// RIC -> E2
static
void check_subscription(sm_agent_t* ag, sm_ric_t* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);
 
  sm_subs_data_t data = ric->proc.on_subscription(ric, "2_ms");
  ag->proc.on_subscription(ag, &data); 

  free_sm_subs_data(&data);
}

// E2 -> RIC
static
void check_indication(sm_agent_t* ag, sm_ric_t* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);

  sm_ind_data_t sm_data = ag->proc.on_indication(ag);
  defer({ free_sm_ind_data(&sm_data); }); 

  sm_ag_if_rd_t msg = ric->proc.on_indication(ric, &sm_data);

  mcs_ind_data_t* data = &msg.mcs_stats;
  assert(msg.type == MCS_STATS_V0);

  assert(eq_mcs_ind_msg(&cp.msg, &data->msg) == true);

  free_mcs_ind_hdr(&data->hdr); 
  free_mcs_ind_msg(&data->msg); 

  free_mcs_ind_hdr(&cp.hdr);
  free_mcs_ind_msg(&cp.msg);
}


static
mcs_ctrl_req_data_t generate_mcs_ctrl()
{
   uint32_t type = rand()%MCS_CTRL_SM_V0_END;
   mcs_ctrl_req_data_t ret = {0}; 
   ret.msg.type = type;

   if(type == MCS_CTRL_SM_V0_ADD){
     fill_mcs_conf(&ret.msg.add_mod_mcs);
   } else if (type == MCS_CTRL_SM_V0_DEL){
     fill_mcs_del(&ret.msg.del_mcs);
   } else if (type == MCS_CTRL_SM_V0_UE_MCS_ASSOC){
     fill_ue_mcs_conf(&ret.msg.ue_mcs); 
   } else {
      assert(0!=0 && "Unknown type");
   }

   return ret;
}

static
void check_ctrl(sm_agent_t* ag, sm_ric_t* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);

  sm_ag_if_wr_t ctrl = {.type = MCS_CTRL_REQ_V0 };
  ctrl.mcs_req_ctrl = generate_mcs_ctrl();

  cp_ctrl.hdr = cp_mcs_ctrl_hdr(&ctrl.mcs_req_ctrl.hdr);
  cp_ctrl.msg = cp_mcs_ctrl_msg(&ctrl.mcs_req_ctrl.msg);

  sm_ctrl_req_data_t ctrl_req = ric->proc.on_control_req(ric, &ctrl);

  sm_ctrl_out_data_t out_data = ag->proc.on_control(ag, &ctrl_req);

  sm_ag_if_ans_t ans = ric->proc.on_control_out(ric, &out_data);
  assert(ans.type == MCS_AGENT_IF_CTRL_ANS_V0 );

  if(ctrl_req.len_hdr > 0)
    free(ctrl_req.ctrl_hdr);

  if(ctrl_req.len_msg > 0)
    free(ctrl_req.ctrl_msg);

  if(out_data.len_out > 0)
    free(out_data.ctrl_out);

  free_mcs_ctrl_out(&ans.mcs);

  free_mcs_ctrl_hdr(&ctrl.mcs_req_ctrl.hdr); 
  free_mcs_ctrl_msg(&ctrl.mcs_req_ctrl.msg); 

  free_mcs_ctrl_hdr(&cp_ctrl.hdr);
  free_mcs_ctrl_msg(&cp_ctrl.msg);
}

int main()
{
  sm_io_ag_t io_ag = {.read = read_RAN, .write = write_RAN};  
  sm_agent_t* sm_ag = make_mcs_sm_agent(io_ag);

  sm_ric_t* sm_ric = make_mcs_sm_ric();

  check_eq_ran_function(sm_ag, sm_ric);
  check_subscription(sm_ag, sm_ric);
  check_indication(sm_ag, sm_ric);
  check_ctrl(sm_ag, sm_ric);

  sm_ag->free_sm(sm_ag);
  sm_ric->free_sm(sm_ric);

  free_ag_mcs();

  printf("Success\n");
  return EXIT_SUCCESS;
}

