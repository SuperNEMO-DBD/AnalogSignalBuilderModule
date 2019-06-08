// test_flasb.cxx

// Standard libraries :
#include <iostream>
#include <string>
#include <cstdlib>

// - Bayeux/datatools:
#include <datatools/exception.h>
#include <datatools/logger.h>
#include <datatools/utils.h>

// Falaise :
#include <falaise/falaise.h>

// This project :
#include <snemo/asb/detail/flasb_sys.h>

void test1();
void test2();

int main( int  argc_ , char ** argv_  )
{
  std::clog << "Hello, ASB!" << std::endl;
  falaise::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;
  try {

    test1();
    test2();

  } catch (std::exception & error) {
    DT_LOG_FATAL(logging, error.what());
    error_code = EXIT_FAILURE;
  } catch (...) {
    DT_LOG_FATAL(logging, "Unexpected error!");
    error_code = EXIT_FAILURE;
  }
  falaise::terminate();
  std::clog << "Bye." << std::endl;
  return error_code;
}

void test1()
{
  std::clog << "test1: Test program for 'snemo::asb::flasb_sys' singleton!" << std::endl;
  if (!snemo::asb::detail::flasb_sys::is_instantiated()) {
    snemo::asb::detail::flasb_sys::instantiate();
  }
  {
    datatools::i_tree_dumpable::base_print_options popts;
    popts.title = "ASB Falaise plugin system singleton: ";
    popts.indent = "[debug] ";
    boost::property_tree::ptree ptopts;
    popts.export_to(ptopts);
    snemo::asb::detail::flasb_sys::instance().initialize();
    snemo::asb::detail::flasb_sys::instance().print_tree(std::clog, ptopts);
    snemo::asb::detail::flasb_sys::instance().shutdown();
  }
  std::clog << "test1: done" << std::endl << std::endl;
}

void test2()
{
  std::clog << "test2: Test program for 'snemo::asb::flasb_sys' singleton!" << std::endl;
  std::clog << "test2: done" << std::endl << std::endl;
}
