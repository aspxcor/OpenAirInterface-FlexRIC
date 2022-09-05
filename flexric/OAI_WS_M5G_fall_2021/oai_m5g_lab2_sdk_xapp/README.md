# OAI M5G SDK

In this README.md, we assume you are at the folder containing this README.

## Preparing the env:

```
$ pip3 install -r requirements.txt
```

## Developing XAPP:

Please read more information in xapps/xapps.py file which is prepared for you.

## API: 

There is only one function in the API that you need to care about:

get_attr(ue = None, attr= None, windows = 1, index_is_rnti=False, remove_empty=False):

        Get attributes of RIC, please use keywords to set the value
        Args:
            ue: index of ue
                None means for every ue
                int: get the attributes of a given ue 
                    Plan: for every enb  
                list: a list of ue to extract attribute for every enbs
                list of list: specific ue for each enb. NOT SUPPORTED YET.
            
            attr: indicators of attributes [1]
                None means for every attribute
                string: get an attribute for every specific ues    
                list[string]: a list of attributes to extract
            
            windows: how many message you want to look back to the past
            
            index_is_rnti: whether if index is rnti or simply index of ue in context
            remove_empty: if set True, remove the empty item in the output

            
           [1] Check the list of indicators BELOW. 
        
        Return:
            List[List[List[ (name: List[Value]) : Attributes] : UE infos] : ENB infos]
        
        Example of returns:
            [[{'mac.ul_mcs2': [0, 0, 0], 'rlc.rxsdu_pkts': [65063, 65063, 65063], 'pdcp.rxpdu_sn': [64672, 64672, 64672]}]]

        Example to use:
        ```
        # get every things
        all_stats = get_attr()

        # get every stats for UE 1
        ue1_stats = get_attr(ue=1)

        # get stats uplink mcs2 for every ue
        ul_mcs2_stats = get_attr(attr="mac.ul_mcs2")

        # get stats uplink and downlink mcs2 for every ue
        udl_mcs2_stats = get_attr(attr=["mac.ul_mcs2", "mac.dl_mcs2"])

        # get stats uplink and downlink mcs2 for some ues
        some_ues_udl_mcs2_stats = get_attr(ue = [0,2,4,6,8,10], attr=["mac.ul_mcs2", "mac.dl_mcs2"])

        ```

Other useful functions:

api.number_connected_ue(): 
        
        Return number of connected ues

api.connected_ue_ids():
        
        Return a list of RNTI of connected UEs

api.num_pdcp_pkg(ue_id):
        
        Return number of stored pdcp msg received from a given UE

api.num_rlc_pkg(ue_id):
    
        Return number of stored rlc msg received from a given UE

api.num_mac_pkg(ue_id):
    
        Return number of stored mac msg received from a given UE

## API: Supported indicators

| Indicator              | Notes                                                                     | 
|------------------------|----------------------------------------------------------------------------
| rlc.txpdu_pkts         | aggregated number of transmitted RLC PDUs                                 | 
| rlc.txpdu_bytes        | aggregated amount of transmitted bytes in RLC PDUs                        | 
| rlc.txpdu_wt_ms        | aggregated head-of-line tx packet waiting <br>time to be transmitted (i.e. send to the MAC layer)                      | 
| rlc.txpdu_dd_pkts      | aggregated number of dropped or discarded <br>tx packets by RLC                                                         | 
| rlc.txpdu_dd_bytes     | aggregated amount of bytes dropped or discarded <br>tx packets by RLC                                                         | 
| rlc.txpdu_retx_pkts    | aggregated number of tx pdus/pkts to be <br>re-transmitted (only applicable to RLC AM)                                | 
| rlc.txpdu_retx_bytes   | aggregated amount of bytes to be <br>re-transmitted (only applicable to RLC AM)                                | 
| rlc.txpdu_segmented    | aggregated number of segmentations                                        | 
| rlc.txpdu_status_pkts  | aggregated number of tx status pdus/pkts <br>(only applicable to RLC AM)                                               | 
| rlc.txpdu_status_bytes | aggregated amount of tx status bytes <br>(only applicable to RLC AM)                                               | 
| rlc.txbuf_occ_bytes    | current tx buffer occupancy in terms of <br>amount of bytes (average: NOT IMPLEMENTED)                                | 
| rlc.txbuf_occ_pkts     | current tx buffer occupancy in terms of <br>number of packets (average: NOT IMPLEMENTED)                              | 
| rlc.rxpdu_pkts         | aggregated number of received RLC PDUs                                    | 
| rlc.rxpdu_bytes        | amount of bytes received by the RLC                                       | 
| rlc.rxpdu_dup_pkts     | aggregated number of duplicate packets                                    | 
| rlc.rxpdu_dup_bytes    | aggregated amount of duplicated bytes                                     | 
| rlc.rxpdu_dd_pkts      | aggregated number of rx packets dropped <br>or discarded by RLC                                                       | 
| rlc.rxpdu_dd_bytes     | aggregated amount of rx bytes dropped <br>or discarded by RLC                                                       | 
| rlc.rxpdu_ow_pkts      | aggregated number of out of window <br>received RLC pdu                                                          | 
| rlc.rxpdu_ow_bytes     | aggregated number of out of window bytes <br>received RLC pdu                                                          | 
| rlc.rxpdu_status_pkts  | aggregated number of rx status pdus/pkts <br>(only applicable to RLC AM)                                               | 
| rlc.rxpdu_status_bytes | aggregated amount of rx status bytes  (only applicable to RLC AM)         | 
| rlc.rxbuf_occ_bytes    | current rx buffer occupancy in terms of amount <br>of bytes (average: NOT IMPLEMENTED)                                       | 
| rlc.rxbuf_occ_pkts     | current rx buffer occupancy in terms of number of packets <br>(average: NOT IMPLEMENTED)                                                | 
| rlc.txsdu_pkts         | number of SDUs delivered                                                  | 
| rlc.txsdu_bytes        | number of bytes of SDUs delivered                                         | 
| rlc.rxsdu_pkts         | number of SDUs received                                                   | 
| rlc.rxsdu_bytes        | number of bytes of SDUs received                                          | 
| rlc.rxsdu_dd_pkts      | number of dropped or discarded SDUs                                       | 
| rlc.rxsdu_dd_bytes     | number of bytes of SDUs dropped or discarded                              | 
| rlc.rnti               | Radio Network Temporary Identifier                                                                          | 
| rlc.mode               | 0: RLC Ack Mode, 1: RLC Unack Mode, 2: RLC Transparent Mode                                           | 
| rlc.rbid               | radio bearer id                                                                          | 
| rlc.frame              | radio frame                                                                          | 
| rlc.slot               | radio slot                                                                          | 
| pdcp.txpdu_pkts        | aggregated number of tx packets                                           | 
| pdcp.txpdu_bytes       | aggregated bytes of tx packets                                            | 
| pdcp.txpdu_sn          | current sequence number of last tx packet (or TX_NEXT)                    | 
| pdcp.rxpdu_pkts        | aggregated number of rx packets                                           | 
| pdcp.rxpdu_bytes       | aggregated bytes of rx packets                                            | 
| pdcp.rxpdu_sn          | current sequence number of last rx packet (or  RX_NEXT)                   | 
| pdcp.rxpdu_oo_pkts     | aggregated number of out-of-order rx pkts  (or RX_REORD)                  | 
| pdcp.rxpdu_oo_bytes    | aggregated amount of out-of-order rx bytes                                | 
| pdcp.rxpdu_dd_pkts     | aggregated number of duplicated discarded packets (or dropped packets <br>because of other reasons such as integrity failure) (or RX_DELIV)         | 
| pdcp.rxpdu_dd_bytes    | aggregated amount of discarded packets' bytes                             | 
| pdcp.rxpdu_ro_count    | this state variable indicates the COUNT value following the COUNT value   |
|                        | associated with the PDCP Data PDU which triggered t-Reordering. (RX_REORD)| 
| pdcp.txsdu_pkts        | number of SDUs delivered                                                  | 
| pdcp.txsdu_bytes       | number of bytes of SDUs delivered                                         | 
| pdcp.rxsdu_pkts        | number of SDUs received                                                   | 
| pdcp.rxsdu_bytes       | number of bytes of SDUs received                                          | 
| pdcp.rnti              | Radio Network Temporary Identifier                                                                          | 
| pdcp.mode              | mode of pdcp                                                                          | 
| pdcp.rbid              | radio bearer                                                                          | 
| pdcp.tstamp            | time stamp                         |
| mac.dl_aggr_tbs        | downlink aggregate transport block size                                                                          | 
| mac.ul_aggr_tbs        | uplink aggregate transport block                                                                          | 
| mac.dl_aggr_bytes_sdus | downlink aggregate bytes sdus                                                                          | 
| mac.ul_aggr_bytes_sdus | uplink aggregate bytes sdus                                                                          | 
| mac.pusch_snr          | pusch snr                                                          | 
| mac.pucch_snr          | pucch snr                                                           | 
| mac.rnti               | Radio Network Temporary Identifier                                                                          | 
| mac.dl_aggr_prb        | downlink aggregate physical resource block                                                                          | 
| mac.ul_aggr_prb        | uplink aggregate physical resource block                                                                           | 
| mac.dl_aggr_sdus       | downlink aggregate sdu                                                                          | 
| mac.ul_aggr_sdus       | uplink aggregate sdu                                                                         | 
| mac.dl_aggr_retx_prb   | downlink aggregate retransmission physical resource block                                                                    | 
| mac.wb_cqi             | channel quality indicator                                                                         | 
| mac.dl_mcs1            | downlink modulation and coding scheme 1                                                                         | 
| mac.ul_mcs1            | uplink modulation and coding scheme 1                                                                         | 
| mac.dl_mcs2            | downlink modulation and coding scheme 2                                                                           | 
| mac.ul_mcs2            | uplink modulation and coding scheme 2                                                                          | 
| mac.phr                | power head room                                                          | 