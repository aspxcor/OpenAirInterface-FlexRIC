cmake_minimum_required(VERSION 3.15) # setting this is required
project(mcs_sm)            # this sets the project name

set(SM_ENCODING_MCS_SRC
                      ../sm_proc_data.c 
                      mcs_sm_agent.c 
                      mcs_sm_ric.c 
                     ../../util/byte_array.c 
                     ../../util/alg_ds/alg/defer.c 
                     ../../util/alg_ds/alg/eq_float.c 
                     ../../util/alg_ds/ds/seq_container/seq_arr.c 
                     ../../util/alg_ds/ds/seq_container/seq_ring.c 
                     ../../util/alg_ds/ds/assoc_container/assoc_rb_tree.c
                     ../../util/alg_ds/ds/assoc_container/bimap.c
                      ie/mcs_control_ie.c 
                      )

if(SM_ENCODING_MCS STREQUAL "PLAIN")
  add_library(mcs_sm SHARED
                      ${SM_ENCODING_MCS_SRC}
                      enc/mcs_enc_plain.c 
                      dec/mcs_dec_plain.c 
                      )

elseif(SM_ENCODING_MCS STREQUAL "ASN" )

  message(FATAL_ERROR "MCS SM ASN not implemented")
  add_subdirectory(ie/asn)

  add_library(mcs_sm SHARED
                      ../sm_proc_data.c 
                      mcs_sm_agent.c 
                      mcs_sm_ric.c 
                      $<TARGET_OBJECTS:e2ap_ds_obj>
                      enc/mcs_enc_asn.c 
                      dec/mcs_dec_asn.c 
                      )

  target_include_directories(mcs_sm  
                            PRIVATE
                            "ie/asn")

  target_compile_options(mcs_sm PUBLIC "-DASN_DISABLE_OER_SUPPORT")
  target_compile_options(mcs_sm PRIVATE -Wno-missing-field-initializers -Wno-unused-parameter)

elseif(SM_ENCODING_MCS STREQUAL "FLATBUFFERS" )

  message(FATAL_ERROR "MCS SM FB not implemented")
  add_library(mcs_sm SHARED
                      ../sm_proc_data.c 
                      mcs_sm_agent.c 
                      mcs_sm_ric.c 
                      $<TARGET_OBJECTS:e2ap_ds_obj>
                      enc/mcs_enc_fb.c 
                      dec/mcs_dec_fb.c 
                      )
else()
  message(FATAL_ERROR "Unknown SM encoding type ")
endif()

target_compile_definitions(mcs_sm PUBLIC ${SM_ENCODING_MCS})

