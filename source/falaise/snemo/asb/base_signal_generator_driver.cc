// base_signal_generator_driver.cc - Implementation of Falaise ASB plugin base_signal_generatord_drive class
//
// Copyright (c) 2016 F. Mauger <mauger@lpccaen.in2p3.fr>
// Copyright (c) 2016 G. Oliviéro <goliviero@lpccaen.in2p3.fr>
//
// This file is part of Falaise/ASB plugin.
//
// Falaise/ASB plugin is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Falaise/ASB plugin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Falaise/ASB plugin.  If not, see <http://www.gnu.org/licenses/>.

// Ourselves:
#include <snemo/asb/base_signal_generator_driver.h>

// Standard library:
#include <sstream>
#include <stdexcept>

// Third party:
// - Boost:
#include <boost/lexical_cast.hpp>
// - Bayeux/datatools :
#include <bayeux/datatools/properties.h>
#include <bayeux/datatools/exception.h>

// This project:
#include <snemo/asb/utils.h>

namespace snemo {

  namespace asb {

    DATATOOLS_FACTORY_SYSTEM_REGISTER_IMPLEMENTATION(base_signal_generator_driver,
                                                     "snemo::asb::base_signal_generator_driver/__system__")

    void base_signal_generator_driver::_set_defaults()
    {
      _logging_priority_ = datatools::logger::PRIO_FATAL;
      _id_.clear();
      _hit_category_.clear();
      _signal_category_.clear();
      _start_signal_id_ = -1;
      _geo_manager_ = nullptr;
      _running_signal_id_ = -1;
      return;
    }

    base_signal_generator_driver::base_signal_generator_driver(const std::string & id_)
    {
      _initialized_ = false;
      _set_defaults();
      set_id(id_);
      return;
    }

    base_signal_generator_driver::~base_signal_generator_driver()
    {
      // Should not be initialized here...
      DT_THROW_IF(is_initialized(), std::logic_error, "Missing explicit reset call in child class!");
      return;
    }

    datatools::logger::priority
    base_signal_generator_driver::get_logging_priority() const
    {
      return _logging_priority_;
    }

    void
    base_signal_generator_driver::set_logging_priority(datatools::logger::priority priority_)
    {
      _logging_priority_ = priority_;
      return;
    }

    bool base_signal_generator_driver::has_id() const
    {
      return !_id_.empty();
    }

    const std::string & base_signal_generator_driver::get_id() const
    {
      return _id_;
    }

    void
    base_signal_generator_driver::set_id(const std::string & id_)
    {
      _id_ = id_;
      return;
    }

    bool base_signal_generator_driver::has_hit_category() const
    {
      return !_hit_category_.empty();
    }

    const std::string & base_signal_generator_driver::get_hit_category() const
    {
      return _hit_category_;
    }

    void base_signal_generator_driver::set_hit_category(const std::string & category_)
    {
      _hit_category_ = category_;
      return;
    }

    bool base_signal_generator_driver::has_signal_category() const
    {
      return !_signal_category_.empty();
    }

    const std::string & base_signal_generator_driver::get_signal_category() const
    {
      return _signal_category_;
    }

    void base_signal_generator_driver::set_signal_category(const std::string & category_)
    {
      _signal_category_ = category_;
      return;
    }

    void base_signal_generator_driver::set_start_signal_id(const int sid_)
    {
      _start_signal_id_ = sid_;
      return;
    }

    int base_signal_generator_driver::get_start_signal_id() const
    {
      return _start_signal_id_;
    }

    int base_signal_generator_driver::get_running_signal_id() const
    {
      return _running_signal_id_;
    }

    bool base_signal_generator_driver::has_geo_manager() const
    {
      return _geo_manager_ != nullptr;
    }

    void base_signal_generator_driver::set_geo_manager(const geomtools::manager & mgr_ )
    {
      _geo_manager_ = & mgr_;
      return;
    }

    const geomtools::manager & base_signal_generator_driver::get_geo_manager() const
    {
      DT_THROW_IF(!has_geo_manager(), std::logic_error, "Missing geometry initialized !");
      return *_geo_manager_;
    }

    bool base_signal_generator_driver::is_initialized() const
    {
      return _initialized_;
    }

    void base_signal_generator_driver::_set_initialized_(bool i_)
    {
      _initialized_ = i_;
      return;
    }

    void base_signal_generator_driver::initialize_simple()
    {
      datatools::properties dummy_config;
      initialize(dummy_config);
      return;
    }

    void base_signal_generator_driver::initialize(const datatools::properties & config_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Already initialized !");

      // Logging priority:
      datatools::logger::priority lp = datatools::logger::extract_logging_configuration(config_, _logging_priority_);
      DT_THROW_IF(lp == datatools::logger::PRIO_UNDEFINED,
                  std::logic_error,
                  "Invalid logging priority level !");
      set_logging_priority(lp);

      // std::cerr << "DEVEL: id = " << _id_ << std::endl;
      if (_id_.empty()) {
        if (config_.has_key("id")) {
          set_id(config_.fetch_string("id"));
        }
      }
      // std::cerr << "DEVEL: id = " << _id_ << std::endl;
      DT_THROW_IF(_id_.empty(), std::logic_error, "Missing ID!");

      if (_hit_category_.empty()) {
        if (config_.has_key("hit_category")) {
          set_hit_category(config_.fetch_string("hit_category"));
        }
      }
      DT_THROW_IF(_hit_category_.empty(), std::logic_error, "Missing hit category!");

      if (_signal_category_.empty()) {
        if (config_.has_key("signal_category")) {
          set_signal_category(config_.fetch_string("signal_category"));
        }
      }
      DT_THROW_IF(_signal_category_.empty(), std::logic_error, "Missing signal category!");

      if (_start_signal_id_ == -1) {
        if (config_.has_key("start_signal_id")) {
          set_start_signal_id(config_.fetch_positive_integer("start_signal_id"));
        }
      }
      if (_start_signal_id_ == -1) {
        _start_signal_id_ = 0;
      }

      _initialize(config_);

      _running_signal_id_ = _start_signal_id_;

      _set_initialized_(true);
      return;
    }

    void base_signal_generator_driver::_increment_running_signal_id()
    {
      _running_signal_id_++;
      return;
    }

    void base_signal_generator_driver::reset()
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");
      _set_initialized_(false);
      _id_.clear();
      _hit_category_.clear();
      _signal_category_.clear();
      _start_signal_id_ = -1;
      _running_signal_id_ = -1;
      _geo_manager_ = nullptr;
      _reset();
      return;
    }

    void base_signal_generator_driver::build_multi_signal(mctools::signal::base_signal & a_multi_signal_,
							  const std::vector<mctools::signal::base_signal> & list_of_atomic_signal_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");


      datatools::properties multi_signal_config;
      std::vector<std::string> component_labels;


      for (unsigned int i = 0; i < list_of_atomic_signal_.size(); i++)
	{
	  const mctools::signal::base_signal atomic_signal = list_of_atomic_signal_[i];
	  const int comp_sig_id    = atomic_signal.get_hit_id();
	  std::string comp_sig_key = "sig" + boost::lexical_cast<std::string>(comp_sig_id);
	  component_labels.push_back(comp_sig_key);
	  std::string comp_sig_prefix = "components." + comp_sig_key + ".";
	  std::string comp_sig_label;
	  build_private_signal_name(comp_sig_id, comp_sig_label);

	  datatools::properties comp_shape_parameters;
	  atomic_signal.get_auxiliaries().export_and_rename_starting_with(comp_shape_parameters,
									  mctools::signal::base_signal::shape_parameter_prefix(),
									  "");
	  a_multi_signal_.add_private_shape(comp_sig_label,
					   atomic_signal.get_shape_type_id(),
					   comp_shape_parameters);

	  // Private shape
	  std::string key_key        = comp_sig_prefix + "key";
	  std::string time_shift_key = comp_sig_prefix + "time_shift";
	  std::string scaling_key    = comp_sig_prefix + "scaling";
	  multi_signal_config.store(key_key, comp_sig_label);
	  multi_signal_config.store_real_with_explicit_unit(time_shift_key, 0.0 * CLHEP::ns);
	  multi_signal_config.set_unit_symbol(time_shift_key, "ns");
	  multi_signal_config.store_real(scaling_key, 1.0);

	}

      multi_signal_config.store("components", component_labels);
      a_multi_signal_.set_shape_parameters(multi_signal_config);
      a_multi_signal_.initialize(multi_signal_config);


      return;
    }

    void base_signal_generator_driver::process(const mctools::simulated_data & sim_data_,
                                               mctools::signal::signal_data & sim_signal_data_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");
      _process(sim_data_, sim_signal_data_);
      return;
    }

    void base_signal_generator_driver::tree_dump(std::ostream & out_,
                                                 const std::string & title_,
                                                 const std::string & indent_,
                                                 bool inherit_) const
    {
      if (!title_.empty()) {
        out_ << indent_ << title_ << std::endl;
      }

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Logging : '"
           << datatools::logger::get_priority_label(_logging_priority_) << "'"
           << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "ID : '" << get_id() << "'"
           << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Hit category : '" << get_hit_category() << "'"
           << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Signal category : '" << get_signal_category() << "'"
           << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Start signal ID : " << get_start_signal_id()
           << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Running signal ID : " << get_running_signal_id()
           << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Geometry manager : ";
      if (has_geo_manager()) {
        out_ << "<yes>";
      } else {
        out_ << "<no>";
      }
      out_ << std::endl;

      _tree_dump(out_, indent_);

      out_ << indent_ << datatools::i_tree_dumpable::inherit_tag(inherit_)
           << "Initialized : " << std::boolalpha << is_initialized() << std::endl;

      return;
    }

  } // end of namespace asb

} // end of namespace snemo
