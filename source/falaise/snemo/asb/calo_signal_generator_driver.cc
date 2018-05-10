// calo_signal_generator_driver.cc
//
// Copyright (c) 2016-2017 F. Mauger <mauger@lpccaen.in2p3.fr>
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
#include <snemo/asb/calo_signal_generator_driver.h>

// Third party:
// - Boost:
#include <boost/lexical_cast.hpp>
// - Bayeux/geomtools:
#include <bayeux/geomtools/sensitive.h>
// - Bayeux/mctools:
#include <bayeux/mctools/signal/base_signal.h>
#include <bayeux/mctools/utils.h>

// This project:
#include <snemo/asb/utils.h>

namespace snemo {

  namespace asb {

    // Registration instantiation macro :
    SNEMO_ASB_SIGNAL_GENERATOR_DRIVER_REGISTRATION_IMPLEMENT(calo_signal_generator_driver, "snemo::asb::calo_signal_generator_driver")

    /*** Implementation of the interface ***/

    void calo_signal_generator_driver::_set_defaults_()
    {
      _model_ = MODEL_INVALID;
      _rise_time_ = 8 * CLHEP::ns;  // Rise time on calo signal (from Bordeaux wavecatcher signals)
      _fall_time_ = 70 * CLHEP::ns; // Fall time on calo signal (from Bordeaux wavecatcher signals)
      _energy_amplitude_factor_ = 0.3 * CLHEP::volt / CLHEP::MeV; // Energy to amplitude conversion factor
      return;
    }

    void calo_signal_generator_driver::set_model(const model_type model_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized and locked!");
      _model_ = model_;
      return;
    }

    calo_signal_generator_driver::model_type calo_signal_generator_driver::get_model() const
    {
      return _model_;
    }

    void calo_signal_generator_driver::set_rise_time(const double rise_time_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized and locked!");
      _rise_time_ = rise_time_;
      return;
    }

    double calo_signal_generator_driver::get_rise_time() const
    {
      return _rise_time_;
    }

    void calo_signal_generator_driver::set_fall_time(const double fall_time_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized and locked!");
      _fall_time_ = fall_time_;
      return;
    }

    double calo_signal_generator_driver::get_fall_time() const
    {
      return _fall_time_;
    }

    void calo_signal_generator_driver::set_energy_amplitude_factor(const double factor_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized and locked!");
      _energy_amplitude_factor_ = factor_;
      return;
    }

    double calo_signal_generator_driver::get_energy_amplitude_factor() const
    {
      return _energy_amplitude_factor_;
    }

    calo_signal_generator_driver::calo_signal_generator_driver(const std::string & id_)
      : base_signal_generator_driver(id_)
    {
      _model_ = MODEL_INVALID;
      return;
    }

    calo_signal_generator_driver::calo_signal_generator_driver(const model_type model_,
                                                               const std::string & id_)
      : base_signal_generator_driver(id_),
        _model_(model_)
    {
      return;
    }

    calo_signal_generator_driver::~calo_signal_generator_driver()
    {
      if (is_initialized()) {
        this->calo_signal_generator_driver::reset();
      }
      return;
    }

    void calo_signal_generator_driver::_initialize(const datatools::properties & config_)
    {
      if (_model_ == MODEL_INVALID) {
        if (config_.has_key("model")) {
          std::string model_label = config_.fetch_string("model");
          if (model_label == "triangle") {
            set_model(MODEL_TRIANGLE);
          } else {
            DT_THROW(std::logic_error,
                     "Unsupported driver model '" << model_label << "'!");
          }
        }
      }

      if (_model_ == MODEL_INVALID) {
        DT_THROW(std::logic_error, "Missing driver model!");
      }

      if (config_.has_key("rise_time")) {
        double rise_time = config_.fetch_real_with_explicit_dimension("rise_time", "time");
        _rise_time_ = rise_time;
      }

      if (config_.has_key("fall_time")) {
        double fall_time = config_.fetch_real_with_explicit_dimension("fall_time", "time");
        _fall_time_ = fall_time;
      }

      if (config_.has_key("energy_amplitude_factor")) {
        double e2a =  config_.fetch_dimensionless_real("energy_amplitude_factor");
        _energy_amplitude_factor_ = e2a * CLHEP::volt / CLHEP::MeV;
      }

      return;
    }

    void calo_signal_generator_driver::_reset()
    {
      // clear resources...

      _model_ = MODEL_INVALID;
      _set_defaults_();
      return;
    }

    double calo_signal_generator_driver::_convert_energy_to_amplitude(const double energy_)
    {
      double amplitude = _energy_amplitude_factor_ * energy_;
      return amplitude; // maybe units problem for the moment
    }

    void calo_signal_generator_driver::_process(const mctools::simulated_data & sim_data_,
                                                mctools::signal::signal_data & sim_signal_data_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Calo signal generator driver is not initialized !");
      DT_THROW_IF(_model_ == MODEL_INVALID, std::logic_error, "Calo signal generator driver model is invalid !");
      if (_model_ == MODEL_TRIANGLE) {
        _process_triangle_model_(sim_data_, sim_signal_data_);
      }
      return;
    }

    void calo_signal_generator_driver::_process_triangle_model_(const mctools::simulated_data & sim_data_,
                                                                mctools::signal::signal_data & sim_signal_data_)
    {
      std::string hit_label = get_hit_category();
      if (!sim_data_.has_step_hits(hit_label)) {
        // No hit in the proper category:
        return;
      }
      const size_t number_of_calo_hits = sim_data_.get_number_of_step_hits(hit_label);

      // Reference time of the event:
      double event_time_ref;
      datatools::invalidate(event_time_ref);
      // Set of GIDs associated to hits:
      std::set<geomtools::geom_id> set_of_gids;
      // Number of hits per GID:
      std::map<geomtools::geom_id, unsigned int> map_gid_hit_in_gid;

      // Search calo time reference for the event : we use the time of the first hit
      // as a relative reference time for all modelled signals:
      int last_hit_id = -1;
      for (size_t ihit = 0; ihit < number_of_calo_hits; ihit++) {
        const mctools::base_step_hit & calo_hit = sim_data_.get_step_hit(hit_label, ihit);
        const int hit_id = calo_hit.get_hit_id();
        const double start_signal_time = calo_hit.get_time_start();
        if (!datatools::is_valid(event_time_ref)) event_time_ref = start_signal_time;
        if (start_signal_time < event_time_ref) event_time_ref = start_signal_time;
        if (hit_id > last_hit_id) last_hit_id = hit_id;
      }
      DT_LOG_DEBUG(get_logging_priority(), "Last hit ID=" << last_hit_id);

      // Collection of atomic signals:
      std::vector<mctools::signal::base_signal> atomic_signals;
      static const std::string alpha_quenching_key
        = geomtools::sensitive::make_key(mctools::sensitive_utils::SENSITIVE_RECORD_ALPHA_QUENCHING);

      // Traverse the collection of calo hits and build the collection of
      // corresponding atomic signals:
      for (size_t ihit = 0; ihit < number_of_calo_hits; ihit++) {
        // Extract the hit:
        const mctools::base_step_hit & calo_hit = sim_data_.get_step_hit(hit_label, ihit);
        // Fetch
        unsigned int calo_hit_id = calo_hit.get_hit_id();
        const geomtools::geom_id & calo_gid = calo_hit.get_geom_id();
        const geomtools::vector_3d & start_position = calo_hit.get_position_start();
        const geomtools::vector_3d & stop_position = calo_hit.get_position_stop();
        const std::string & particle_name = calo_hit.get_particle_name();
        const double start_signal_time = calo_hit.get_time_start();
        const double stop_signal_time  = calo_hit.get_time_stop();
        const double energy_deposit    = calo_hit.get_energy_deposit();
        /*
         * quenching_factor should depend on the particle name
         * and association of delta-rays to a primary alpha track:
         *
         */
        const double quenching_factor = 1.0;
        // Detect an alpha particle:
        bool alpha = (particle_name == "alpha");
        bool alpha_quenching = false;
        if (alpha) {
          alpha_quenching = true;
        }
        if (particle_name == "electron"
            && calo_hit.get_auxiliaries().has_flag(alpha_quenching_key)) {
          alpha_quenching = true;
        }

        const double geometric_factor  = 1.0;
        /*
         * geometric_factor should depend on the relative position of the hit with respect to
         * the scintillator surface, the PMT glass and the optical module axis.
         *
         */
        const geomtools::vector_3d & mean_position = 0.5 * (start_position + stop_position);

        const double effective_energy_deposit = energy_deposit * quenching_factor * geometric_factor;

        // Build main parameters of the triangle signal:
        const double t0 = start_signal_time - event_time_ref;
        const double t1 = t0 + _rise_time_;
        const double t2 = t1 + _fall_time_;
        const double amplitude = _convert_energy_to_amplitude(effective_energy_deposit);

        bool build_signal = true;
        // Should we consider case where we do not generate the
        // signal associated to the hit ?
        if (!build_signal) {
          continue;
        }
        // Build a new signal associated to the hit:
        {
          mctools::signal::base_signal new_signal;
          atomic_signals.push_back(new_signal);
        }
        mctools::signal::base_signal & a_signal = atomic_signals.back();
        int signal_id = this->get_running_signal_id();
        this->_increment_running_signal_id();
        a_signal.set_hit_id(signal_id);
        a_signal.set_geom_id(calo_gid);
        a_signal.grab_auxiliaries().store_integer("origin_hit_id", calo_hit_id);
        a_signal.set_category(get_signal_category());
        a_signal.set_time_ref(event_time_ref);
        a_signal.set_shape_type_id("mctools::signal::triangle_signal_shape");
        a_signal.set_shape_string_parameter("polarity", "-");
        a_signal.set_shape_real_parameter_with_explicit_unit("t0", t0, "ns");
        a_signal.set_shape_real_parameter_with_explicit_unit("t1", t1, "ns");
        a_signal.set_shape_real_parameter_with_explicit_unit("t2", t2, "ns");
        a_signal.set_shape_real_parameter_with_explicit_unit("amplitude", amplitude, "V");
        a_signal.initialize_simple();
        if (datatools::logger::is_debug(get_logging_priority())) {
          DT_LOG_DEBUG(get_logging_priority(), "Calo hit signal in driver : ");
          a_signal.tree_dump(std::cerr, "", "[debug] ");
        }
        DT_LOG_DEBUG(get_logging_priority(), "Signal ID = " << signal_id);

        // Update the map of multiplicity per GID:
        auto it = map_gid_hit_in_gid.find(calo_gid);
        bool already_exist = false;
        if (it != map_gid_hit_in_gid.end()) already_exist = true;
        // Increment the hit multiplicity associated to the block:
        if (already_exist) {
          map_gid_hit_in_gid.find(calo_gid)->second += 1;
        } else {
          map_gid_hit_in_gid.insert(std::pair<geomtools::geom_id, unsigned int>(calo_gid, 1));
        }
        // Update the set of hit GIDs:
        set_of_gids.insert(calo_gid);
      }

      // Merge signals which are in the same calo block (thanks to GID) :
      // int current_multi_signal_id = last_signal_id;
      for (auto it = map_gid_hit_in_gid.begin(); it != map_gid_hit_in_gid.end(); it++) {
        const geomtools::geom_id & gid = it->first;
        DT_LOG_DEBUG(get_logging_priority(), "Processing signals in GID=" << gid << "...");
        if (it->second == 1) {
          // There is only one signal in the block, we scan the collection to locate it :
          mctools::signal::base_signal & sim_signal
            = sim_signal_data_.add_signal(get_signal_category());
          for (size_t isig = 0; isig < atomic_signals.size(); isig++) {
            if (atomic_signals[isig].get_geom_id() == gid) {
              // Find the signal hit with the corresponding GID:
              DT_LOG_DEBUG(get_logging_priority(), "Found an unique signal.");
              // Direct copy of the signal:
              sim_signal = atomic_signals[isig];
              break;
            }
          }
        } else {
          std::size_t number_of_signals_in_gid = it->second;
          DT_LOG_DEBUG(get_logging_priority(), "Merging " << number_of_signals_in_gid << " calo signal in GID=" << gid << "...");

          // Several signal in the same block:
          mctools::signal::base_signal & sim_signal
            = sim_signal_data_.add_signal(get_signal_category());
          // The composite signal receives a new ID from the driver:
          int sim_signal_id = this->get_running_signal_id();
          this->_increment_running_signal_id();
          sim_signal.set_hit_id(sim_signal_id);
          sim_signal.set_geom_id(gid);
          sim_signal.set_category(get_signal_category());
          sim_signal.set_time_ref(event_time_ref);
          sim_signal.set_shape_type_id("mctools::signal::multi_signal_shape");

          datatools::properties multi_signal_config;
          // Traverse the collection of atomic signals with the same GID:
          std::vector<std::string> component_labels;
          std::size_t counter_of_signals_in_gid = 0;
          for (size_t isig = 0; isig < atomic_signals.size(); isig++) {
            const mctools::signal::base_signal & atomic_signal = atomic_signals[isig];
            const int comp_hit_id = atomic_signal.get_hit_id();
            if (atomic_signal.get_geom_id() != gid) {
              continue;
            }
            DT_LOG_DEBUG(get_logging_priority(), "Found another signal to merge.");
            counter_of_signals_in_gid++;
            std::string comp_hit_key = "hit" + boost::lexical_cast<std::string>(component_labels.size());
            component_labels.push_back(comp_hit_key);
            std::string comp_hit_prefix = "components." + comp_hit_key + ".";
            datatools::properties comp_shape_parameters;
            // Extract shape parameters from the atomic signal:
            atomic_signal.get_auxiliaries().export_and_rename_starting_with(comp_shape_parameters,
                                                                            mctools::signal::base_signal::shape_parameter_prefix(),
                                                                            "");
            std::string comp_hit_label;
            build_private_signal_name(comp_hit_id, comp_hit_label);
            // Make the atomic signal a private shape:
            sim_signal.add_private_shape(comp_hit_label,
                                         atomic_signal.get_shape_type_id(),
                                         comp_shape_parameters);
            // Private shape
            std::string key_key        = comp_hit_prefix + "key";
            std::string time_shift_key = comp_hit_prefix + "time_shift";
            std::string scaling_key    = comp_hit_prefix + "scaling";
            multi_signal_config.store(key_key, comp_hit_label);
            multi_signal_config.store_real_with_explicit_unit(time_shift_key, 0.0 * CLHEP::ns);
            multi_signal_config.set_unit_symbol(time_shift_key, "ns");
            multi_signal_config.store_real(scaling_key, 1.0);
          }
          multi_signal_config.store("components", component_labels);
          sim_signal.set_shape_parameters(multi_signal_config);
	  sim_signal.initialize(multi_signal_config);

          if (counter_of_signals_in_gid > number_of_signals_in_gid) {
            // All signal in this GIDs have been taken into account:
            break;
          }
	}
      }
      DT_LOG_DEBUG(get_logging_priority(), "Signal data: ");
      // sim_signal_data_.tree_dump(std::cerr, "", "[debug] ");
      if (datatools::logger::is_debug(get_logging_priority())) {
        for (int isig = 0; isig < (int) sim_signal_data_.get_number_of_signals(get_signal_category()); isig++) {
          const mctools::signal::base_signal & sig = sim_signal_data_.get_signal(get_signal_category(), isig);
          // sig.tree_dump(std::cerr, "Embedded signal: ", "[debug] ");
        }
      }
      return;
    }

    void calo_signal_generator_driver::_tree_dump(std::ostream & out_,
                                                  const std::string & indent_) const
    {

      std::string model_str = "";
      if (get_model() == MODEL_INVALID) model_str = "<none>";
      else if (get_model() == MODEL_TRIANGLE) model_str = "triangle";

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Driver specific parameters  : " << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::skip_tag << datatools::i_tree_dumpable::tag
           << "Model      : '" << model_str << "'"
           << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::skip_tag << datatools::i_tree_dumpable::tag
           << "Rise time  : " << _rise_time_ / CLHEP::ns << " ns"
           << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::skip_tag << datatools::i_tree_dumpable::tag
           << "Fall time  : " << _fall_time_ / CLHEP::ns << " ns"
           << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::skip_tag << datatools::i_tree_dumpable::last_tag
           << "E/A factor : " << _energy_amplitude_factor_ / (CLHEP::volt/CLHEP::MeV) << " V/MeV"
           << std::endl;

      return;
    }

  } // end of namespace asb

} // end of namespace snemo
