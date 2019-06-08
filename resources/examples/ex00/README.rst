AnalogSignalBuilder Falaise module
=====================================

Introduction
------------

This example shows how to use the ASB module with ``flreconstruct``. A
pipeline is  defined that processes  the SuperNEMO simulated  data (SD
bank from the flsimulate output) to produce simulated signal data (SSD
bank)

Configuration file(s):

* ``asb-reco.conf``  :  the ``flreconstruct``  pipeline  configuration
  files that describes the chain of modules to be processed.

* ``service_manager.conf`` : the list of the files defining services.

* ``services.conf``  : the  configuration of  the services  namely the
  ``geometry`` service.


Usage
-----

  1. First setup the Falaise software.

  2. Generate a set of simulated events with ``flsimulate`` program.

  3. Run the reconstruction pipeline:

      * Run the reconstruction pipeline: ::

          $ flreconstruct \
            -p asb-reco.conf \
	    -i your_simulated_file.brio \
	    -o your_output.brio

        The ``flreconstruct``  program should perform the  pipeline as
        defined in ``asb-reco.conf``. At the end, the SSD bank will be
        produced with a collection of calo signals and a collection of
        tracker signals. It is possible  to build the shape embeded in
        each  signal   with  a   mctools::signal_shape_builder.   Only
        parameters  useful to  reconstruct the  shape are  stored. The
        shape  is a  mygsl::i_unary_function  with  an useful  eval(x)
        function.

Warning
-------

Take care of the :

* ``Falaise_AnalogSignalBuilder.directory : string`` parameter. It has
  to  be set  where your  install of  the ASB  is. You  must have  the
  library   file  :   ``libFalaise_AnalogSignalBuilder.so``  in   this
  repository.
