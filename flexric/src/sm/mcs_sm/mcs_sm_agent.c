#include "mcs_sm_agent.h"
#include "mcs_sm_id.h"
#include "enc/mcs_enc_generic.h"
#include "dec/mcs_dec_generic.h"
#include "../../util/alg_ds/alg/defer.h"


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{

  sm_agent_t base;

#ifdef ASN
  mcs_enc_asn_t enc;
#elif FLATBUFFERS 
  mcs_enc_fb_t enc;
#elif PLAIN
  mcs_enc_plain_t enc;
#else
  static_assert(false, "No encryption type selected");
#endif

} sm_mcs_agent_t;


// Function pointers provided by the RAN for the 
// 5 procedures, 
// subscription, indication, control, 
// E2 Setup and RIC Service Update. 
//
static
subscribe_timer_t  on_subscription_mcs_sm_ag(sm_agent_t* sm_agent, const sm_subs_data_t* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);

  sm_mcs_agent_t* sm = (sm_mcs_agent_t*)sm_agent;
 
  mcs_event_trigger_t ev = mcs_dec_event_trigger(&sm->enc, data->len_et, data->event_trigger);

  subscribe_timer_t timer = {.ms = ev.ms };
  return timer;
//  const sm_wr_if_t wr = {.type = SUBSCRIBE_TIMER, .sub_timer = timer };

//  sm->base.io.write(&wr);

//  printf("on_subscription called with event trigger = %u \n", ev.ms);
}

static
sm_ind_data_t on_indication_mcs_sm_ag(sm_agent_t* sm_agent)
{
//  printf("on_indication mcs called \n");

  assert(sm_agent != NULL);
  sm_mcs_agent_t* sm = (sm_mcs_agent_t*)sm_agent;

  sm_ind_data_t ret = {0};

  // Fill Indication Header
  mcs_ind_hdr_t hdr = {.dummy = 0 };
  byte_array_t ba_hdr = mcs_enc_ind_hdr(&sm->enc, &hdr );
  ret.ind_hdr = ba_hdr.buf;
  ret.len_hdr = ba_hdr.len;

  // Fill Indication Message 
  sm_ag_if_rd_t rd_if = {0};
  rd_if.type = MCS_STATS_V0;
  sm->base.io.read(&rd_if);

// Liberate the memory if previously allocated by the RAN. It sucks
  mcs_ind_data_t* ind = &rd_if.mcs_stats;
  defer({ free_mcs_ind_hdr(&ind->hdr) ;});
  defer({ free_mcs_ind_msg(&ind->msg) ;});
  defer({ free_mcs_call_proc_id(ind->proc_id);});

  byte_array_t ba = mcs_enc_ind_msg(&sm->enc, &rd_if.mcs_stats.msg);
  ret.ind_msg = ba.buf;
  ret.len_msg = ba.len;

  // Fill Call Process ID
  ret.call_process_id = NULL;
  ret.len_cpid = 0;

  return ret;
}

static
sm_ctrl_out_data_t on_control_mcs_sm_ag(sm_agent_t* sm_agent, sm_ctrl_req_data_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);
  sm_mcs_agent_t* sm = (sm_mcs_agent_t*) sm_agent;


  sm_ag_if_wr_t wr = {.type = MCS_CTRL_REQ_V0 };
  wr.mcs_req_ctrl.hdr = mcs_dec_ctrl_hdr(&sm->enc, data->len_hdr, data->ctrl_hdr);
  defer({ free_mcs_ctrl_hdr(&wr.mcs_req_ctrl.hdr ); });

  wr.mcs_req_ctrl.msg = mcs_dec_ctrl_msg(&sm->enc, data->len_msg, data->ctrl_msg);
  defer({ free_mcs_ctrl_msg(&wr.mcs_req_ctrl.msg); });
   

  sm_ag_if_ans_t ans = sm->base.io.write(&wr);
  printf("Agent received type is %d\n",ans.type);
  assert(ans.type == MCS_AGENT_IF_CTRL_ANS_V0);
  defer({free_mcs_ctrl_out(&ans.mcs); });

  byte_array_t ba = mcs_enc_ctrl_out(&sm->enc, &ans.mcs);

  sm_ctrl_out_data_t ret = {0};
  ret.len_out = ba.len;
  ret.ctrl_out = ba.buf;

  return ret;
}

static
sm_e2_setup_t on_e2_setup_mcs_sm_ag(sm_agent_t* sm_agent)
{
  assert(sm_agent != NULL);
//  printf("on_e2_setup called \n");

  sm_mcs_agent_t* sm = (sm_mcs_agent_t*)sm_agent;

  sm_e2_setup_t setup = {.len_rfd =0, .ran_fun_def = NULL  }; 

  setup.len_rfd = strlen(sm->base.ran_func_name);
  setup.ran_fun_def = calloc(1, strlen(sm->base.ran_func_name));
  assert(setup.ran_fun_def != NULL);
  memcpy(setup.ran_fun_def, sm->base.ran_func_name, strlen(sm->base.ran_func_name));

//  sm_e2_setup_t setup = {.len_rfd =0, .ran_fun_def = NULL  }; 
  return setup;
}

static
void on_ric_service_update_mcs_sm_ag(sm_agent_t* sm_agent, sm_ric_service_update_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);


  printf("on_ric_service_update called \n");
}

static
void free_mcs_sm_ag(sm_agent_t* sm_agent)
{
  assert(sm_agent != NULL);
  sm_mcs_agent_t* sm = (sm_mcs_agent_t*)sm_agent;
  free(sm);
}


sm_agent_t* make_mcs_sm_agent(sm_io_ag_t io)
{
  sm_mcs_agent_t* sm = calloc(1, sizeof(sm_mcs_agent_t));
  assert(sm != NULL && "Memory exhausted!!!");

  *(uint16_t*)(&sm->base.ran_func_id) = SM_MCS_ID; 

  sm->base.io = io;
  sm->base.free_sm = free_mcs_sm_ag;

  sm->base.proc.on_subscription = on_subscription_mcs_sm_ag;
  sm->base.proc.on_indication = on_indication_mcs_sm_ag;
  sm->base.proc.on_control = on_control_mcs_sm_ag;
  sm->base.proc.on_ric_service_update = on_ric_service_update_mcs_sm_ag;
  sm->base.proc.on_e2_setup = on_e2_setup_mcs_sm_ag;
  sm->base.handle = NULL;

  *(uint16_t*)(&sm->base.ran_func_id) = SM_MCS_ID; 
  assert(strlen(SM_MCS_STR) < sizeof( sm->base.ran_func_name) );
  memcpy(sm->base.ran_func_name, SM_MCS_STR, strlen(SM_MCS_STR)); 

  return &sm->base;
}

uint16_t id_mcs_sm_agent(sm_agent_t const* sm_agent )
{
  assert(sm_agent != NULL);
  sm_mcs_agent_t* sm = (sm_mcs_agent_t*)sm_agent;
  return sm->base.ran_func_id;
}

