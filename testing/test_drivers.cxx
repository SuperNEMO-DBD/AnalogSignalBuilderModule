// test_drivers.cxx

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
#include <mctools/signal/utils.h>

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

// Boost :
#include <boost/program_options.hpp>

// Falaise:
#include <falaise/falaise.h>

// This project :
#include <snemo/asb/calo_signal_generator_driver.h>
#include <snemo/asb/tracker_signal_generator_driver.h>
#include <snemo/asb/utils.h>
// analog_signal_builder_module.h>

struct params_type
{
  datatools::logger::priority logging = datatools::logger::PRIO_DEBUG;
  bool draw = false;
  int number_of_events = -1;
  std::string input_filename = "";
};

void test_drivers_1(const params_type &);
void test_drivers_2(const params_type &);

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
    test_drivers_1(params);

    std::clog << "Next test..." << std::endl;
    test_drivers_2(params);

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

void test_drivers_1(const params_type & params_)
{
  std::clog << "[info] test_drivers_1..." << std::endl;

  std::string SD_bank_label   = "SD";  // Simulated Data "SD" bank label :
  std::string hit_category    = "calo";
  std::string signal_category = "sigcalo";

  mctools::signal::signal_shape_builder ssb1;
  ssb1.set_logging_priority(datatools::logger::PRIO_DEBUG);
  ssb1.set_category(signal_category);
  ssb1.add_registered_shape_type_id("mctools::signal::triangle_signal_shape");
  ssb1.add_registered_shape_type_id("mctools::signal::triangle_gate_signal_shape");
  ssb1.add_registered_shape_type_id("mctools::signal::multi_signal_shape");
  ssb1.add_registered_shape_type_id("mygsl::linear_combination_function");
  ssb1.initialize_simple();
  ssb1.tree_dump(std::clog, "Signal shape builder 1", "[info] ");

  snemo::asb::calo_signal_generator_driver CSGD1;
  CSGD1.set_logging_priority(datatools::logger::PRIO_DEBUG);
  CSGD1.set_id("CaloSignal");
  CSGD1.set_hit_category(hit_category);
  CSGD1.set_signal_category(signal_category);
  CSGD1.set_model(snemo::asb::calo_signal_generator_driver::MODEL_TRIANGLE);
  CSGD1.set_rise_time(8 * CLHEP::ns);
  CSGD1.set_fall_time(70 * CLHEP::ns);
  CSGD1.set_energy_amplitude_factor(0.3 * CLHEP::volt / CLHEP::MeV);
  CSGD1.initialize_simple();
  CSGD1.tree_dump(std::clog, "Calo signal generator driver", "[info] ");

  std::string SD_filename = "";
  if (params_.input_filename.empty()) SD_filename = "${FALAISE_ASB_TESTING_DIR}/data/Se82_0nubb-source_strips_bulk_SD_10_events.brio";
  else SD_filename = params_.input_filename;
  datatools::fetch_path_with_env(SD_filename);

  // Event reader :
  dpp::input_module reader;
  reader.set_name("InputSD");
  datatools::properties reader_config;
  reader_config.store("logging.priority", "notice");
  reader_config.store("max_record_total", params_.number_of_events);
  reader_config.store("files.mode", "single");
  reader_config.store("files.single.filename", SD_filename);
  reader.initialize_standalone(reader_config);

  // Event record :
  datatools::things ER;

  std::clog << "\n[info] Event loop:" << std::endl;
  std::size_t psd_count = 0; // Event counter
  while (!reader.is_terminated()) {
    std::clog << "\n[info] **************************" << std::endl;
    std::clog << "[info] * Event Record #" << psd_count << std::endl;
    std::clog << "[info] **************************" << std::endl;
    std::clog << "[info] Processing a new event record..." << std::endl;
    reader.process(ER);

    // A plain 'mctools::simulated_data' object is stored here :
    if (ER.has(SD_bank_label) && ER.is_a<mctools::simulated_data>(SD_bank_label)) {

      // Access to the input "SD" bank with a stored 'mctools::simulated_data' :
      const mctools::simulated_data & SD = ER.get<mctools::simulated_data>(SD_bank_label);
      if (SD.has_step_hits(hit_category)) {

        // Output data model:
        mctools::signal::signal_data SSD;
        std::clog << "[info] Found " << SD.get_number_of_step_hits(hit_category) << " '" << hit_category << "' hits..." << std::endl;
        CSGD1.process(SD, SSD);
        if (SSD.has_signals(signal_category)) std::clog << "[info] SSD size = " << SSD.get_number_of_signals(signal_category) << std::endl;
        std::vector<std::string> sigcats;
        SSD.build_list_of_categories(sigcats);
        std::clog << "[info] List of embedded signal categories:" << std::endl;
        for (const auto & sigcat : sigcats) {
          std::clog << "[info]  * SSD's signal category : '" << sigcat << "'" << std::endl;
        }
        if (SSD.has_signals(signal_category)) {

          for (unsigned int isignal = 0;
               isignal < SSD.get_number_of_signals(signal_category);
               isignal++) {
            std::clog << "[info] SSD signal #" << isignal << "..." << std::endl;
            const mctools::signal::base_signal & my_signal = SSD.get_signal(signal_category, isignal);

            datatools::properties my_signal_shape_properties;
            my_signal.get_auxiliaries().export_and_rename_starting_with(my_signal_shape_properties,
                                                                        mctools::signal::base_signal::shape_parameter_prefix(),
                                                                        "");
	    mctools::signal::build_shape(ssb1, my_signal);
          }

	  ssb1.tree_dump(std::clog, "Signal shape builder 1", "[info] ");

          datatools::temp_file tmp_file;
          tmp_file.set_remove_at_destroy(false);
          tmp_file.create("/tmp", "test_drivers_");
          // Generate sampled shapes associated to signals:
          std::clog << "[info] Temp file : '" << tmp_file.get_filename() << "'" << std::endl;

          std::set<std::string> fkeys;
          ssb1.build_list_of_functors(fkeys);
          // Fill the temp file:
          for (const auto & fkey : fkeys) {
            const std::string & signal_name = fkey;
            if (snemo::asb::is_private_signal_name(signal_name)) continue;
            // std::clog << "[info] Computing signal shape '" << signal_name << "' for further plot..." << std::endl;
            tmp_file.out() << "#@shape=" << fkey << ":\n";
            ssb1.get_functor(fkey).write_ascii_with_units(tmp_file.out(),
                                                          -0.0 * CLHEP::nanosecond,
                                                          +400.0 * CLHEP::nanosecond,
                                                          1024,
                                                          CLHEP::ns,
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
            g1.cmd("set xrange [-20:400]");
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
            g1.cmd("set yrange [-1:+0.1]");
            g1.set_xlabel("time (ns)").set_ylabel("Signal (V)");

            {
              std::size_t fcount = 0;
              std::ostringstream plot_cmd;
              plot_cmd << "plot ";
              for (const auto & fkey : fkeys) {
                if (snemo::asb::is_private_signal_name(fkey)) continue;
                if (fcount > 0) {
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

          // Remove functors from the signal shap builder:
          ssb1.clear_functors();
        } // if (SSD.has_signals(signal_category))
      } // if (SD.has_step_hits(hit_category))
    } // end if (ER.has(SD_bank_label)...)

    ER.clear();
    DT_LOG_NOTICE(params_.logging, "Simulated data #" << psd_count);
    psd_count++;
    if (psd_count > params_.number_of_events) break;
  } // end of reader

  std::clog << std::endl;
  return;
}


void test_drivers_2(const params_type & params_)
{
  std::clog << "[info] test_drivers_2..." << std::endl;

  std::string SD_bank_label   = "SD";  // Simulated Data "SD" bank label :
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
  ssb1.tree_dump(std::clog, "Signal shape builder 1", "[info] ");

  snemo::asb::tracker_signal_generator_driver TSGD1;
  TSGD1.set_logging_priority(datatools::logger::PRIO_DEBUG);
  TSGD1.set_geo_manager(my_manager);
  TSGD1.set_id("TrackerSignal");
  TSGD1.set_hit_category(hit_category);
  TSGD1.set_signal_category(signal_category);
  TSGD1.set_model(snemo::asb::tracker_signal_generator_driver::MODEL_TRIANGLE_GATE);
  TSGD1.set_rise_time(1 * CLHEP::microsecond);
  TSGD1.set_fall_time(1 * CLHEP::microsecond);
  TSGD1.set_amplitude(0.1 * CLHEP::volt);
  TSGD1.initialize_simple();
  TSGD1.tree_dump(std::clog, "Tracker signal generator driver", "[info] ");

  std::string SD_filename = "";
  if (params_.input_filename.empty()) SD_filename = "${FALAISE_ASB_TESTING_DIR}/data/Se82_0nubb-source_strips_bulk_SD_10_events.brio";
  else SD_filename = params_.input_filename;
  datatools::fetch_path_with_env(SD_filename);

  // Event reader :
  dpp::input_module reader;
  reader.set_name("InputSD");
  datatools::properties reader_config;
  reader_config.store("logging.priority", "notice");
  reader_config.store("max_record_total", params_.number_of_events);
  reader_config.store("files.mode", "single");
  reader_config.store("files.single.filename", SD_filename);
  reader.initialize_standalone(reader_config);

  // Event record :
  datatools::things ER;

  std::clog << "\n[info] Event loop:" << std::endl;
  std::size_t psd_count = 0; // Event counter
  while (!reader.is_terminated()) {
    std::clog << "\n[info] **************************" << std::endl;
    std::clog << "[info] * Event Record #" << psd_count << std::endl;
    std::clog << "[info] **************************" << std::endl;
    std::clog << "[info] Processing a new event record..." << std::endl;
    reader.process(ER);

    // A plain 'mctools::simulated_data' object is stored here :
    if (ER.has(SD_bank_label) && ER.is_a<mctools::simulated_data>(SD_bank_label)) {

      // Access to the input "SD" bank with a stored 'mctools::simulated_data' :
      const mctools::simulated_data & SD = ER.get<mctools::simulated_data>(SD_bank_label);
      if (SD.has_step_hits(hit_category)) {

        // Output data model:
        mctools::signal::signal_data SSD;
        std::clog << "[info] Found " << SD.get_number_of_step_hits(hit_category) << " '" << hit_category << "' hits..." << std::endl;
        TSGD1.process(SD, SSD);
	if (SSD.has_signals(signal_category)) std::clog << "[info] SSD size = " << SSD.get_number_of_signals(signal_category) << std::endl;
        std::vector<std::string> sigcats;
        SSD.build_list_of_categories(sigcats);
        std::clog << "[info] List of embedded signal categories:" << std::endl;
        for (const auto & sigcat : sigcats) {
          std::clog << "[info]  * SSD's signal category : '" << sigcat << "'" << std::endl;
	  SSD.tree_dump(std::clog, "Signal Data");
        }
        if (SSD.has_signals(signal_category)) {

          for (unsigned int isignal = 0;
               isignal < SSD.get_number_of_signals(signal_category);
               isignal++) {
            // std::clog << "[info] SSD signal #" << isignal << "..." << std::endl;
            const mctools::signal::base_signal & my_signal = SSD.get_signal(signal_category, isignal);

            datatools::properties my_signal_shape_properties;
            my_signal.get_auxiliaries().export_and_rename_starting_with(my_signal_shape_properties,
                                                                        mctools::signal::base_signal::shape_parameter_prefix(),
                                                                        "");
	    mctools::signal::build_shape(ssb1, my_signal);

	  } // end of isignal

          datatools::temp_file tmp_file;
          tmp_file.set_remove_at_destroy(false);
          tmp_file.create("/tmp", "test_drivers_");
          // Generate sampled shapes associated to signals:
          std::clog << "[info] Temp file : '" << tmp_file.get_filename() << "'" << std::endl;

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
                if (fcount > 0) {
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

          // Remove functors from the signal shap builder:
          ssb1.clear_functors();
        } // if (SSD.has_signals(signal_category))
      } // if (SD.has_step_hits(hit_category))
    } // end if (ER.has(SD_bank_label)...)

    ER.clear();
    DT_LOG_NOTICE(params_.logging, "Simulated data #" << psd_count);
    psd_count++;
    if (psd_count > params_.number_of_events) break;
  } // end of reader

  std::clog << std::endl;
  return;
}
