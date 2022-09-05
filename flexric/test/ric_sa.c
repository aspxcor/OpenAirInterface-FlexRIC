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

#include <arpa/inet.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>


const uint16_t MAC_ran_func_id = 142;
const uint16_t RLC_ran_func_id = 143;
const uint16_t PDCP_ran_func_id = 144;
const uint16_t SLICE_ran_func_id = 145; // Not implemented yet
const uint16_t MCS_ran_func_id = 200;
const char* cmd = "5_ms";

static inline
bool e2_node_connected()
{
  size_t const sz = e2_nodes();
  return sz != 0;
}

static
void sig_handler(int sig_num)
{
  printf("\nEnding the near-RIC with signal number = %d\n", sig_num);

  if(e2_node_connected() == true ) {
    rm_report_service_near_ric_api(MAC_ran_func_id, cmd);
    rm_report_service_near_ric_api(RLC_ran_func_id, cmd);
    rm_report_service_near_ric_api(PDCP_ran_func_id, cmd);
    rm_report_service_near_ric_api(MCS_ran_func_id, cmd);
  } else {
    printf("[NEAR-RIC]: E2 Node never connected \n");
  }

  // Stop the RIC
  stop_near_ric_api();

  exit(0);
}


static inline
void wait_e2_node()
{
  while(1){
   sleep(1);
    if(e2_node_connected() == true){
      sleep(1);
      break;
    }
  }
}

int main()
{
  // Signal handler
  signal(SIGINT, sig_handler);

  // Init the RIC
  init_near_ric_api();

  wait_e2_node();

  report_service_near_ric_api(MAC_ran_func_id, cmd );

  report_service_near_ric_api(RLC_ran_func_id, cmd);

  report_service_near_ric_api(PDCP_ran_func_id, cmd);

  report_service_near_ric_api(MCS_ran_func_id, cmd);

  printf("RAN function OK!\n");

  // test MCS control
  // MCS control parameters
  char input[10];
  int initial_mcs = 24;
  int* input_value = &initial_mcs;
  const char* cmd2 = "ADD";
  
  while(1){
    printf("Waiting for MCS control...\n");
    scanf( "%s", input );
    if( strcmp( input, "MCS" ) == 0 ){
      printf("Please input MCS value (1-28)...\n");
      scanf( "%d", input_value );
      added_mod_mcs.dl.len_mcss = 1;
      added_mod_mcs.dl.mcss = calloc(added_mod_mcs.dl.len_mcss, sizeof(mcs_params_t));
      for (size_t i = 0; i < added_mod_mcs.dl.len_mcss; i++)
      {
        added_mod_mcs.dl.mcss[i].id = 0;
        added_mod_mcs.dl.mcss[i].type = MCS_ALG_SM_V0_STATIC;
        added_mod_mcs.dl.mcss[i].sta.mcs_value = *input_value;
      }
      
      added_mod_mcs.ul.len_mcss = 1;
      added_mod_mcs.ul.mcss = calloc(added_mod_mcs.ul.len_mcss, sizeof(mcs_params_t));
      for (size_t i = 0; i < added_mod_mcs.ul.len_mcss; i++)
      {
        added_mod_mcs.ul.mcss[i].id = 1;
        added_mod_mcs.ul.mcss[i].type = MCS_ALG_SM_V0_ORIGIN;
      }
      printf("Prepare MCS values OK!\n");
      control_service_near_ric_api(MCS_ran_func_id, cmd2 );
    }
    sleep(5);
  }

}

