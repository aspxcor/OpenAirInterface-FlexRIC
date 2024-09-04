# OpenAirInterface-FlexRIC
## OpenAirInterface project
OpenAirInterface (OAI) project is an open source project maintained and developed by OSA (The OpenAirInterface Software Alliance) , the founder is Eurecom. the organization is committed to 3GPP (The 3rd Generation Partnership Project) cellular networks related to the development and maintenance of open source software ecosystem. Project) cellular networks related to 3GPP (The 3rd Generation Partnership Project) packet core network, radio access network and user equipment and other open source software ecosystem development and maintenance.
### The main components
#### Software component
`openair-cn`: simulation of LTE EPC (Evolved Packet Core), mainly composed of HSS (Home Subscriber Server), MME (Mobility Management Entity), SGW ( Serving Gateway), PGW (PDN Gateway) and other network elements.
`openairinterface5g`: Simulation of terminal and base station side. The project can be compiled into a base station (eNodeB) or user terminal (UE) by changing the compilation options according to requirements. When emulating a base station, it provides 3GPP-compliant S1 interfaces, including the control-plane S1-MME interface and the user-plane S1-U interface.
#### Hardware component
The OAI platform requires Intel-based PCs to deploy base stations or user terminals, and has been designed to be compatible with a variety of RF hardware platforms and to interface easily with third-party SDR (Software Defined Radio) peripherals, including the Eurecom XMIMO2 RF platform that is currently officially supported, USRP (Universal Software Radio Peripheral) B and X series, BladeRF and LimeSDR.
### Features and drawbacks
#### Features
Provides a flexible, customizable solution for IoT applications, developers can use the project to quickly build and customize their own IoT solutions.
#### Drawbacks
Only provide open source code, and does not provide technical support, and the lack of related description documents, part of the code is written in a chaotic manner, the code structure needs to be improved; the development of high-level protocols is incomplete, most of the high-level related functions have not been implemented, and many of the implemented functions are simplified versions; the version of the open source code is unstable, and a lot of the code is uploaded without rigorous review, which results in the unavailability of the version, and reduces the efficiency of the development and use of the project.
## FlexRIC Project
FlexRIC is a flexible intelligent controller project for wireless access networks, which is an important part of the O-RAN architecture. It mainly consists of three parts: E2 Agent, RIC (Radio Intelligent Controller) and Xapp.
### Function
#### SM (Service Model)
In the form of dynamic library for E2 Agent and Xapp. SM is responsible for calling RAN function to collect data and send control commands to the radio access network, and providing corresponding control interface and data to Xapp.
#### Providing flexible control capability
It can realize intelligent control and management of the radio access network and help operators better optimize network performance, improve resource utilization, and adapt to different service requirements and scenarios.
#### Significance
It plays an important role in the development of O-RAN architecture, promotes the development of radio access network in the direction of more open, intelligent and flexible, and provides more powerful technical support and innovation space for 5G and future wireless communication network.
