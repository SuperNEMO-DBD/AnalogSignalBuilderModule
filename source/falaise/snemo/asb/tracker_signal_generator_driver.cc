// snemo/asb/tracker_signal_generator_driver.cc
//
// Copyright (c) 2016-2018 F. Mauger <mauger@lpccaen.in2p3.fr>
//                         G. Oliviéro <goliviero@lpccaen.in2p3.fr>
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
#include <snemo/asb/tracker_signal_generator_driver.h>

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

    void tracker_signal_generator_driver::_set_defaults_()
    {
      _model_ = MODEL_INVALID;
      // _rise_time_ = 8 * CLHEP::ns;  // Rise time on tracker signal (from Bordeaux wavecatcher signals)
      // _fall_time_ = 70 * CLHEP::ns; // Fall time on tracker signal (from Bordeaux wavecatcher signals)
      // _energy_amplitude_factor_ = 0.3 * CLHEP::volt / CLHEP::MeV; // Energy to amplitude conversion factor
      return;
    }

    void tracker_signal_generator_driver::set_model(const model_type model_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized and locked!");
      _model_ = model_;
      return;
    }

    tracker_signal_generator_driver::model_type tracker_signal_generator_driver::get_model() const
    {
      return _model_;
    }

    // void tracker_signal_generator_driver::set_rise_time(const double rise_time_)
    // {
    //   DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized and locked!");
    //   _rise_time_ = rise_time_;
    //   return;
    // }

    // double tracker_signal_generator_driver::get_rise_time() const
    // {
    //   return _rise_time_;
    // }

    // void tracker_signal_generator_driver::set_fall_time(const double fall_time_)
    // {
    //   DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized and locked!");
    //   _fall_time_ = fall_time_;
    //   return;
    // }

    // double tracker_signal_generator_driver::get_fall_time() const
    // {
    //   return _fall_time_;
    // }

    // void tracker_signal_generator_driver::set_energy_amplitude_factor(const double factor_)
    // {
    //   DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized and locked!");
    //   _energy_amplitude_factor_ = factor_;
    //   return;
    // }

    // double tracker_signal_generator_driver::get_energy_amplitude_factor() const
    // {
    //   return _energy_amplitude_factor_;
    // }

    tracker_signal_generator_driver::tracker_signal_generator_driver(const std::string & id_)
      : base_signal_generator_driver(id_)
    {
      _model_ = MODEL_INVALID;
      return;
    }

    tracker_signal_generator_driver::tracker_signal_generator_driver(const model_type model_,
								     const std::string & id_)
      : base_signal_generator_driver(id_),
        _model_(model_)
    {
      return;
    }

    tracker_signal_generator_driver::~tracker_signal_generator_driver()
    {
      if (is_initialized())
	{
	  this->tracker_signal_generator_driver::reset();
	}
      return;
    }

    void tracker_signal_generator_driver::_initialize(const datatools::properties & config_)
    {
      if (_model_ == MODEL_INVALID)
	{
	  if (config_.has_key("model"))
	    {
	      std::string model_label = config_.fetch_string("model");
	      if (model_label == "triangle_gate")
		{
		  set_model(MODEL_TRIANGLE_GATE);
		}
	      else
		{
		  DT_THROW(std::logic_error, "Unsupported driver model '" << model_label << "'!");
		}
	    }
	}

      if (_model_ == MODEL_INVALID)
	{
	  DT_THROW(std::logic_error, "Missing driver model!");
	}

      // if (config_.has_key("rise_time")) {
      //   double rise_time = config_.fetch_real_with_explicit_dimension("rise_time", "time");
      //   _rise_time_ = rise_time;
      // }

      // if (config_.has_key("fall_time")) {
      //   double fall_time = config_.fetch_real_with_explicit_dimension("fall_time", "time");
      //   _fall_time_ = fall_time;
      // }

      // if (config_.has_key("energy_amplitude_factor")) {
      //   double e2a =  config_.fetch_dimensionless_real("energy_amplitude_factor");
      //   _energy_amplitude_factor_ = e2a * CLHEP::volt / CLHEP::MeV;
      // }

      return;
    }

    void tracker_signal_generator_driver::_reset()
    {
      // clear resources...
      _model_ = MODEL_INVALID;
      _set_defaults_();
      return;
    }

    // double tracker_signal_generator_driver::_convert_energy_to_amplitude(const double energy_)
    // {
    //   double amplitude = _energy_amplitude_factor_ * energy_;
    //   return amplitude; // maybe units problem for the moment
    // }

    void tracker_signal_generator_driver::_process(const mctools::simulated_data & sim_data_,
						   mctools::signal::signal_data & sim_signal_data_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Tracker signal generator driver is not initialized !");
      DT_THROW_IF(_model_ == MODEL_INVALID, std::logic_error, "Tracker signal generator driver model is invalid !");
      if (_model_ == MODEL_TRIANGLE_GATE)
	{
	  std::clog << "triangle gate " << std::endl;
	  _process_triangle_gate_model_(sim_data_, sim_signal_data_);
	}
      return;
    }

    void tracker_signal_generator_driver::_process_triangle_gate_model_(const mctools::simulated_data & sim_data_,
									mctools::signal::signal_data & sim_signal_data_)
    {
      // For the moment, each trackerrimeter hit is represented by a triangle gate tracker signal.
      // The next step is to take into account multi hit into one GID. Several
      // 'small' signals must construct a 'multi signal' for one GID (sum)

      std::string hit_label = get_hit_category();
      if (!sim_data_.has_step_hits(hit_label))
	{
	  // No hit in the proper category:
	  return;
	}
      const size_t number_of_tracker_hits = sim_data_.get_number_of_step_hits(hit_label);

      // pickup the ID mapping from the geometry manager:
      const geomtools::mapping& the_mapping = get_geo_manager().get_mapping();

      // Collection of atomic signals:
      std::vector<mctools::signal::base_signal> atomic_signals;

      // Traverse the collection of tracker hits and build the collection of
      // corresponding atomic signals:
      for (size_t ihit = 0; ihit < number_of_tracker_hits; ihit++)
	{
	  // extract the hit:
	  const mctools::base_step_hit & a_tracker_hit = sim_data_.get_step_hit(hit_label, ihit);

	  // extract the corresponding geom ID:
	  const geomtools::geom_id & gid = a_tracker_hit.get_geom_id();

	  // extract the geom info of the corresponding cell:
	  const geomtools::geom_info& ginfo = the_mapping.get_geom_info(gid);

	  // the position of the ion/electron pair creation within the cell volume:
	  const geomtools::vector_3d & ionization_world_pos = a_tracker_hit.get_position_start();

	  // the position of the Geiger avalanche impact on the anode wire:
	  const geomtools::vector_3d & avalanche_impact_world_pos = a_tracker_hit.get_position_stop();

	  // compute the position of the anode impact in the drift cell coordinates reference frame:
	  geomtools::vector_3d avalanche_impact_cell_pos;
	  ginfo.get_world_placement().mother_to_child(avalanche_impact_world_pos,
						      avalanche_impact_cell_pos);
	  // longitudinal position:
	  const double longitudinal_position = avalanche_impact_cell_pos.z();

	  // true drift distance:
	  const double drift_distance = (avalanche_impact_world_pos - ionization_world_pos).mag();


	  DT_LOG_DEBUG(get_logging_priority(), "Geom ID : " << gid);
	  DT_LOG_DEBUG(get_logging_priority(), "ionization_world_pos : " << ionization_world_pos);
	  DT_LOG_DEBUG(get_logging_priority(), "avalanche_impact_world_pos : " << avalanche_impact_world_pos);
	  DT_LOG_DEBUG(get_logging_priority(), "avalanche_impact_cell_pos : " << avalanche_impact_cell_pos);
	  DT_LOG_DEBUG(get_logging_priority(), "longitudinal_position_pos : " << longitudinal_position);
	  DT_LOG_DEBUG(get_logging_priority(), "drift_distance : " << drift_distance);



	} // end of ihit

      DT_LOG_DEBUG(get_logging_priority(), "Signal data: ");
      sim_signal_data_.tree_dump(std::cerr, "", "[debug] ");
      if (datatools::logger::is_debug(get_logging_priority()))
	{
	  for (int isig = 0; isig < (int) sim_signal_data_.get_number_of_signals(get_signal_category()); isig++)
	    {
	      const mctools::signal::base_signal & sig = sim_signal_data_.get_signal(get_signal_category(), isig);
	      sig.tree_dump(std::cerr, "Embedded signal: ", "[debug] ");
	    }
	}
      return;
    }

    void tracker_signal_generator_driver::_tree_dump(std::ostream & out_,
						     const std::string & indent_) const
    {

      std::string model_str = "";
      if (get_model() == MODEL_INVALID) model_str = "<none>";
      else if (get_model() == MODEL_TRIANGLE_GATE) model_str = "triangle_gate";

      out_ << indent_ << datatools::i_tree_dumpable::tag
	   << "Driver specific parameters  : " << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::skip_tag << datatools::i_tree_dumpable::tag
	   << "Model      : '" << model_str << "'"
	   << std::endl;

      // out_ << indent_ << datatools::i_tree_dumpable::skip_tag << datatools::i_tree_dumpable::tag
      //      << "Rise time  : " << _rise_time_ / CLHEP::ns << " ns"
      //      << std::endl;

      // out_ << indent_ << datatools::i_tree_dumpable::skip_tag << datatools::i_tree_dumpable::tag
      //      << "Fall time  : " << _fall_time_ / CLHEP::ns << " ns"
      //      << std::endl;

      // out_ << indent_ << datatools::i_tree_dumpable::skip_tag << datatools::i_tree_dumpable::last_tag
      //      << "E/A factor : " << _energy_amplitude_factor_ / (CLHEP::volt/CLHEP::MeV) << " V/MeV"
      //      << std::endl;

      return;
    }

  } // end of namespace asb

} // end of namespace snemo
