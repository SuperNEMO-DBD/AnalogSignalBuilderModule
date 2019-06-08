// test_calo_signal_generator_driver.cxx

// Standard libraries :
#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <cstdio>

// - Bayeux/datatools:
#include <datatools/utils.h>
#include <datatools/clhep_units.h>

// Falaise:
#include <falaise/falaise.h>

// This project :
#include <snemo/asb/calo_signal_generator_driver.h>

void test_csgd_1();
void test_csgd_2();

int main( int  argc_ , char **argv_  )
{
  falaise::initialize(argc_, argv_);
  datatools::logger::priority logging = datatools::logger::PRIO_DEBUG;
  int error_code = EXIT_SUCCESS;
  try {
    std::clog << "Test program for class 'snemo::asb::calo_signal_generator_driver'!" << std::endl;

    test_csgd_1();
    test_csgd_2();

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

void test_csgd_1()
{
  std::clog << "[info] test_csgd_1..." << std::endl;

  snemo::asb::calo_signal_generator_driver CSGD;
  CSGD.set_logging_priority(datatools::logger::PRIO_DEBUG);
  CSGD.set_id("CaloSignal");
  CSGD.set_hit_category("calo");
  CSGD.set_signal_category("sigcalo");
  CSGD.set_start_signal_id(0);
  CSGD.set_model(snemo::asb::calo_signal_generator_driver::MODEL_TRIANGLE);
  CSGD.set_rise_time(8 * CLHEP::ns);
  CSGD.set_fall_time(70 * CLHEP::ns);
  CSGD.set_energy_amplitude_factor(0.3 * CLHEP::volt / CLHEP::MeV);
  CSGD.initialize_simple();
  CSGD.tree_dump(std::clog, "My calo signal generator driver", "[info] ");

  std::clog << std::endl;
  return;
}

void test_csgd_2()
{
  std::clog << "[info] test_csgd_2..." << std::endl;

  std::string calo_signal_generator_config_filename
    = "${FALAISE_ASB_TESTING_DIR}/config/calo_signal_generator.conf";
  datatools::fetch_path_with_env(calo_signal_generator_config_filename);
  datatools::properties calo_signal_generator_config;
  calo_signal_generator_config.read_configuration(calo_signal_generator_config_filename);
  calo_signal_generator_config.tree_dump(std::clog, "Calo signal generator driver params: ", "[info] ");

  snemo::asb::calo_signal_generator_driver CSGD;
  CSGD.initialize(calo_signal_generator_config);
  CSGD.tree_dump(std::clog, "My calo signal generator driver: ", "[info] ");

  std::clog << std::endl;
  return;
}

// end
