============================================
Analog Signal Builder (ASB) Falaise plugin
============================================
:Authors: G.Oliviéro <goliviero@lpccaen.in2p3.fr>,
	  Y.Lemière <lemiere@lpccaen.in2p3.fr>,
	  F.Mauger <mauger@lpccaen.in2p3.fr>,
:Date:    29/11/2016
   ..

The Analog Signal  Builder (ASB) module is a  Falaise plugin dedicated
to the first processing stage of SuperNEMO Monte Carlo data.

It aims  to process  truth simulated  data (`SD`  bank) produced  by the
`flsimulate`  Monte  Carlo simulation  program  and  generate models  of
analog signals associated to calorimeter and tracker hits. The result is
then stored in the `SSD` bank (*simulated signal data*).


UML diagram of the process :
-----------------------------------------

PRELIMINARY
::



                                        +------------------+
                                        |   Calo signal    |
                                        |   Calo signal    |
          |***********************|  /->|   Calo signal    |
       /->| SD_to_calo_signal_algo| /   |   Calo signal    |
      /   |***********************+/    |   Calo signal    |
     /                 |          /     +------------------+
    /                  v         /
 +-----+          |-------------|
 | SD  |          | Signal Data |
 +-----+          |-------------|
    \                    ^     \        +------------------+
     \                   |      \       |  Geiger signal   |
      \                  |       \      |  Geiger signal   |
       \                 |        \     |  Geiger signal   |
        \   |********************| \    |  Geiger signal   |
         \->|SD_to_GG_signal_algo|  \   |  Geiger signal   |
            |********************|   \->|  Geiger signal   |
                                        |  Geiger signal   |
                                        |  Geiger signal   |
                                        |  Geiger signal   |
                                        |  Geiger signal   |
                                        |  Geiger signal   |
                                        +------------------+
..
