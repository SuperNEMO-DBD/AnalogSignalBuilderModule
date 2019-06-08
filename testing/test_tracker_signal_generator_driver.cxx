// test_tracker_signal_generator_driver.cxx

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
// - Bayeux/dpp:
#include <dpp/input_module.h>
#include <dpp/output_module.h>
// - Bayeux/mygsl:
#include <mygsl/parameter_store.h>
#include <mygsl/i_unary_function_with_derivative.h>
// - Bayeux/geomtools:
#include <geomtools/geomtools_config.h>
#include <geomtools/gnuplot_draw.h>
#if GEOMTOOLS_WITH_GNUPLOT_DISPLAY == 1
#include <geomtools/gnuplot_i.h>
#include <geomtools/gnuplot_drawer.h>
#endif // GEOMTOOLS_WITH_GNUPLOT_DISPLAY

// Falaise:
#include <falaise/falaise.h>

// Boost :
#include <boost/program_options.hpp>

// This project :
#include <snemo/asb/tracker_signal_generator_driver.h>
#include <snemo/asb/utils.h>

struct params_type
{
  datatools::logger::priority logging = datatools::logger::PRIO_DEBUG;
  bool draw = false;
  int number_of_events = -1;
  std::string input_filename = "";
};

void test_tsgd_1();
void test_tsgd_2();
void test_tsgd_3(const params_type &);


int main( int  argc_ , char **argv_  )
{
  falaise::initialize(argc_, argv_);
  datatools::logger::priority logging = datatools::logger::PRIO_DEBUG;
  int error_code = EXIT_SUCCESS;
  try {
    std::clog << "Test program for class 'snemo::asb::tracker_signal_generator_driver'!" << std::endl;

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

    test_tsgd_1();
    test_tsgd_2();
    test_tsgd_3(params);

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

void test_tsgd_1()
{
  std::clog << "[info] test_tsgd_1..." << std::endl;

  snemo::asb::tracker_signal_generator_driver TSGD;
  TSGD.set_logging_priority(datatools::logger::PRIO_DEBUG);
  TSGD.set_id("TrackerSignal");
  TSGD.set_hit_category("gg");
  TSGD.set_signal_category("sigtracker");
  TSGD.set_start_signal_id(0);
  TSGD.set_model(snemo::asb::tracker_signal_generator_driver::MODEL_TRIANGLE_GATE);
  // TSGD.set_rise_time(8 * CLHEP::ns);
  // TSGD.set_fall_time(70 * CLHEP::ns);
  // TSGD.set_energy_amplitude_factor(0.3 * CLHEP::volt / CLHEP::MeV);
  TSGD.initialize_simple();
  TSGD.tree_dump(std::clog, "My tracker signal generator driver", "[info] ");

  std::clog << std::endl;
  return;
}

void test_tsgd_2()
{
  std::clog << "[info] test_tsgd_2..." << std::endl;

  std::string tracker_signal_generator_config_filename
    = "${FALAISE_ASB_TESTING_DIR}/config/tracker_signal_generator.conf";
  datatools::fetch_path_with_env(tracker_signal_generator_config_filename);
  datatools::properties tracker_signal_generator_config;
  tracker_signal_generator_config.read_configuration(tracker_signal_generator_config_filename);
  tracker_signal_generator_config.tree_dump(std::clog, "Tracker signal generator driver params: ", "[info] ");

  snemo::asb::tracker_signal_generator_driver TSGD;
  TSGD.initialize(tracker_signal_generator_config);
  TSGD.tree_dump(std::clog, "My tracker signal generator driver: ", "[info] ");

  std::clog << std::endl;
  return;
}

void test_tsgd_3(const params_type & params_)
{
  std::clog << "[info] test_tsgd_3..." << std::endl;

  std::string SD_bank_label   = "SD";  // Simulated Data "SD" bank label :
  std::string SSD_bank_label  = "SSD"; // Simulated Signal Data "SSD" bank label :
  std::string hit_category    = "gg";
  std::string signal_category = "sigtracker";

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

  mctools::signal::signal_shape_builder ssb1;
  ssb1.set_logging_priority(datatools::logger::PRIO_DEBUG);
  ssb1.set_category(signal_category);
  ssb1.add_registered_shape_type_id("mctools::signal::triangle_signal_shape");
  ssb1.add_registered_shape_type_id("mctools::signal::triangle_gate_signal_shape");
  ssb1.add_registered_shape_type_id("mctools::signal::multi_signal_shape");
  ssb1.add_registered_shape_type_id("mygsl::linear_combination_function");
  ssb1.initialize_simple();
  // ssb1.tree_dump(std::clog, "Signal shape builder 1", "[info] ");

  // Tracker signal driver :
  std::string tracker_signal_generator_config_filename
    = "${FALAISE_ASB_TESTING_DIR}/config/tracker_signal_generator.conf";
  datatools::fetch_path_with_env(tracker_signal_generator_config_filename);
  datatools::properties tracker_signal_generator_config;
  tracker_signal_generator_config.read_configuration(tracker_signal_generator_config_filename);
  tracker_signal_generator_config.tree_dump(std::clog, "Tracker signal generator driver params: ", "[info] ");

  snemo::asb::tracker_signal_generator_driver TSGD;
  TSGD.set_geo_manager(my_manager);
  // TSGD.set_logging_priority(params_.logging)
  TSGD.initialize(tracker_signal_generator_config);
  TSGD.tree_dump(std::clog, "My tracker signal generator driver: ", "[info] ");

  std::string SD_filename = "";
  if (params_.input_filename.empty()) SD_filename = "${FALAISE_ASB_TESTING_DIR}/data/Se82_0nubb-source_strips_bulk_SD_10_events.brio";
  else SD_filename = params_.input_filename;
  datatools::fetch_path_with_env(SD_filename);

  // Event reader :
  dpp::input_module reader;
  datatools::properties reader_config;
  reader_config.store("logging.priority", "notice");
  reader_config.store("max_record_total", params_.number_of_events);
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
        TSGD.process(SD, SSD);
        if (SSD.has_signals(signal_category))
	  {
	    std::clog << "[info] SSD size = " << SSD.get_number_of_signals(signal_category) << std::endl;
	    SSD.tree_dump(std::clog, "SSD");

	    for (unsigned int isignal = 0; isignal < SSD.get_number_of_signals(signal_category); isignal++) {
	      const mctools::signal::base_signal & my_signal = SSD.get_signal(signal_category, isignal);
	      datatools::properties my_signal_shape_properties;
	      my_signal.get_auxiliaries().export_and_rename_starting_with(my_signal_shape_properties,
									  mctools::signal::base_signal::shape_parameter_prefix(),
									  "");
	      snemo::asb::build_shape(ssb1, my_signal);
	    } // end of isignal

	    datatools::temp_file tmp_file;
	    tmp_file.set_remove_at_destroy(false);
	    tmp_file.create("/tmp", "test_drivers_");
	    // Generate sampled shapes associated to signals:
	    std::clog << "[info] Temp file : '" << tmp_file.get_filename() << "'" << std::endl;

	    datatools::temp_file tmp_deriv_file;
	    tmp_deriv_file.set_remove_at_destroy(false);
	    tmp_deriv_file.create("/tmp", "test_drivers_deriv");
	    // Generate sampled shapes associated to signals:
	    std::clog << "[info] Temp derivative file : '" << tmp_deriv_file.get_filename() << "'" << std::endl;

	    // Build the full list of all functors to construct signal shapes
	    std::set<std::string> fkeys;
	    ssb1.build_list_of_functors(fkeys);
	    // ssb1.tree_dump(std::clog);

	    // Fill the temp file:
	    for (const auto & fkey : fkeys) {
	      const std::string & signal_name = fkey;
	      if (snemo::asb::is_private_signal_name(signal_name)) continue;
	      // std::clog << "[info] Computing signal shape '" << signal_name << "' for further plot..." << std::endl;
	      tmp_file.out() << "#@shape=" << fkey << ":\n";
	      ssb1.get_functor(fkey).write_ascii_with_units(tmp_file.out(),
							    -0.0 * CLHEP::microsecond,
							    +100.0 * CLHEP::microsecond,
							    1500,
							    CLHEP::microsecond,
							    CLHEP::volt,
							    16, 16
							    );
	      tmp_file.out() << "\n\n" << std::flush;
	    }

	    if (params_.draw) {
#if GEOMTOOLS_WITH_GNUPLOT_DISPLAY == 1
	      Gnuplot g1;
	      g1.cmd("set title 'Test snemo::asb' ");
	      g1.cmd("set key out");
	      g1.cmd("set grid");
	      g1.cmd("set xrange [-20:120]");
	      {
		std::ostringstream cmd1;
		cmd1 << "volt=" << CLHEP::volt;
		g1.cmd(cmd1.str());
		std::ostringstream cmd2;
		cmd2 << "nanosecond=" << CLHEP::nanosecond;
		g1.cmd(cmd2.str());
		// std::ostringstream cmd3;
		// cmd3 << "nVs=nanosecond*volt";
		// g1.cmd(cmd3.str());
	      }
	      g1.cmd("set yrange [-0.5:+0.1]");
	      g1.set_xlabel("time (us)").set_ylabel("Signal (V)");

	      {
		std::size_t fcount = 0;
		std::ostringstream plot_cmd;
		plot_cmd << "plot ";
		for (const auto & fkey : fkeys) {
		  if (snemo::asb::is_private_signal_name(fkey)) continue;
		  if (fcount > 0) { // set fcount < 3 to see only : 1 anodic signal and 2 cathodic signals
		    plot_cmd << ',';
		  }
		  plot_cmd << "  '" << tmp_file.get_filename() << "' "
			   << " index " << fcount << " using (column(1)"
			   << "):(column(2)"<< ')'
			   << " title 'Signal shape " << fkey << "' with lines lw 3"
		    ;

		  fcount++;
		}

		g1.cmd(plot_cmd.str());
		g1.showonscreen(); // window output
		geomtools::gnuplot_drawer::wait_for_key();
		usleep(200);
	      }

#endif // GEOMTOOLS_WITH_GNUPLOT_DISPLAY == 1
	    } // end if (draw)

	    ssb1.clear_functors();
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




// end
