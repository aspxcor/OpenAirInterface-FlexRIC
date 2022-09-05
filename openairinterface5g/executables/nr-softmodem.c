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


#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>


#include "T.h"

#undef MALLOC //there are two conflicting definitions, so we better make sure we don't use it at all
#include <common/utils/assertions.h>

#include "msc.h"

#include "PHY/types.h"
#include "common/ran_context.h"

#include "PHY/defs_gNB.h"
#include "PHY/defs_common.h"
#include "common/config/config_userapi.h"
#include "common/utils/load_module_shlib.h"
#undef MALLOC //there are two conflicting definitions, so we better make sure we don't use it at all
//#undef FRAME_LENGTH_COMPLEX_SAMPLES //there are two conflicting definitions, so we better make sure we don't use it at all

#include "../../ARCH/COMMON/common_lib.h"
#include "../../ARCH/ETHERNET/USERSPACE/LIB/if_defs.h"

//#undef FRAME_LENGTH_COMPLEX_SAMPLES //there are two conflicting definitions, so we better make sure we don't use it at all

#include "PHY/phy_vars.h"
#include "SCHED/sched_common_vars.h"
#include "LAYER2/MAC/mac_vars.h"
#include "RRC/LTE/rrc_vars.h"
#include "PHY_INTERFACE/phy_interface_vars.h"
#include "gnb_config.h"
#include "SIMULATION/TOOLS/sim.h"
#include <targets/RT/USER/lte-softmodem.h>

#ifdef SMBV
#include "PHY/TOOLS/smbv.h"
unsigned short config_frames[4] = {2,9,11,13};
#endif

#include "common/utils/LOG/log.h"
#include "common/utils/LOG/vcd_signal_dumper.h"
#include "UTIL/OPT/opt.h"

//#include "PHY/TOOLS/time_meas.h"

#include "intertask_interface.h"

#include "PHY/INIT/phy_init.h"

#include "system.h"
#include <openair2/GNB_APP/gnb_app.h>
#include "PHY/TOOLS/phy_scope_interface.h"
#include "PHY/TOOLS/nr_phy_scope.h"
#include "stats.h"
#include "nr-softmodem.h"
#include "executables/softmodem-common.h"
#include "executables/thread-common.h"
#include "NB_IoT_interface.h"
#include "x2ap_eNB.h"
#include "ngap_gNB.h"
#include "gnb_paramdef.h"
#include <openair3/ocp-gtpu/gtp_itf.h>
#include "nfapi/oai_integration/vendor_ext.h"


//////////////////////////////////
//// E2 Agent headers
//////////////////////////////////

#include "agent_if/read/sm_ag_if_rd.h"
#include "agent_if/sm_io.h"
#include "agent_if/e2_agent_api.h"
#include "openair2/LAYER2/nr_rlc/nr_rlc_entity.h"
#include "openair2/LAYER2/nr_pdcp/nr_pdcp_entity.h"
#include "openair2/LAYER2/nr_rlc/nr_rlc_oai_api.h"
#include "openair2/LAYER2/nr_pdcp/nr_pdcp.h"
#include <time.h>

//////////////////////////////////
//////////////////////////////////
//////////////////////////////////


pthread_cond_t nfapi_sync_cond;
pthread_mutex_t nfapi_sync_mutex;
int nfapi_sync_var=-1; //!< protected by mutex \ref nfapi_sync_mutex

extern uint8_t nfapi_mode; // Default to monolithic mode
THREAD_STRUCT thread_struct;
pthread_cond_t sync_cond;
pthread_mutex_t sync_mutex;
int sync_var=-1; //!< protected by mutex \ref sync_mutex.
int config_sync_var=-1;

volatile int             start_gNB = 0;
volatile int             oai_exit = 0;

static int wait_for_sync = 0;

unsigned int mmapped_dma=0;
int single_thread_flag=1;

int8_t threequarter_fs=0;

uint64_t downlink_frequency[MAX_NUM_CCs][4];
int32_t uplink_frequency_offset[MAX_NUM_CCs][4];

//Temp fix for inexistent NR upper layer
unsigned char NB_gNB_INST = 1;


int UE_scan = 1;
int UE_scan_carrier = 0;
runmode_t mode = normal_txrx;
static double snr_dB=20;

FILE *input_fd=NULL;


#if MAX_NUM_CCs == 1
rx_gain_t rx_gain_mode[MAX_NUM_CCs][4] = {{max_gain,max_gain,max_gain,max_gain}};
double tx_gain[MAX_NUM_CCs][4] = {{20,0,0,0}};
double rx_gain[MAX_NUM_CCs][4] = {{110,0,0,0}};
#else
rx_gain_t rx_gain_mode[MAX_NUM_CCs][4] = {{max_gain,max_gain,max_gain,max_gain},{max_gain,max_gain,max_gain,max_gain}};
double tx_gain[MAX_NUM_CCs][4] = {{20,0,0,0},{20,0,0,0}};
double rx_gain[MAX_NUM_CCs][4] = {{110,0,0,0},{20,0,0,0}};
#endif

double rx_gain_off = 0.0;

static int tx_max_power[MAX_NUM_CCs]; /* =  {0,0}*/;


int chain_offset=0;


uint8_t dci_Format = 0;
uint8_t agregation_Level =0xFF;

uint8_t nb_antenna_tx = 1;
uint8_t nb_antenna_rx = 1;

char ref[128] = "internal";
char channels[128] = "0";

int rx_input_level_dBm;

int otg_enabled;

//int number_of_cards = 1;


//static NR_DL_FRAME_PARMS      *frame_parms[MAX_NUM_CCs];
//static nfapi_nr_config_request_t *config[MAX_NUM_CCs];
uint32_t timing_advance = 0;
uint64_t num_missed_slots=0; // counter for the number of missed slots

#include <executables/split_headers.h>
#include <SIMULATION/ETH_TRANSPORT/proto.h>

int split73=0;
void sendFs6Ul(PHY_VARS_eNB *eNB, int UE_id, int harq_pid, int segmentID, int16_t *data, int dataLen, int r_offset) {
  AssertFatal(false, "Must not be called in this context\n");
}
void sendFs6Ulharq(enum pckType type, int UEid, PHY_VARS_eNB *eNB, LTE_eNB_UCI *uci, int frame, int subframe, uint8_t *harq_ack, uint8_t tdd_mapping_mode, uint16_t tdd_multiplexing_mask,
                   uint16_t rnti, int32_t stat) {
  AssertFatal(false, "Must not be called in this context\n");
}


extern void reset_opp_meas(void);
extern void print_opp_meas(void);

extern void *udp_eNB_task(void *args_p);

int transmission_mode=1;
int emulate_rf = 0;
int numerology = 0;


static char *parallel_config = NULL;
static char *worker_config = NULL;

/* struct for ethernet specific parameters given in eNB conf file */
eth_params_t *eth_params;

openair0_config_t openair0_cfg[MAX_CARDS];

double cpuf;

extern char uecap_xer[1024];
char uecap_xer_in=0;

/* see file openair2/LAYER2/MAC/main.c for why abstraction_flag is needed
 * this is very hackish - find a proper solution
 */
uint8_t abstraction_flag=0;

/* forward declarations */
void set_default_frame_parms(nfapi_nr_config_request_scf_t *config[MAX_NUM_CCs], NR_DL_FRAME_PARMS *frame_parms[MAX_NUM_CCs]);

/*---------------------BMC: timespec helpers -----------------------------*/

struct timespec min_diff_time = { .tv_sec = 0, .tv_nsec = 0 };
struct timespec max_diff_time = { .tv_sec = 0, .tv_nsec = 0 };

struct timespec clock_difftime(struct timespec start, struct timespec end) {
  struct timespec temp;

  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }

  return temp;
}

void print_difftimes(void) {
#ifdef DEBUG
  printf("difftimes min = %lu ns ; max = %lu ns\n", min_diff_time.tv_nsec, max_diff_time.tv_nsec);
#else
  LOG_I(HW,"difftimes min = %lu ns ; max = %lu ns\n", min_diff_time.tv_nsec, max_diff_time.tv_nsec);
#endif
}

void update_difftimes(struct timespec start, struct timespec end) {
  struct timespec diff_time = { .tv_sec = 0, .tv_nsec = 0 };
  int             changed = 0;
  diff_time = clock_difftime(start, end);

  if ((min_diff_time.tv_nsec == 0) || (diff_time.tv_nsec < min_diff_time.tv_nsec)) {
    min_diff_time.tv_nsec = diff_time.tv_nsec;
    changed = 1;
  }

  if ((max_diff_time.tv_nsec == 0) || (diff_time.tv_nsec > max_diff_time.tv_nsec)) {
    max_diff_time.tv_nsec = diff_time.tv_nsec;
    changed = 1;
  }

#if 1

  if (changed) print_difftimes();

#endif
}

/*------------------------------------------------------------------------*/

unsigned int build_rflocal(int txi, int txq, int rxi, int rxq) {
  return (txi + (txq<<6) + (rxi<<12) + (rxq<<18));
}
unsigned int build_rfdc(int dcoff_i_rxfe, int dcoff_q_rxfe) {
  return (dcoff_i_rxfe + (dcoff_q_rxfe<<8));
}


#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KBLU  "\x1B[34m"
#define RESET "\033[0m"


void exit_function(const char *file, const char *function, const int line, const char *s) {
  int ru_id;

  if (s != NULL) {
    printf("%s:%d %s() Exiting OAI softmodem: %s\n",file,line, function, s);
  }

  oai_exit = 1;

  if (RC.ru == NULL)
    exit(-1); // likely init not completed, prevent crash or hang, exit now...

  for (ru_id=0; ru_id<RC.nb_RU; ru_id++) {
    if (RC.ru[ru_id] && RC.ru[ru_id]->rfdevice.trx_end_func) {
      RC.ru[ru_id]->rfdevice.trx_end_func(&RC.ru[ru_id]->rfdevice);
      RC.ru[ru_id]->rfdevice.trx_end_func = NULL;
    }

    if (RC.ru[ru_id] && RC.ru[ru_id]->ifdevice.trx_end_func) {
      RC.ru[ru_id]->ifdevice.trx_end_func(&RC.ru[ru_id]->ifdevice);
      RC.ru[ru_id]->ifdevice.trx_end_func = NULL;
    }
  }

  sleep(1); //allow lte-softmodem threads to exit first
  exit(1);
}



int create_gNB_tasks(uint32_t gnb_nb) {
  LOG_D(GNB_APP, "%s(gnb_nb:%d)\n", __FUNCTION__, gnb_nb);
  itti_wait_ready(1);

  if (gnb_nb > 0) {
    /* Last task to create, others task must be ready before its start */
    /*if (itti_create_task (TASK_GNB_APP, gNB_app_task, NULL) < 0) {
      LOG_E(GNB_APP, "Create task for gNB APP failed\n");
      return -1;
    }*/
    if(itti_create_task(TASK_SCTP, sctp_eNB_task, NULL) < 0) {
      LOG_E(SCTP, "Create task for SCTP failed\n");
      return -1;
    }

    if (is_x2ap_enabled()) {
      if(itti_create_task(TASK_X2AP, x2ap_task, NULL) < 0) {
        LOG_E(X2AP, "Create task for X2AP failed\n");
      }
    } else {
      LOG_I(X2AP, "X2AP is disabled.\n");
    }
  }

  paramdef_t NETParams[]  =  GNBNETPARAMS_DESC;
  char aprefix[MAX_OPTNAME_SIZE*2 + 8];
  sprintf(aprefix,"%s.[%i].%s",GNB_CONFIG_STRING_GNB_LIST,0,GNB_CONFIG_STRING_NETWORK_INTERFACES_CONFIG);
  config_get( NETParams,sizeof(NETParams)/sizeof(paramdef_t),aprefix);

  for(int i = GNB_INTERFACE_NAME_FOR_NG_AMF_IDX; i <= GNB_IPV4_ADDRESS_FOR_NG_AMF_IDX; i++) {
    if( NETParams[i].strptr == NULL) {
      LOG_E(NGAP, "No configuration in the file.\n");
      NGAP_CONF_MODE = 0;
    } else {
      LOG_D(NGAP, "Configuration in the file: %s.\n",*NETParams[i].strptr);
    }
  }

  if (AMF_MODE_ENABLED) {
    if (gnb_nb > 0) {
      /*
      if (itti_create_task (TASK_SCTP, sctp_eNB_task, NULL) < 0) {
        LOG_E(SCTP, "Create task for SCTP failed\n");
        return -1;
      }
      */
      if(NGAP_CONF_MODE) {
        if (itti_create_task (TASK_NGAP, ngap_gNB_task, NULL) < 0) {
          LOG_E(NGAP, "Create task for NGAP failed\n");
          return -1;
        }
      } else {
        LOG_E(NGAP, "Ngap task not created\n");
      }

      if(!emulate_rf) {
        if (itti_create_task (TASK_UDP, udp_eNB_task, NULL) < 0) {
          LOG_E(UDP_, "Create task for UDP failed\n");
          return -1;
        }
      }

      /*if (itti_create_task (TASK_GTPV1_U, &nr_gtpv1u_gNB_task, NULL) < 0) {
        LOG_E(GTPU, "Create task for GTPV1U failed\n");
        return -1;
      }*/
    }
  }

  if (gnb_nb > 0) {
    if (itti_create_task (TASK_GNB_APP, gNB_app_task, NULL) < 0) {
      LOG_E(GNB_APP, "Create task for gNB APP failed\n");
      return -1;
    }

    LOG_I(NR_RRC,"Creating NR RRC gNB Task\n");

    if (itti_create_task (TASK_RRC_GNB, rrc_gnb_task, NULL) < 0) {
      LOG_E(NR_RRC, "Create task for NR RRC gNB failed\n");
      return -1;
    }

    //Use check on x2ap to consider the NSA scenario and check on AMF_MODE_ENABLED for the SA scenario
    if(is_x2ap_enabled() || AMF_MODE_ENABLED) {
      if (itti_create_task (TASK_GTPV1_U, &nr_gtpv1u_gNB_task, NULL) < 0) {
        LOG_E(GTPU, "Create task for GTPV1U failed\n");
        return -1;
      }
    }
  }

  return 0;
}


static void get_options(void) {
  paramdef_t cmdline_params[] = CMDLINE_PARAMS_DESC_GNB ;
  CONFIG_SETRTFLAG(CONFIG_NOEXITONHELP);
  get_common_options(SOFTMODEM_GNB_BIT );
  config_process_cmdline( cmdline_params,sizeof(cmdline_params)/sizeof(paramdef_t),NULL);
  CONFIG_CLEARRTFLAG(CONFIG_NOEXITONHELP);

  if ( !(CONFIG_ISFLAGSET(CONFIG_ABORT)) ) {
    memset((void *)&RC,0,sizeof(RC));
    /* Read RC configuration file */
    NRRCConfig();
    NB_gNB_INST = RC.nb_nr_inst;
    NB_RU   = RC.nb_RU;
    printf("Configuration: nb_rrc_inst %d, nb_nr_L1_inst %d, nb_ru %hhu\n",NB_gNB_INST,RC.nb_nr_L1_inst,NB_RU);
  }

  if(parallel_config != NULL) set_parallel_conf(parallel_config);

  if(worker_config != NULL) set_worker_conf(worker_config);
}


void set_default_frame_parms(nfapi_nr_config_request_scf_t *config[MAX_NUM_CCs],
                             NR_DL_FRAME_PARMS *frame_parms[MAX_NUM_CCs]) {
  for (int CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {
    frame_parms[CC_id] = (NR_DL_FRAME_PARMS *) malloc(sizeof(NR_DL_FRAME_PARMS));
    config[CC_id] = (nfapi_nr_config_request_scf_t *) malloc(sizeof(nfapi_nr_config_request_scf_t));
    config[CC_id]->ssb_config.scs_common.value = 1;
    config[CC_id]->cell_config.frame_duplex_type.value = 1; //FDD
    //config[CC_id]->subframe_config.dl_cyclic_prefix_type.value = 0; //NORMAL
    config[CC_id]->carrier_config.dl_grid_size[1].value = 106;
    config[CC_id]->carrier_config.ul_grid_size[1].value = 106;
    config[CC_id]->cell_config.phy_cell_id.value = 0;
    ///dl frequency to be filled in
    /*  //Set some default values that may be overwritten while reading options
        frame_parms[CC_id]->frame_type          = FDD;
        frame_parms[CC_id]->tdd_config          = 3;
        frame_parms[CC_id]->tdd_config_S        = 0;
        frame_parms[CC_id]->N_RB_DL             = 100;
        frame_parms[CC_id]->N_RB_UL             = 100;
        frame_parms[CC_id]->Ncp                 = NORMAL;
        frame_parms[CC_id]->Ncp_UL              = NORMAL;
        frame_parms[CC_id]->Nid_cell            = 0;
        frame_parms[CC_id]->num_MBSFN_config    = 0;
        frame_parms[CC_id]->nb_antenna_ports_gNB  = 1;
        frame_parms[CC_id]->nb_antennas_tx      = 1;
        frame_parms[CC_id]->nb_antennas_rx      = 1;

        frame_parms[CC_id]->nushift             = 0;

        frame_parms[CC_id]->phich_config_common.phich_resource = oneSixth;
        frame_parms[CC_id]->phich_config_common.phich_duration = normal;
        // UL RS Config
        frame_parms[CC_id]->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift = 0;//n_DMRS1 set to 0
        frame_parms[CC_id]->pusch_config_common.ul_ReferenceSignalsPUSCH.groupHoppingEnabled = 0;
        frame_parms[CC_id]->pusch_config_common.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled = 0;
        frame_parms[CC_id]->pusch_config_common.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH = 0;

        frame_parms[CC_id]->prach_config_common.rootSequenceIndex=22;
        frame_parms[CC_id]->prach_config_common.prach_ConfigInfo.zeroCorrelationZoneConfig=1;
        frame_parms[CC_id]->prach_config_common.prach_ConfigInfo.prach_ConfigIndex=0;
        frame_parms[CC_id]->prach_config_common.prach_ConfigInfo.highSpeedFlag=0;
        frame_parms[CC_id]->prach_config_common.prach_ConfigInfo.prach_FreqOffset=0;

    //    downlink_frequency[CC_id][0] = 2680000000; // Use float to avoid issue with frequency over 2^31.
    //    downlink_frequency[CC_id][1] = downlink_frequency[CC_id][0];
    //    downlink_frequency[CC_id][2] = downlink_frequency[CC_id][0];
    //    downlink_frequency[CC_id][3] = downlink_frequency[CC_id][0];
        //printf("Downlink for CC_id %d frequency set to %u\n", CC_id, downlink_frequency[CC_id][0]);
        frame_parms[CC_id]->dl_CarrierFreq=downlink_frequency[CC_id][0];
    */
  }
}

void wait_RUs(void) {
  LOG_I(PHY,"Waiting for RUs to be configured ... RC.ru_mask:%02lx\n", RC.ru_mask);
  // wait for all RUs to be configured over fronthaul
  pthread_mutex_lock(&RC.ru_mutex);

  while (RC.ru_mask>0) {
    pthread_cond_wait(&RC.ru_cond,&RC.ru_mutex);
    printf("RC.ru_mask:%02lx\n", RC.ru_mask);
  }

  pthread_mutex_unlock(&RC.ru_mutex);
  LOG_I(PHY,"RUs configured\n");
}

void wait_gNBs(void) {
  int i;
  int waiting=1;

  while (waiting==1) {
    printf("Waiting for gNB L1 instances to all get configured ... sleeping 50ms (nb_nr_sL1_inst %d)\n",RC.nb_nr_L1_inst);
    usleep(50*1000);
    waiting=0;

    for (i=0; i<RC.nb_nr_L1_inst; i++) {
      if (RC.gNB[i]->configured==0) {
        waiting=1;
        break;
      }
    }
  }

  printf("gNB L1 are configured\n");
}

/*
 * helper function to terminate a certain ITTI task
 */
void terminate_task(task_id_t task_id, module_id_t mod_id) {
  LOG_I(GNB_APP, "sending TERMINATE_MESSAGE to task %s (%d)\n", itti_get_task_name(task_id), task_id);
  MessageDef *msg;
  msg = itti_alloc_new_message (ENB_APP, 0, TERMINATE_MESSAGE);
  itti_send_msg_to_task (task_id, ENB_MODULE_ID_TO_INSTANCE(mod_id), msg);
}

//extern void  free_transport(PHY_VARS_gNB *);
extern void  nr_phy_free_RU(RU_t *);

int stop_L1L2(module_id_t gnb_id) {
  LOG_W(GNB_APP, "stopping nr-softmodem\n");
  oai_exit = 1;

  if (!RC.ru) {
    LOG_F(GNB_APP, "no RU configured\n");
    return -1;
  }

  /* stop trx devices, multiple carrier currently not supported by RU */
  if (RC.ru[gnb_id]) {
    if (RC.ru[gnb_id]->rfdevice.trx_stop_func) {
      RC.ru[gnb_id]->rfdevice.trx_stop_func(&RC.ru[gnb_id]->rfdevice);
      LOG_I(GNB_APP, "turned off RU rfdevice\n");
    } else {
      LOG_W(GNB_APP, "can not turn off rfdevice due to missing trx_stop_func callback, proceeding anyway!\n");
    }

    if (RC.ru[gnb_id]->ifdevice.trx_stop_func) {
      RC.ru[gnb_id]->ifdevice.trx_stop_func(&RC.ru[gnb_id]->ifdevice);
      LOG_I(GNB_APP, "turned off RU ifdevice\n");
    } else {
      LOG_W(GNB_APP, "can not turn off ifdevice due to missing trx_stop_func callback, proceeding anyway!\n");
    }
  } else {
    LOG_W(GNB_APP, "no RU found for index %d\n", gnb_id);
    return -1;
  }

  /* these tasks need to pick up new configuration */
  terminate_task(TASK_RRC_ENB, gnb_id);
  LOG_I(GNB_APP, "calling kill_gNB_proc() for instance %d\n", gnb_id);
  kill_gNB_proc(gnb_id);
  LOG_I(GNB_APP, "calling kill_NR_RU_proc() for instance %d\n", gnb_id);
  kill_NR_RU_proc(gnb_id);
  oai_exit = 0;
  //free_transport(RC.gNB[gnb_id]);
  phy_free_nr_gNB(RC.gNB[gnb_id]);
  nr_phy_free_RU(RC.ru[gnb_id]);
  free_lte_top();
  return 0;
}

/*
 * Restart the nr-softmodem after it has been soft-stopped with stop_L1L2()
 */
int restart_L1L2(module_id_t gnb_id) {
  RU_t *ru = RC.ru[gnb_id];
  MessageDef *msg_p = NULL;
  LOG_W(GNB_APP, "restarting nr-softmodem\n");
  /* block threads */
  sync_var = -1;
  RC.gNB[gnb_id]->configured = 0;
  RC.ru_mask |= (1 << ru->idx);
  set_function_spec_param(RC.ru[gnb_id]);
  LOG_I(GNB_APP, "attempting to create ITTI tasks\n");
  // No more rrc thread, as many race conditions are hidden behind
  rrc_enb_init();
  itti_mark_task_ready(TASK_RRC_ENB);
  /* pass a reconfiguration request which will configure everything down to
   * RC.eNB[i][j]->frame_parms, too */
  msg_p = itti_alloc_new_message(TASK_ENB_APP, 0, RRC_CONFIGURATION_REQ);
  RRC_CONFIGURATION_REQ(msg_p) = RC.rrc[gnb_id]->configuration;
  itti_send_msg_to_task(TASK_RRC_ENB, ENB_MODULE_ID_TO_INSTANCE(gnb_id), msg_p);
  /* TODO XForms might need to be restarted, but it is currently (09/02/18)
   * broken, so we cannot test it */
  wait_gNBs();
  init_RU_proc(ru);
  ru->rf_map.card = 0;
  ru->rf_map.chain = 0; /* CC_id + chain_offset;*/
  wait_RUs();
  init_eNB_afterRU();
  printf("Sending sync to all threads\n");
  pthread_mutex_lock(&sync_mutex);
  sync_var=0;
  pthread_cond_broadcast(&sync_cond);
  pthread_mutex_unlock(&sync_mutex);
  return 0;
}

static  void wait_nfapi_init(char *thread_name) {
  printf( "waiting for NFAPI PNF connection and population of global structure (%s)\n",thread_name);
  pthread_mutex_lock( &nfapi_sync_mutex );

  while (nfapi_sync_var<0)
    pthread_cond_wait( &nfapi_sync_cond, &nfapi_sync_mutex );

  pthread_mutex_unlock(&nfapi_sync_mutex);
  printf( "NFAPI: got sync (%s)\n", thread_name);
}

void init_pdcp(void) {
  if (!NODE_IS_DU(RC.nrrrc[0]->node_type)) {
    // pdcp_layer_init();
    uint32_t pdcp_initmask = (IS_SOFTMODEM_NOS1) ?
                             (PDCP_USE_NETLINK_BIT | LINK_ENB_PDCP_TO_IP_DRIVER_BIT) : LINK_ENB_PDCP_TO_GTPV1U_BIT;

    if (IS_SOFTMODEM_NOS1) {
      printf("IS_SOFTMODEM_NOS1 option enabled \n");
      pdcp_initmask = pdcp_initmask | ENB_NAS_USE_TUN_BIT | SOFTMODEM_NOKRNMOD_BIT;
    }

    pdcp_module_init(pdcp_initmask);

    if (NODE_IS_CU(RC.nrrrc[0]->node_type)) {
      LOG_I(PDCP, "node is CU, pdcp send rlc_data_req by proto_agent \n");
      pdcp_set_rlc_data_req_func((send_rlc_data_req_func_t)proto_agent_send_rlc_data_req);
    } else {
      LOG_I(PDCP, "node is gNB \n");
      pdcp_set_rlc_data_req_func((send_rlc_data_req_func_t) rlc_data_req);
      pdcp_set_pdcp_data_ind_func((pdcp_data_ind_func_t) pdcp_data_ind);
    }
  } else {
    LOG_I(PDCP, "node is DU, rlc send pdcp_data_ind by proto_agent \n");
    pdcp_set_pdcp_data_ind_func((pdcp_data_ind_func_t) proto_agent_send_pdcp_data_ind);
  }
}


static
const int mod_id = 0;

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
void read_mac_sm(mac_ind_msg_t* data)
{
  assert(data != NULL);

  data->tstamp = time_now_us();

//  assert(0!=0 && "Read MAC called");

  const NR_UE_info_t* UE_info = &RC.nrmac[mod_id]->UE_info;
  const size_t num_ues = UE_info->num_UEs;

  data->len_ue_stats = num_ues;
  if(data->len_ue_stats > 0){
    data->ue_stats = calloc(data->len_ue_stats, sizeof(mac_ue_stats_impl_t));
    assert( data->ue_stats != NULL && "Memory exhausted" );
  }

  const NR_list_t* ue_list = &UE_info->list;
  size_t i = 0;
  for (int ue_id = ue_list->head; ue_id >= 0; ue_id = ue_list->next[ue_id]) {
    const NR_UE_sched_ctrl_t* sched_ctrl = &UE_info->UE_sched_ctrl[ue_id];
    const NR_mac_stats_t* uestats = &UE_info->mac_stats[ue_id];
    mac_ue_stats_impl_t* rd = &data->ue_stats[i];

    rd->dl_aggr_tbs = uestats->dlsch_total_bytes;
    rd->ul_aggr_tbs = uestats->ulsch_total_bytes_rx;


    rd->rnti = UE_info->rnti[ue_id];
    rd->dl_aggr_prb = uestats->dlsch_total_rbs; 
    rd->ul_aggr_prb = uestats->ulsch_total_rbs;
    rd->dl_aggr_retx_prb = uestats->dlsch_total_rbs_retx ;

    rd->dl_aggr_bytes_sdus = uestats->lc_bytes_tx[3] ;
    rd->ul_aggr_bytes_sdus = uestats->lc_bytes_rx[3];

    rd->dl_aggr_sdus = uestats->dlsch_num_mac_sdu;
    rd->ul_aggr_sdus = uestats->ulsch_num_mac_sdu;

    rd->pusch_snr = (float) sched_ctrl->pusch_snrx10 / 10; //: float = -64;
    rd->pucch_snr = (float) sched_ctrl->pucch_snrx10 / 10 ; //: float = -64;

    // no CQI measurements implemented in OAI 5G yet
    rd->wb_cqi = 0; 
    rd->dl_mcs1 = sched_ctrl->sched_pdsch.mcs;
    rd->ul_mcs1 = sched_ctrl->sched_pusch.mcs ;
    rd->dl_mcs2 = 0; 
    rd->ul_mcs2 = 0; 
    rd->phr = sched_ctrl->ph; 

    ++i;
  }
}

static
uint32_t num_act_rb(NR_UE_info_t const* UE_info)
{
  assert(UE_info!= NULL);

  const NR_list_t* ue_list = &UE_info->list;

  uint32_t act_rb = 0;
  for (int ue_id = ue_list->head; ue_id >= 0; ue_id = ue_list->next[ue_id]) {
    uint16_t const rnti = UE_info->rnti[ue_id];
    for(int rb_id = 1; rb_id < 6; ++rb_id ){
      nr_rlc_statistics_t rlc = {0};
      const int srb_flag = 0;
      const int rc = nr_rlc_get_statistics(rnti, srb_flag, rb_id, &rlc);
      if(rc == 1) ++act_rb;
    }
  }
  return act_rb; 
}


static
void read_rlc_sm(rlc_ind_msg_t* data)
{
  assert(data != NULL);

  // use MAC structures to get RNTIs
  const NR_UE_info_t* UE_info = &RC.nrmac[mod_id]->UE_info;
  uint32_t const act_rb = num_act_rb(UE_info); 

  //assert(0!=0 && "Read RLC called");

  data->len = act_rb;
  if(data->len > 0){
	  data->rb = calloc(data->len, sizeof(rlc_radio_bearer_stats_t));
	  assert(data->rb != NULL && "Memory exhausted");
  }

  data->tstamp = time_now_us();

  const NR_list_t* ue_list = &UE_info->list;
  uint32_t i = 0;
  for (int ue_id = ue_list->head; ue_id >= 0; ue_id = ue_list->next[ue_id]) {
    uint16_t const rnti = UE_info->rnti[ue_id];
    //for every LC ID 
    for(int rb_id = 1; rb_id < 6; ++rb_id ){
      nr_rlc_statistics_t rb_rlc = {0};
      const int srb_flag = 0;
      const int rc = nr_rlc_get_statistics(rnti, srb_flag, rb_id, &rb_rlc);
      if(rc == 0) continue;
      rlc_radio_bearer_stats_t* sm_rb = &data->rb[i]; 

      sm_rb->txpdu_pkts = rb_rlc.txpdu_pkts; 
      sm_rb->txpdu_bytes =  rb_rlc.txpdu_bytes;        /* aggregated amount of transmitted bytes in RLC PDUs */
      sm_rb->txpdu_wt_ms =  rb_rlc.txpdu_wt_ms;      /* aggregated head-of-line tx packet waiting time to be transmitted (i.e. send to the MAC layer) */
      sm_rb->txpdu_dd_pkts = rb_rlc.txpdu_dd_pkts;      /* aggregated number of dropped or discarded tx packets by RLC */
      sm_rb->txpdu_dd_bytes = rb_rlc.txpdu_dd_bytes;     /* aggregated amount of bytes dropped or discarded tx packets by RLC */
      sm_rb->txpdu_retx_pkts = rb_rlc.txpdu_retx_pkts;    /* aggregated number of tx pdus/pkts to be re-transmitted (only applicable to RLC AM) */
      sm_rb->txpdu_retx_bytes = rb_rlc.txpdu_retx_bytes ;   /* aggregated amount of bytes to be re-transmitted (only applicable to RLC AM) */
      sm_rb->txpdu_segmented = rb_rlc.txpdu_segmented  ;    /* aggregated number of segmentations */
      sm_rb->txpdu_status_pkts = rb_rlc.txpdu_status_pkts  ;  /* aggregated number of tx status pdus/pkts (only applicable to RLC AM) */
      sm_rb->txpdu_status_bytes = rb_rlc.txpdu_status_bytes  ; /* aggregated amount of tx status bytes  (only applicable to RLC AM) */
      sm_rb->txbuf_occ_bytes = rb_rlc.txbuf_occ_bytes  ;    /* current tx buffer occupancy in terms of amount of bytes (average: NOT IMPLEMENTED) */
      sm_rb->txbuf_occ_pkts = rb_rlc.txbuf_occ_pkts  ;     /* current tx buffer occupancy in terms of number of packets (average: NOT IMPLEMENTED) */

      /* RX */
      sm_rb->rxpdu_pkts = rb_rlc.rxpdu_pkts ;         /* aggregated number of received RLC PDUs */
      sm_rb->rxpdu_bytes = rb_rlc.rxpdu_bytes ;        /* amount of bytes received by the RLC */
      sm_rb->rxpdu_dup_pkts = rb_rlc.rxpdu_dup_pkts ;     /* aggregated number of duplicate packets */
      sm_rb->rxpdu_dup_bytes = rb_rlc.rxpdu_dup_bytes ;    /* aggregated amount of duplicated bytes */
      sm_rb->rxpdu_dd_pkts = rb_rlc.rxpdu_dd_pkts ;      /* aggregated number of rx packets dropped or discarded by RLC */
      sm_rb->rxpdu_dd_bytes = rb_rlc.rxpdu_dd_bytes ;     /* aggregated amount of rx bytes dropped or discarded by RLC */
      sm_rb->rxpdu_ow_pkts = rb_rlc.rxpdu_ow_pkts ;      /* aggregated number of out of window received RLC pdu */
      sm_rb->rxpdu_ow_bytes = rb_rlc.rxpdu_ow_bytes ;     /* aggregated number of out of window bytes received RLC pdu */
      sm_rb->rxpdu_status_pkts = rb_rlc.rxpdu_status_pkts ;  /* aggregated number of rx status pdus/pkts (only applicable to RLC AM) */
      sm_rb->rxpdu_status_bytes = rb_rlc.rxpdu_status_bytes ; /* aggregated amount of rx status bytes  (only applicable to RLC AM) */

      sm_rb->rxbuf_occ_bytes = rb_rlc.rxbuf_occ_bytes ;    /* current rx buffer occupancy in terms of amount of bytes (average: NOT IMPLEMENTED) */
      sm_rb->rxbuf_occ_pkts = rb_rlc.rxbuf_occ_pkts ;     /* current rx buffer occupancy in terms of number of packets (average: NOT IMPLEMENTED) */

      /* TX */
      sm_rb->txsdu_pkts = rb_rlc.txsdu_pkts ;         /* number of SDUs delivered */
      sm_rb->txsdu_bytes = rb_rlc.txsdu_bytes ;        /* number of bytes of SDUs delivered */

      /* RX */
      sm_rb->rxsdu_pkts = rb_rlc.rxsdu_pkts ;         /* number of SDUs received */
      sm_rb->rxsdu_bytes = rb_rlc.rxsdu_bytes ;        /* number of bytes of SDUs received */
      sm_rb->rxsdu_dd_pkts = rb_rlc.rxsdu_dd_pkts ;      /* number of dropped or discarded SDUs */
      sm_rb->rxsdu_dd_bytes = rb_rlc.rxsdu_dd_bytes ;     /* number of bytes of SDUs dropped or discarded */

      sm_rb->mode = rb_rlc.mode;               /* 0: RLC AM, 1: RLC UM, 2: RLC TM */
      sm_rb->rnti = rnti;
      sm_rb->rbid = rb_id;

      ++i;
    }
  }
}


static
void read_pdcp_sm(pdcp_ind_msg_t* data)
{
  assert(data != NULL);

  //assert(0!=0 && "Calling PDCP");
  // for the moment and while we don't have a split base station, we use the
  // MAC structures to obtain the RNTIs which we use to query the PDCP
  const NR_UE_info_t* UE_info = &RC.nrmac[mod_id]->UE_info;
  uint32_t const act_rb = num_act_rb(UE_info); 

  data->len = act_rb;
  data->tstamp = time_now_us();
//  data->slot = 0;

  if(data->len > 0){
    data->rb = calloc(data->len , sizeof(pdcp_radio_bearer_stats_t));
    assert(data->rb != NULL && "Memory exhausted!");
  }

  size_t i = 0;
  const NR_list_t* ue_list = &UE_info->list;
  for (int ue_id = ue_list->head; ue_id >= 0; ue_id = ue_list->next[ue_id]) {

    const int rnti = UE_info->rnti[ue_id];
    for(size_t rb_id = 1; rb_id < 6; ++rb_id){
      nr_pdcp_statistics_t pdcp = {0};

      const int srb_flag = 0;
      const int rc = nr_pdcp_get_statistics(rnti, srb_flag, rb_id, &pdcp);

      if(rc == 0) continue;

      pdcp_radio_bearer_stats_t* rd = &data->rb[i];


      rd->txpdu_pkts = pdcp.txpdu_pkts ;     /* aggregated number of tx packets */
      rd->txpdu_bytes = pdcp.txpdu_bytes;    /* aggregated bytes of tx packets */
      rd->txpdu_sn = pdcp.txpdu_sn ;       /* current sequence number of last tx packet (or TX_NEXT) */
      rd->rxpdu_pkts = pdcp.rxpdu_pkts ;     /* aggregated number of rx packets */
      rd->rxpdu_bytes = pdcp.rxpdu_bytes ;    /* aggregated bytes of rx packets */
      rd->rxpdu_sn = pdcp.rxpdu_sn ;       /* current sequence number of last rx packet (or  RX_NEXT) */
      rd->rxpdu_oo_pkts = pdcp.rxpdu_oo_pkts  ;       /* aggregated number of out-of-order rx pkts  (or RX_REORD) */
      rd->rxpdu_oo_bytes = pdcp.rxpdu_oo_bytes  ; /* aggregated amount of out-of-order rx bytes */
      rd->rxpdu_dd_pkts = pdcp.rxpdu_dd_pkts  ;  /* aggregated number of duplicated discarded packets (or dropped packets because of other reasons such as integrity failure) (or RX_DELIV) */
      rd->rxpdu_dd_bytes = pdcp.rxpdu_dd_bytes; /* aggregated amount of discarded packets' bytes */
      rd->rxpdu_ro_count = pdcp.rxpdu_ro_count  ; /* this state variable indicates the COUNT value following the COUNT value associated with the PDCP Data PDU which triggered t-Reordering. (RX_REORD) */
      rd->txsdu_pkts = pdcp.txsdu_pkts ;     /* number of SDUs delivered */
      rd->txsdu_bytes = pdcp.txsdu_bytes ;    /* number of bytes of SDUs delivered */
      rd->rxsdu_pkts = pdcp.rxsdu_pkts ;     /* number of SDUs received */
      rd->rxsdu_bytes = pdcp.rxsdu_bytes ;    /* number of bytes of SDUs received */
      rd->rnti = rnti; 
      rd->mode = pdcp.mode;               /* 0: PDCP AM, 1: PDCP UM, 2: PDCP TM */
      rd->rbid = rb_id;

      ++i;
    }
  }
}

//////////////////////////////////////
// MCS Control  
/////////////////////////////////////

#include "openair2/LAYER2/NR_MAC_COMMON/nr_mac_extern.h"

// mcs_has_changed = PTHREAD_COND_INITIALIZER;
// mcs_lock = PTHREAD_MUTEX_INITIALIZER;

ul_dl_mcs_conf_timestamp_t ul_mcs_value;
ul_dl_mcs_conf_timestamp_t dl_mcs_value;
pthread_cond_t dl_mcs_has_changed = PTHREAD_COND_INITIALIZER;
pthread_mutex_t dl_mcs_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ul_mcs_has_changed = PTHREAD_COND_INITIALIZER;
pthread_mutex_t ul_mcs_lock = PTHREAD_MUTEX_INITIALIZER;

static
void read_mcs_sm(mcs_ind_msg_t* data){
  assert(data != NULL);

  data->tstamp = time_now_us();
  const NR_UE_info_t* UE_info = &RC.nrmac[mod_id]->UE_info;
  data->ue_mcs_conf.len_ue_mcs = UE_info->num_UEs;
  data->mcs_conf.dl.len_mcss = UE_info->num_UEs;
  data->mcs_conf.ul.len_mcss = UE_info->num_UEs;

  if(data->ue_mcs_conf.len_ue_mcs > 0){
    data->ue_mcs_conf.ues = calloc(data->ue_mcs_conf.len_ue_mcs, sizeof(ue_mcs_assoc_t));
    assert( data->ue_mcs_conf.ues != NULL && "Memory exhausted" );
  }

  if(data->mcs_conf.dl.len_mcss > 0){
    data->mcs_conf.dl.mcss = calloc(data->mcs_conf.dl.len_mcss, sizeof(mcs_params_t));
    assert( data->mcs_conf.dl.mcss != NULL && "Memory exhausted" );
  }

  if(data->mcs_conf.ul.len_mcss > 0){
    data->mcs_conf.ul.mcss = calloc(data->mcs_conf.ul.len_mcss, sizeof(mcs_params_t));
    assert( data->mcs_conf.ul.mcss != NULL && "Memory exhausted" );
  }

  const NR_list_t* ue_list = &UE_info->list;
  size_t i = 0;
  
  for (int ue_id = ue_list->head; ue_id >= 0; ue_id = ue_list->next[ue_id]) {
    const NR_UE_sched_ctrl_t* sched_ctrl = &UE_info->UE_sched_ctrl[ue_id];
    mcs_params_t* dl_rd = &data->mcs_conf.dl.mcss[i];
    mcs_params_t* ul_rd = &data->mcs_conf.ul.mcss[i];

    dl_rd->id = UE_info->rnti[ue_id];
    if(dl_mcs_value.mcs_conf.mcss == NULL){
      dl_rd->type = MCS_ALG_SM_V0_ORIGIN;
    }
    else{
      dl_rd->type = dl_mcs_value.mcs_conf.mcss[i].type;
    }
    dl_rd->sta.mcs_value = sched_ctrl->sched_pdsch.mcs;
    
    ul_rd->id = UE_info->rnti[ue_id];
    if(ul_mcs_value.mcs_conf.mcss == NULL){
      ul_rd->type = MCS_ALG_SM_V0_ORIGIN;
    }
    else{
      ul_rd->type = ul_mcs_value.mcs_conf.mcss[i].type;
    }
    ul_rd->sta.mcs_value = sched_ctrl->sched_pusch.mcs;

    ++i;
  }
}



static
void read_RAN(sm_ag_if_rd_t* data)
{
  assert(data != NULL);
  assert(data->type == MAC_STATS_V0 
        || data->type == RLC_STATS_V0
        || data->type == PDCP_STATS_V0
        || data->type == MCS_STATS_V0
        );

  if(data->type == MAC_STATS_V0 ){
    read_mac_sm(&data->mac_stats.msg);
  //  printf("calling REAd MAC\n");
  }else if(data->type == RLC_STATS_V0) {
    read_rlc_sm(&data->rlc_stats.msg);
  //  printf("calling REAd RLC\n");
  } else if(data->type == PDCP_STATS_V0){
    read_pdcp_sm(&data->pdcp_stats.msg);
    //printf("calling REAd PDCP\n");
//  } else if(RRC_STATS_V0){
//    read_rrc_sm(&data->rrc_stats);
  } else if (data->type == MCS_STATS_V0){
    // Todo
    read_mcs_sm(&data->mcs_stats.msg);
  }
  else {
    assert(0!=0 && "Unknown data type!");
  }

}

//////////////////////////////////////
// MCS Control  
/////////////////////////////////////

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

  return dst;
}

static
void add_mcs_values(mcs_conf_t const* data){
  assert(data != NULL);

  const NR_UE_info_t* UE_info = &RC.nrmac[mod_id]->UE_info;
  const size_t num_ues = UE_info->num_UEs;

  assert(data->ul.len_mcss == num_ues && data->dl.len_mcss == num_ues);
  assert(data->ul.len_mcss != 0 && "no UE connected!");

  int64_t cur_time = time_now_us();
  int64_t last_time = ul_mcs_value.tstamp;
  
  ul_mcs_value.tstamp = cur_time;
  ul_mcs_value.mcs_conf.len_mcss = data->ul.len_mcss;
  ul_mcs_value.mcs_conf.mcss = calloc(data->ul.len_mcss, sizeof(mcs_params_t));
  assert(ul_mcs_value.mcs_conf.mcss != NULL);
  for (size_t i = 0; i < data->ul.len_mcss; i++)
  {
    ul_mcs_value.mcs_conf.mcss[i] = cp_mcs_params(&data->ul.mcss[i]);
    printf("u_id %ld: %d\n", i, ul_mcs_value.mcs_conf.mcss[i].id);
    printf("u_value %ld: %d\n", i, ul_mcs_value.mcs_conf.mcss[i].sta.mcs_value);
    printf("u_type %ld: %d\n", i, ul_mcs_value.mcs_conf.mcss[i].type);
  }
  
  dl_mcs_value.tstamp = cur_time;
  dl_mcs_value.mcs_conf.len_mcss = data->dl.len_mcss;
  dl_mcs_value.mcs_conf.mcss = calloc(data->dl.len_mcss, sizeof(mcs_params_t));
  assert(dl_mcs_value.mcs_conf.mcss != NULL);
  for (size_t i = 0; i < data->dl.len_mcss; i++)
  {
    dl_mcs_value.mcs_conf.mcss[i] = cp_mcs_params(&data->dl.mcss[i]);
    printf("d_id %ld: %d\n", i, dl_mcs_value.mcs_conf.mcss[i].id);
    printf("d_value %ld: %d\n", i, dl_mcs_value.mcs_conf.mcss[i].sta.mcs_value);
    printf("d_type %ld: %d\n", i, dl_mcs_value.mcs_conf.mcss[i].type);
  }

  if(last_time != cur_time){
    pthread_mutex_lock(&dl_mcs_lock);
    pthread_cond_signal(&dl_mcs_has_changed);
    printf("DL MCS control signal sent!\n");
    pthread_mutex_unlock(&dl_mcs_lock);

    pthread_mutex_lock(&ul_mcs_lock);
    pthread_cond_signal(&ul_mcs_has_changed);
    printf("UL MCS control signal sent!\n");
    pthread_mutex_unlock(&ul_mcs_lock);
  }
}

static
void write_mcs_sm(mcs_ctrl_msg_t const* data){
  assert(data != NULL);
  assert(data->type == MCS_CTRL_SM_V0_ADD
        || data->type == MCS_CTRL_SM_V0_DEL
        || data->type == MCS_CTRL_SM_V0_UE_MCS_ASSOC);
  
  if(data->type == MCS_CTRL_SM_V0_ADD){
    add_mcs_values(&data->add_mod_mcs);
  }
  else if (data->type == MCS_CTRL_SM_V0_DEL){
    assert(0!=0 && "Not implemented!");
  }
  else if (data->type == MCS_CTRL_SM_V0_UE_MCS_ASSOC){
    assert(0!=0 && "Not implemented!");
  }
  else{
    assert(0!=0 && "Unknown data type!");
  }
  
}

static
sm_ag_if_ans_t write_RAN(sm_ag_if_wr_t const* data)
{
  assert(data != NULL);
  assert(data->type == SUBSCRIBE_TIMER  
         || data->type == MCS_CTRL_REQ_V0 );

  printf("Current control data type: %d\n", data->type);

  if(data->type == SUBSCRIBE_TIMER){
    printf("SUBSCRIBE_TIMER called with ms = %u \n", data->sub_timer.ms);
    sm_ag_if_ans_t ans = {.type = MCS_AGENT_IF_CTRL_ANS_V0  };
    return ans;
  } 

  assert(data->type == MCS_CTRL_REQ_V0 && "Unsupported control command!");
  // get the mcs values and send the mcs_change signal
  write_mcs_sm(&data->mcs_req_ctrl.msg);
  char* str = "MCS write OK!";

  mcs_ctrl_out_t out = {.len_diag = strlen(str), .diagnostic = str};

  sm_ag_if_ans_t ans = {.type = MCS_AGENT_IF_CTRL_ANS_V0, .mcs = out};

  return ans;
}

int main( int argc, char **argv ) {
  int ru_id, CC_id = 0;
  start_background_system();

  ///static configuration for NR at the moment
  if ( load_configmodule(argc,argv,CONFIG_ENABLECMDLINEONLY) == NULL) {
    exit_fun("[SOFTMODEM] Error, configuration module init failed\n");
  }

  set_softmodem_sighandler();
#ifdef DEBUG_CONSOLE
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
#endif
  mode = normal_txrx;
  memset(&openair0_cfg[0],0,sizeof(openair0_config_t)*MAX_CARDS);
  memset(tx_max_power,0,sizeof(int)*MAX_NUM_CCs);
  logInit();
  configure_linux();
  printf("Reading in command-line options\n");
  get_options ();

  if (CONFIG_ISFLAGSET(CONFIG_ABORT) ) {
    fprintf(stderr,"Getting configuration failed\n");
    exit(-1);
  }


  openair0_cfg[0].threequarter_fs = threequarter_fs;
  AMF_MODE_ENABLED = get_softmodem_params()->sa;
  NGAP_CONF_MODE   = get_softmodem_params()->sa;

  if (get_softmodem_params()->do_ra)
    AssertFatal(get_softmodem_params()->phy_test == 0,"RA and phy_test are mutually exclusive\n");

  if (get_softmodem_params()->sa)
    AssertFatal(get_softmodem_params()->phy_test == 0,"Standalone mode and phy_test are mutually exclusive\n");

#if T_TRACER
  T_Config_Init();
#endif
  //randominit (0);
  set_taus_seed (0);
  printf("configuring for RAU/RRU\n");

  if (opp_enabled ==1) {
    reset_opp_meas();
  }

  cpuf=get_cpu_freq_GHz();
  itti_init(TASK_MAX, tasks_info);
  // initialize mscgen log after ITTI
  MSC_INIT(MSC_E_UTRAN, ADDED_QUEUES_MAX+TASK_MAX);
  init_opt();
  if(PDCP_USE_NETLINK)
    if(!IS_SOFTMODEM_NOS1)
      netlink_init();
  
#ifndef PACKAGE_VERSION
#  define PACKAGE_VERSION "UNKNOWN-EXPERIMENTAL"
#endif
  LOG_I(HW, "Version: %s\n", PACKAGE_VERSION);

  if (RC.nb_nr_L1_inst > 0)
    RCconfig_NR_L1();

  // don't create if node doesn't connect to RRC/S1/GTP
  AssertFatal(create_gNB_tasks(1) == 0,"cannot create ITTI tasks\n");
  /* Start the agent. If it is turned off in the configuration, it won't start */
  /*
  RCconfig_nr_flexran();

  for (i = 0; i < RC.nb_nr_L1_inst; i++) {
    flexran_agent_start(i);
  }
  */
  // init UE_PF_PO and mutex lock
  pthread_mutex_init(&ue_pf_po_mutex, NULL);
  memset (&UE_PF_PO[0][0], 0, sizeof(UE_PF_PO_t)*NUMBER_OF_UE_MAX*MAX_NUM_CCs);
  mlockall(MCL_CURRENT | MCL_FUTURE);
  pthread_cond_init(&sync_cond,NULL);
  pthread_mutex_init(&sync_mutex, NULL);
  usleep(1000);

  if (NFAPI_MODE) {
    printf("NFAPI*** - mutex and cond created - will block shortly for completion of PNF connection\n");
    pthread_cond_init(&sync_cond,NULL);
    pthread_mutex_init(&sync_mutex, NULL);
  }

  const char *nfapi_mode_str = "<UNKNOWN>";

  switch(NFAPI_MODE) {
    case 0:
      nfapi_mode_str = "MONOLITHIC";
      break;

    case 1:
      nfapi_mode_str = "PNF";
      break;

    case 2:
      nfapi_mode_str = "VNF";
      break;

    default:
      nfapi_mode_str = "<UNKNOWN NFAPI MODE>";
      break;
  }

  printf("NFAPI MODE:%s\n", nfapi_mode_str);

  if (NFAPI_MODE==NFAPI_MODE_VNF)
    wait_nfapi_init("main?");

  printf("START MAIN THREADS\n");
  // start the main threads
  number_of_cards = 1;
  printf("RC.nb_nr_L1_inst:%d\n", RC.nb_nr_L1_inst);

  if (RC.nb_nr_L1_inst > 0) {
    printf("Initializing gNB threads single_thread_flag:%d wait_for_sync:%d\n", single_thread_flag,wait_for_sync);
    init_gNB(single_thread_flag,wait_for_sync);
  }

  printf("wait_gNBs()\n");
  wait_gNBs();
  printf("About to Init RU threads RC.nb_RU:%d\n", RC.nb_RU);

  if (RC.nb_RU >0) {
    printf("Initializing RU threads\n");
    init_NR_RU(get_softmodem_params()->rf_config_file);

    for (ru_id=0; ru_id<RC.nb_RU; ru_id++) {
      RC.ru[ru_id]->rf_map.card=0;
      RC.ru[ru_id]->rf_map.chain=CC_id+chain_offset;
    }
  }

  config_sync_var=0;

//////////////////////////////////
//////////////////////////////////
//// Init the E2 Agent

  sleep(2);
  const gNB_RRC_INST* rrc = RC.nrrrc[mod_id];
  assert(rrc != NULL && "rrc cannot be NULL");

  const int mcc = rrc->configuration.mcc[0]; // 208; 
  const int mnc = rrc->configuration.mnc[0]; // 94; 
  const int mnc_digit_len = rrc->configuration.mnc_digit_length[0]; // 2;
  const int nb_id = rrc->configuration.cell_identity; //42;
  sm_io_ag_t io = {.read = read_RAN, .write = write_RAN};

  printf("[E2 NODE]: mcc = %d mnc = %d mnc_digit = %d nd_id = %d \n", mcc, mnc, mnc_digit_len, nb_id);

  init_agent_api( mcc, mnc, mnc_digit_len, nb_id, io);
//////////////////////////////////
//////////////////////////////////

  if (NFAPI_MODE==NFAPI_MODE_PNF) {
    wait_nfapi_init("main?");
  }

  if (RC.nb_nr_L1_inst > 0) {
    printf("wait RUs\n");
    wait_RUs();
    printf("ALL RUs READY!\n");
    printf("RC.nb_RU:%d\n", RC.nb_RU);
    // once all RUs are ready initialize the rest of the gNBs ((dependence on final RU parameters after configuration)
    printf("ALL RUs ready - init gNBs\n");

    if(IS_SOFTMODEM_DOSCOPE) {
      sleep(1);
      scopeParms_t p;
      p.argc=&argc;
      p.argv=argv;
      p.gNB=RC.gNB[0];
      p.ru=RC.ru[0];
      load_softscope("nr",&p);
    }

    if (NFAPI_MODE != NFAPI_MODE_PNF && NFAPI_MODE != NFAPI_MODE_VNF) {
      printf("Not NFAPI mode - call init_eNB_afterRU()\n");
      init_eNB_afterRU();
    } else {
      printf("NFAPI mode - DO NOT call init_gNB_afterRU()\n");
    }

    printf("ALL RUs ready - ALL gNBs ready\n");
    // connect the TX/RX buffers
    printf("Sending sync to all threads\n");
    pthread_mutex_lock(&sync_mutex);
    sync_var=0;
    pthread_cond_broadcast(&sync_cond);
    pthread_mutex_unlock(&sync_mutex);
  }

  printf("About to call end_configmodule() from %s() %s:%d\n", __FUNCTION__, __FILE__, __LINE__);
  // We have to set PARAMFLAG_NOFREE on right paramters before re-enabling end_configmodule()
  //end_configmodule();
  printf("Called end_configmodule() from %s() %s:%d\n", __FUNCTION__, __FILE__, __LINE__);
  // wait for end of program
  printf("TYPE <CTRL-C> TO TERMINATE\n");
  //getchar();
  printf("Entering ITTI signals handler\n");
  itti_wait_tasks_end();
  printf("Returned from ITTI signal handler\n");
  oai_exit=1;
  printf("oai_exit=%d\n",oai_exit);
  // stop threads
  /*#ifdef XFORMS

      printf("waiting for XFORMS thread\n");

      if (do_forms==1) {
        pthread_join(forms_thread,&status);
        fl_hide_form(form_stats->stats_form);
        fl_free_form(form_stats->stats_form);

          fl_hide_form(form_stats_l2->stats_form);
          fl_free_form(form_stats_l2->stats_form);

          for(UE_id=0; UE_id<scope_enb_num_ue; UE_id++) {
      for(CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {
        fl_hide_form(form_enb[CC_id][UE_id]->phy_scope_gNB);
        fl_free_form(form_enb[CC_id][UE_id]->phy_scope_gNB);
      }
          }
      }

  #endif*/
  printf("stopping MODEM threads\n");
  // cleanup
  stop_gNB(NB_gNB_INST);

  if (RC.nb_nr_L1_inst > 0) {
    stop_RU(NB_RU);
  }

  /* release memory used by the RU/gNB threads (incomplete), after all
   * threads have been stopped (they partially use the same memory) */
  for (int inst = 0; inst < NB_gNB_INST; inst++) {
    //free_transport(RC.gNB[inst]);
    phy_free_nr_gNB(RC.gNB[inst]);
  }

  for (int inst = 0; inst < NB_RU; inst++) {
    nr_phy_free_RU(RC.ru[inst]);
  }

  free_lte_top();
  pthread_cond_destroy(&sync_cond);
  pthread_mutex_destroy(&sync_mutex);
  pthread_cond_destroy(&nfapi_sync_cond);
  pthread_mutex_destroy(&nfapi_sync_mutex);
  pthread_mutex_destroy(&ue_pf_po_mutex);

  //////////////////////////////////////
  // MCS Control  
  /////////////////////////////////////

  pthread_mutex_destroy(&dl_mcs_lock);
  pthread_cond_destroy(&dl_mcs_has_changed);
  pthread_mutex_destroy(&ul_mcs_lock);
  pthread_cond_destroy(&ul_mcs_has_changed);

  // *** Handle per CC_id openair0

  for(ru_id=0; ru_id<NB_RU; ru_id++) {
    if (RC.ru[ru_id]->rfdevice.trx_end_func)
      RC.ru[ru_id]->rfdevice.trx_end_func(&RC.ru[ru_id]->rfdevice);

    if (RC.ru[ru_id]->ifdevice.trx_end_func)
      RC.ru[ru_id]->ifdevice.trx_end_func(&RC.ru[ru_id]->ifdevice);
  }

  logClean();
  printf("Bye.\n");
  return 0;
}
