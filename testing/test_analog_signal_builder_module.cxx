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

// Falaise:
#include <falaise/falaise.h>

// This project :
#include <snemo/asb/analog_signal_builder_module.h>


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
  falaise::terminate();
  return (error_code);
}

void test_asbm_1(const params_type & params_)
{
  std::clog << "[info] test_asbm_1..." << std::endl;

  std::string SD_bank_label   = "SD";  // Simulated Data "SD" bank label :
  std::string SSD_bank_label  = "SSD"; // Simulated Signal Data "SD" bank label :
  std::string calo_hit_category    = "calo";
  std::string calo_signal_category = "sigcalo";
  std::string tracker_hit_category = "tracker";
  std::string tracker_signal_category = "sigtracker";

  mctools::signal::signal_shape_builder ssb1;
  ssb1.set_logging_priority(datatools::logger::PRIO_FATAL);
  ssb1.set_category(signal_category);
  ssb1.add_registered_shape_type_id("mctools::signal::triangle_signal_shape");
  ssb1.add_registered_shape_type_id("mctools::signal::triangle_gate_signal_shape");
  ssb1.add_registered_shape_type_id("mctools::signal::multi_signal_shape");
  ssb1.add_registered_shape_type_id("mygsl::linear_combination_function");
  ssb1.initialize_simple();
  ssb1.tree_dump(std::clog, "Signal shape builder 1", "[info] ");

  std::string SD_filename
    = "${FALAISE_ASB_TESTING_DIR}/data/Se82_0nubb-source_strips_bulk_SD_10_events.brio";
  int event_number = 5;
  if (params_.is_event_number){
    event_number = params_.arg_event_number;
  }

  // Event reader :
  dpp::input_module reader;
  datatools::properties reader_config;
  reader_config.store("logging.priority", "notice");
  reader_config.store("max_record_total", event_number);
  reader_config.store("files.mode", "single");
  reader_config.store("files.single.filename", SD_filename);
  reader.initialize_standalone(reader_config);

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
      mctools::signal::signal_data SSD;

      if (SD.has_step_hits(hit_category)) {
        std::clog << "[info] Found '" << hit_category << "' hits..." << std::endl;
        // CSGD1.process(SD, SSD);
        std::clog << "[info] SSD size = " << SSD.get_number_of_signals(signal_category) << std::endl;
      }

      ER.clear();
      psd_count++;
      DT_LOG_NOTICE(params_.logging, "Simulated data #" << psd_count);
    }
  } // end of reader

  std::clog << std::endl;
  return;
}
