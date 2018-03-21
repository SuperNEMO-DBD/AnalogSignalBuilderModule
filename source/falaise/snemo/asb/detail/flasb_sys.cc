// falaise/detail/falaise_sys.cc - Implementation of Falaise system singleton
//

// Ourselves:
#include <snemo/asb/detail/flasb_sys.h>

// Standard library
#include <cstdlib>
#include <memory>
#include <string>

// Third party:
#include <datatools/kernel.h>
#include <datatools/library_info.h>
#include <datatools/urn_db_service.h>
#include <datatools/urn_to_path_resolver_service.h>

// This project;
#include "falaise/resource.h"
#include "falaise/version.h"
#include "snemo/asb/resource.h"
#include "snemo/asb/version.h"

namespace snemo {

  namespace asb {

    namespace detail {

      // static
      const std::string &flasb_sys::flasb_setup_db_name()
      {
        static const std::string _n("flASBSetupDb");
        return _n;
      }

      // static
      const std::string &flasb_sys::flasb_resource_resolver_name()
      {
        static const std::string _n("flASBResourceResolver");
        return _n;
      }

      datatools::logger::priority flasb_sys::process_logging_env()
      {
        datatools::logger::priority logging = datatools::logger::PRIO_FATAL;
        char *l = getenv("FLASB_SYS_LOGGING");
        if (l) {
          std::string level_label(l);
          ::datatools::logger::priority prio = ::datatools::logger::get_priority(level_label);
          if (prio != ::datatools::logger::PRIO_UNDEFINED) {
            logging = prio;
          }
        }
        return logging;
      }

      // static
      flasb_sys *flasb_sys::_instance_ = nullptr;

      flasb_sys::flasb_sys()
      {
        _logging_ = flasb_sys::process_logging_env();
        if (_logging_ == ::datatools::logger::PRIO_UNDEFINED) {
          DT_LOG_WARNING(::datatools::logger::PRIO_WARNING, "Ignoring invalid FLASB_SYS_LOGGING=\""
                         << getenv("FLASB_SYS_LOGGING")
                         << "\" environment!");
        }
        DT_LOG_TRACE_ENTERING(_logging_);
        DT_THROW_IF(flasb_sys::_instance_ != nullptr, std::logic_error,
                    "Falaise system singleton is already set!");
        flasb_sys::_instance_ = this;
        DT_LOG_TRACE_EXITING(_logging_);
        return;
      }

      flasb_sys::~flasb_sys()
      {
        DT_LOG_TRACE_ENTERING(_logging_);
        if (is_initialized()) {
          shutdown();
        }
        flasb_sys::_instance_ = nullptr;
        DT_LOG_TRACE_EXITING(_logging_);
        return;
      }

      datatools::logger::priority flasb_sys::get_logging() const
      {
        return _logging_;
      }

      bool flasb_sys::is_initialized() const
      {
        return _initialized_;
      }

      void flasb_sys::initialize()
      {
        DT_LOG_TRACE_ENTERING(_logging_);
        DT_THROW_IF(is_initialized(), std::logic_error,
                    "Falaise system singleton is already initialized!");

        // Register library informations in the Bayeux/datatools' kernel:
        _libinfo_registration_();

        // Setup services:
        DT_LOG_TRACE(_logging_, "Falaise system singleton services...");
        _services_.set_name("flservices");
        _services_.set_description("Falaise System Singleton Services");
        _services_.set_allow_dynamic_services(true);
        _services_.initialize();

        _initialize_urn_services_();

        _initialized_ = true;
        DT_LOG_TRACE_EXITING(_logging_);
        return;
      }

      void flasb_sys::shutdown()
      {
        DT_LOG_TRACE_ENTERING(_logging_);
        DT_THROW_IF(!is_initialized(), std::logic_error, "Falaise system singleton is not initialized!");
        _initialized_ = false;

        // Terminate services:
        if (_services_.is_initialized()) {
          _shutdown_urn_services_();

          DT_LOG_TRACE(_logging_, "Terminating Falaise system singleton services...");
          _services_.reset();
        }

        // Deregister library informations from the Bayeux/datatools' kernel:
        _libinfo_deregistration_();

        DT_LOG_TRACE_EXITING(_logging_);
        return;
      }

      datatools::service_manager &flasb_sys::grab_services()
      {
        return _services_;
      }

      const datatools::service_manager &flasb_sys::get_services() const
      {
        return _services_;
      }

      // static
      bool flasb_sys::is_instantiated()
      {
        return _instance_ != nullptr;
      }

      // static
      flasb_sys &flasb_sys::instance()
      {
        return *_instance_;
      }

      // static
      const flasb_sys &flasb_sys::const_instance()
      {
        return *_instance_;
      }

      // static
      flasb_sys &flasb_sys::instantiate()
      {
        if (!flasb_sys::is_instantiated()) {
          static std::unique_ptr<flasb_sys> _flsys_handler;
          if (!_flsys_handler) {
            // Allocate the Falaise sys library singleton and initialize it:
            _flsys_handler.reset(new flasb_sys);
          }
        }
        return flasb_sys::instance();
      }

      void flasb_sys::_libinfo_registration_()
      {
        DT_LOG_TRACE_ENTERING(_logging_);

        DT_THROW_IF(!datatools::kernel::is_instantiated(), std::runtime_error,
                    "The Bayeux/datatools' kernel is not instantiated !");
        datatools::kernel &krnl = datatools::kernel::instance();

        // Populate the library info register, basically dumb if we don't
        // have it so assume it exists and hope for an exception if
        // it doesn't
        datatools::library_info &lib_info_reg = krnl.grab_library_info_register();

        // Bundled submodules:
        {
          DT_LOG_TRACE(_logging_, "Registration of ASB Falaise plugin library in the Bayeux/datatools' kernel...");
          // Falaise ASB itself:
          DT_THROW_IF(lib_info_reg.has("flasb"), std::logic_error, "ASB Falaise plugin is already registered !");
          datatools::properties &flasb_lib_infos = lib_info_reg.registration(
                                                                               "flasb",
                                                                               "ASB Falaise module operates the processing of SD data and,"
                                                                               "generates...",
                                                                               snemo::asb::version::get_version());

          // Register the Falaise resource path in the datatools' kernel:
          flasb_lib_infos.store_string(datatools::library_info::keys::install_resource_dir(),
                                         falaise::get_resource_dir());

          // If the 'FALAISE_RESOURCE_DIR' environment variable is set, it will supersede
          // the official registered resource path above through the 'datatools::fetch_path_with_env'
          // function:
          flasb_lib_infos.store_string(datatools::library_info::keys::env_resource_dir(),
                                         "FLASB_RESOURCE_DIR");

          // Register the Falaise plugin path in the datatools' kernel:
          flasb_lib_infos.store_string(datatools::library_info::keys::install_plugin_lib_dir(),
                                         falaise::get_plugin_dir());

          // If the 'FALAISE_PLUGIN_LIB_DIR' environment variable is set, it will supersed
          // the official registered plugin path above through the 'datatools::library_loader'
          // function:
          flasb_lib_infos.store_string(datatools::library_info::keys::env_plugin_lib_dir(),
                                         "FLASB_PLUGIN_LIB_DIR");
          DT_LOG_TRACE(_logging_, "ASB Falaise plugin library has been registered in the Bayeux/datatools' kernel.");
        }

        DT_LOG_TRACE_EXITING(_logging_);
        return;
      }

      void flasb_sys::_libinfo_deregistration_()
      {
        DT_LOG_TRACE_ENTERING(_logging_);

        if (datatools::kernel::is_instantiated()) {
          datatools::kernel &krnl = datatools::kernel::instance();
          if (krnl.has_library_info_register()) {
            // Access to the datatools kernel library info register:
            datatools::library_info &lib_info_reg = krnl.grab_library_info_register();

            // Unregistration of all registered submodules from the kernel's
            // library info register:
            if (lib_info_reg.has("flasb")) {
              DT_LOG_TRACE(_logging_,
                           "Deregistration of the ASB Falaise plugin library from the Bayeux/datatools' kernel...");
              lib_info_reg.unregistration("flasb");
              DT_LOG_TRACE(_logging_,
                           "ASB Falaise plugin library has been deregistered from the Bayeux/datatools' kernel.");
            }
          }
        }

        DT_LOG_TRACE_EXITING(_logging_);
        return;
      }

      void flasb_sys::_initialize_urn_services_()
      {
        DT_LOG_TRACE_ENTERING(_logging_);

        // Activate an URN info DB service:
        {
          datatools::urn_db_service &urnSetupDb = dynamic_cast<datatools::urn_db_service &>(
                                                                                            _services_.load_no_init(flasb_setup_db_name(), "datatools::urn_db_service"));
          urnSetupDb.set_logging_priority(_logging_);
          std::string urn_db_conf_file = "@flasb:urn/db/snemo_asb_setup_db.conf";
          datatools::fetch_path_with_env(urn_db_conf_file);
          datatools::properties urn_db_conf;
          urn_db_conf.read_configuration(urn_db_conf_file);
          urnSetupDb.initialize_standalone(urn_db_conf);
          if (datatools::logger::is_debug(_logging_)) {
            urnSetupDb.tree_dump(std::cerr, urnSetupDb.get_name() + ": ", "[debug] ");
          }
          DT_LOG_TRACE(_logging_, "Publishing the URN info DB '"
                       << urnSetupDb.get_name() << "' to the Bayeux/datatools' kernel...");
          urnSetupDb.kernel_push();
          DT_LOG_TRACE(_logging_, "URN info DB has been plugged in the Bayeux/datatools' kernel.");
        }

        // Activate an URN resolver service:
        {
          datatools::urn_to_path_resolver_service &urnResourceResolver =
            dynamic_cast<datatools::urn_to_path_resolver_service &>(_services_.load_no_init(
                                                                                            flasb_resource_resolver_name(), "datatools::urn_to_path_resolver_service"));
          urnResourceResolver.set_logging_priority(_logging_);
          std::string urn_resolver_conf_file = "@flasb:urn/resolvers/snemo_asb_resource_path_resolver.conf";
          datatools::fetch_path_with_env(urn_resolver_conf_file);
          datatools::properties urn_resolver_conf;
          urn_resolver_conf.read_configuration(urn_resolver_conf_file);
          urnResourceResolver.initialize_standalone(urn_resolver_conf);
          if (datatools::logger::is_debug(_logging_)) {
            urnResourceResolver.tree_dump(std::cerr, urnResourceResolver.get_name() + ": ", "[debug] ");
          }
          DT_LOG_TRACE(_logging_, "Publishing the URN path resolver '"
                       << urnResourceResolver.get_name()
                       << "' to the Bayeux/datatools' kernel...");
          urnResourceResolver.kernel_push();
          DT_LOG_TRACE(_logging_, "URN path resolver has been plugged in the Bayeux/datatools' kernel.");
        }

        DT_LOG_TRACE_EXITING(_logging_);
        return;
      }

      void flasb_sys::_shutdown_urn_services_()
      {
        DT_LOG_TRACE_ENTERING(_logging_);

        // DeActivate the URN resolver:
        {
          DT_LOG_TRACE(_logging_, "Accessing URN path resolver...");
          datatools::urn_to_path_resolver_service &urnResourceResolver =
            _services_.grab<datatools::urn_to_path_resolver_service &>(flasb_resource_resolver_name());
          DT_LOG_TRACE(_logging_, "Removing URN path resolver '"
                       << urnResourceResolver.get_name()
                       << "' from the  Bayeux/datatools's kernel...");
          urnResourceResolver.kernel_pop();
          DT_LOG_TRACE(_logging_,
                       "URN path resolver has been removed from the  Bayeux/datatools kernel.");
          urnResourceResolver.reset();
        }

        // DeActivate the URN info setup DB:
        {
          DT_LOG_TRACE(_logging_, "Accessing URN info setup DB...");
          datatools::urn_db_service &urnSetupDb =
            _services_.grab<datatools::urn_db_service &>(flasb_setup_db_name());
          DT_LOG_TRACE(_logging_, "Removing URN info setup DB '"
                       << urnSetupDb.get_name()
                       << "' from the  Bayeux/datatools's kernel...");
          urnSetupDb.kernel_pop();
          DT_LOG_TRACE(_logging_,
                       "URN info setup DB has been removed from the  Bayeux/datatools kernel.");
          urnSetupDb.reset();
        }

        DT_LOG_TRACE_EXITING(_logging_);
        return;
      }

    } // end of namespace detail

  } // namespace asb

} // namespace snemo
