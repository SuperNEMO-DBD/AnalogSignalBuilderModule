// test_analog_signal_builder_module.cxx

// Standard libraries :
#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <cstdio>

// - Bayeux/datatools:
#include <datatools/utils.h>
#include <datatools/io_factory.h>
#include <datatools/temporary_files.h>
#include <datatools/clhep_units.h>
// - Bayeux/mctools:
#include <mctools/simulated_data.h>
#include <mctools/signal/signal_shape_builder.h>
// - Bayeux/dpp:
#include <dpp/input_module.h>
#include <dpp/output_module.h>
#include <mygsl/parameter_store.h>
// - Bayeux/geomtools:
#include <geomtools/geomtools_config.h>
#include <geomtools/gnuplot_draw.h>
#if GEOMTOOLS_WITH_GNUPLOT_DISPLAY == 1
#include <geomtools/gnuplot_i.h>
#include <geomtools/gnuplot_drawer.h>
#endif // GEOMTOOLS_WITH_GNUPLOT_DISPLAY

// Boost :
#include <boost/program_options.hpp>

// Falaise:
#include <falaise/falaise.h>

// This project :
#include <snemo/asb/analog_signal_builder_module.h>
#include <snemo/asb/flasb.h>


struct params_type
{
  datatools::logger::priority logging = datatools::logger::PRIO_DEBUG;
  bool draw = false;
  int number_of_events = -1;
  std::string input_filename = "";
};

void test_asbm_1(const params_type &);

int main( int  argc_ , char **argv_  )
{
  falaise::initialize(argc_, argv_);
  snemo::asb::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  try {
    std::clog << "Test program for class 'snemo::asb::analog_signal_builder_module'!" << std::endl;

    // Parameters:
    params_type params;

    // Parse options:
    namespace po = boost::program_options;
    po::options_description opts("Allowed options");
    opts.add_options()
      ("help,h", "produce help message")
      ("draw,D", "draw option")
      ("input,i",
       po::value<std::string>(& params.input_filename),
       "set an input file")
      ("number_of_events,n",
       po::value<int>(& params.number_of_events)->default_value(5),
       "set the maximum number of events")
      ; // end of options description

    // Describe command line arguments :
    po::variables_map vm;
    po::store(po::command_line_parser(argc_, argv_)
	      .options(opts)
	      .run(), vm);
    po::notify(vm);

    // Use command line arguments :
    if (vm.count("help")) {
      std::cout << "Usage : " << std::endl;
      std::cout << opts << std::endl;
      return(error_code);
    }

    // Use command line arguments :
    if (vm.count("draw")) {
      params.draw = true;
    }

    std::clog << "First test..." << std::endl;
    test_asbm_1(params);

    std::clog << "The end." << std::endl;

  } catch (std::exception & error) {
    std::cerr << "error: " << error.what () << std::endl;
    error_code = EXIT_FAILURE;
  } catch (...) {
    std::cerr << "error: " << "Unexpected error!" << std::endl;
    error_code = EXIT_FAILURE;
  }
  snemo::asb::terminate();
  falaise::terminate();
  return (error_code);
}

void test_asbm_1(const params_type & params_)
{
  std::clog << "[info] test_asbm_1..." << std::endl;

  std::string SD_bank_label   = "SD";  // Simulated Data "SD" bank label :
  std::string SSD_bank_label  = "SSD"; // Simulated Signal Data "SSD" bank label

  std::string SD_filename = "";
  if (params_.input_filename.empty()) SD_filename = "${FALAISE_ASB_TESTING_DIR}/data/Se82_0nubb-source_strips_bulk_SD_10_events.brio";
  else SD_filename = params_.input_filename;
  datatools::fetch_path_with_env(SD_filename);

  // Geom manager :
  std::string manager_config_file;
  manager_config_file = "@falaise:config/snemo/demonstrator/geometry/4.0/manager.conf";
  datatools::fetch_path_with_env(manager_config_file);
  datatools::properties manager_config;
  datatools::properties::read_config (manager_config_file,
				      manager_config);
  geomtools::manager my_manager;
  manager_config.update ("build_mapping", true);
  if (manager_config.has_key ("mapping.excluded_categories"))
    {
      manager_config.erase ("mapping.excluded_categories");
    }
  my_manager.initialize (manager_config);
  my_manager.tree_dump(std::clog, "My geometry manager");

  // Event reader :
  dpp::input_module reader;
  datatools::properties reader_config;
  reader_config.store("logging.priority", "notice");
  reader_config.store("max_record_total", params_.number_of_events);
  reader_config.store("files.mode", "single");
  reader_config.store("files.single.filename", SD_filename);
  reader.initialize_standalone(reader_config);

  // Problem in module registration :
  std::string asbm_config_filename = "@flasb:config/snemo/demonstrator/simulation/asb/0.1/asb.conf";
  datatools::fetch_path_with_env(asbm_config_filename);
  datatools::properties asbm_config;
  asbm_config.read_configuration(asbm_config_filename);
  // asbm_config.tree_dump(std::clog, "Analog Signal Builder Module config: ", "[info] ");

  // Analog Signal Builder module:
  snemo::asb::analog_signal_builder_module asbm1;
  asbm1.set_geometry_manager(my_manager);
  asbm1.initialize_standalone(asbm_config);

  // Event record :
  datatools::things ER;

  std::size_t psd_count = 0; // Event counter
  while (!reader.is_terminated()) {
    std::clog << "[info] Processing a new event record..." << std::endl;
    reader.process(ER);
    // A plain `mctools::simulated_data' object is stored here :
    if (ER.has(SD_bank_label) && ER.is_a<mctools::simulated_data>(SD_bank_label)) {
      // Access to the "SD" bank with a stored `mctools::simulated_data' :
      const mctools::simulated_data & SD = ER.get<mctools::simulated_data>(SD_bank_label);

      asbm1.process(ER);
      // ER.tree_dump(std::clog, "an ER after processing");

      const mctools::signal::signal_data & SSD = ER.get<mctools::signal::signal_data>(SSD_bank_label);
      SSD.tree_dump(std::clog, "A SSD");

      std::string signal_category = "sigcalo";
      if (SSD.has_signals(signal_category)) {
	std::size_t number_of_calos = SSD.get_number_of_signals(signal_category);
	for (std::size_t icalo = 0; icalo < number_of_calos; icalo++) {
	  const mctools::signal::base_signal & a_calo_signal = SSD.get_signal(signal_category,icalo);
	  // a_calo_signal.tree_dump(std::clog,"A calo signal");
	}
      }

      signal_category = "sigtracker";
      if (SSD.has_signals(signal_category)) {
	std::size_t number_of_trackers = SSD.get_number_of_signals(signal_category);
	for (std::size_t itrack = 0; itrack < number_of_trackers; itrack++) {
	  const mctools::signal::base_signal & a_tracker_signal = SSD.get_signal(signal_category, itrack);
	  // a_tracker_signal.tree_dump(std::clog,"A tracker signal");
	}
      }

      ER.clear();
      psd_count++;
      DT_LOG_NOTICE(params_.logging, "Simulated data #" << psd_count);
    }
  } // end of reader

  std::clog << std::endl;
  return;
}
