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
// - Bayeux/mctools:
#include <bayeux/mctools/signal/base_signal.h>
#include <bayeux/mctools/utils.h>

// This project:
#include <snemo/asb/utils.h>

namespace snemo {

  namespace asb {

    // Registration instantiation macro :
    SNEMO_ASB_SIGNAL_GENERATOR_DRIVER_REGISTRATION_IMPLEMENT(tracker_signal_generator_driver, "snemo::asb::tracker_signal_generator_driver")

    /*** Implementation of the interface ***/

    void tracker_signal_generator_driver::_set_defaults_()
    {
      _model_ = MODEL_INVALID;
      _rise_time_ = 1 * CLHEP::microsecond; // Rise time on tracker signal
      _fall_time_ = 1 * CLHEP::microsecond; // Fall time on tracker signal
      _amplitude_ = 100e-3 * CLHEP::volt;   // Amplitude for a tracker signal
      return;
    }

    void tracker_signal_generator_driver::set_model(const model_type model_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized !");
      _model_ = model_;
      return;
    }

    tracker_signal_generator_driver::model_type tracker_signal_generator_driver::get_model() const
    {
      return _model_;
    }

    void tracker_signal_generator_driver::set_rise_time(const double rise_time_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized and locked!");
      _rise_time_ = rise_time_;
      return;
    }

    double tracker_signal_generator_driver::get_rise_time() const
    {
      return _rise_time_;
    }

    void tracker_signal_generator_driver::set_fall_time(const double fall_time_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized and locked!");
      _fall_time_ = fall_time_;
      return;
    }

    double tracker_signal_generator_driver::get_fall_time() const
    {
      return _fall_time_;
    }

    void tracker_signal_generator_driver::set_amplitude(const double amplitude_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized and locked!");
      _amplitude_ = amplitude_;
      return;
    }

    double tracker_signal_generator_driver::get_amplitude() const
    {
      return _amplitude_;
    }

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

      _geiger_.initialize(config_);

      if (config_.has_key("rise_time")) {
        double rise_time = config_.fetch_real_with_explicit_dimension("rise_time", "time");
        _rise_time_ = rise_time;
      }

      if (config_.has_key("fall_time")) {
        double fall_time = config_.fetch_real_with_explicit_dimension("fall_time", "time");
        _fall_time_ = fall_time;
      }

      if (config_.has_key("amplitude")) {
        double amplitude =  config_.fetch_real_with_explicit_dimension("amplitude", "electric_potential");
        _amplitude_ = amplitude;
      }

      return;
    }


    void tracker_signal_generator_driver::_reset()
    {
      // clear resources...
      _model_ = MODEL_INVALID;
      _geiger_.reset();
      _set_defaults_();
      return;
    }

    void tracker_signal_generator_driver::_process(const mctools::simulated_data & sim_data_,
						   mctools::signal::signal_data  & sim_signal_data_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Tracker signal generator driver is not initialized !");
      DT_THROW_IF(_model_ == MODEL_INVALID, std::logic_error, "Tracker signal generator driver model is invalid !");
      if (_model_ == MODEL_TRIANGLE_GATE)
	{
	  _process_triangle_gate_model_(sim_data_, sim_signal_data_);
	}
      return;
    }

    void tracker_signal_generator_driver::_process_triangle_gate_model_(const mctools::simulated_data & sim_data_,
									mctools::signal::signal_data  & sim_signal_data_)
    {
      // For the moment, each tracker hit is represented by a triangle gate.

      std::string hit_label = get_hit_category();
      if (!sim_data_.has_step_hits(hit_label))
	{
	  // No hit in the proper category:
	  return;
	}

      // pickup the ID mapping from the geometry manager:
      const geomtools::mapping& the_mapping = get_geo_manager().get_mapping();

      // Temporary output in a file for check purpose
      std::string temp_filename = "/tmp/anode_efficiency.dat";
      std::ofstream temp_stream;
      temp_stream.open(temp_filename);
      for (double r = 0 * CLHEP::mm; r <= 35 * CLHEP::mm; r+=0.1 * CLHEP::mm)
	{
	  int mode = 1;
	  double an_eff = _geiger_.get_anode_efficiency(r, mode);
	  temp_stream << r  << ' ' << an_eff << std::endl;
	}
      temp_stream.close();


      // WIP : is this really coherent for the event time reference for tracker ?
      // // Reference time of the event:
      // double event_time_ref;
      // datatools::invalidate(event_time_ref);
      // // Traverse the collection of tracker to search the first tracker hit. It will define the event time ref for the tracker :
      // for (size_t ihit = 0; ihit < number_of_tracker_hits; ihit++)
      // 	{
      // 	  const mctools::base_step_hit & tracker_hit = sim_data_.get_step_hit(hit_label, ihit);
      // 	  const double start_signal_time = tracker_hit.get_time_start();

      // 	  if (!datatools::is_valid(event_time_ref)) event_time_ref = start_signal_time;
      // 	  if (start_signal_time < event_time_ref) event_time_ref = start_signal_time;
      // 	}


      // For a given GID (Geiger cell), several hits can happen (pile-up)
      // Only the first hit will produce a signal if the Geiger cell hasn't recovery it electric field (dead time)

      // New mutable 'SD' bank is created (copy of the input SD) but degraded.
      // We want to flag hits taking place between the 1st hit and the recovery dead time of the cell
      // Geiger dead time is taken into account and a given geiger cell can trigger again after 1 ms

      mctools::simulated_data flaged_SD = sim_data_;
      const size_t total_number_of_tracker_hits = sim_data_.get_number_of_step_hits(hit_label);

      for (size_t ihit = 0; ihit < total_number_of_tracker_hits; ihit++)
      	{
      	  // Search the drift distance closer to the anodic wire for a given cell
      	  // The first hit with the smallest drift distance will give a signal and then the cell is dead
      	  // After the dead time recovery (1 ms), an other hit can give a new tracker signal

      	  mctools::base_step_hit & GGH_1 = flaged_SD.grab_step_hit("gg", ihit);

      	  for (size_t jhit = ihit + 1; jhit < total_number_of_tracker_hits; jhit++)
      	    {
      	      mctools::base_step_hit & GGH_2 = flaged_SD.grab_step_hit("gg", jhit);
      	      if (GGH_1.get_geom_id() == GGH_2.get_geom_id())
      		{
      		  const geomtools::vector_3d & ionization_gg_1 = GGH_1.get_position_start();
      		  const geomtools::vector_3d & avalanche_gg_1  = GGH_1.get_position_stop();
      		  const double drift_distance_gg_1 = (avalanche_gg_1 - ionization_gg_1).mag();
      		  const double ionization_time_gg_1 = GGH_1.get_time_start();
		  const double computed_drift_time_gg_1 = _geiger_.compute_drift_time_from_drift_distance(drift_distance_gg_1);
		  const double anode_time_gg_1 = ionization_time_gg_1 + computed_drift_time_gg_1;

      		  const geomtools::vector_3d & ionization_gg_2 = GGH_2.get_position_start();
      		  const geomtools::vector_3d & avalanche_gg_2  = GGH_2.get_position_stop();
      		  const double drift_distance_gg_2 = (avalanche_gg_2 - ionization_gg_2).mag();
      		  const double ionization_time_gg_2 = GGH_2.get_time_start();
		  const double computed_drift_time_gg_2 = _geiger_.compute_drift_time_from_drift_distance(drift_distance_gg_2);
		  const double anode_time_gg_2 = ionization_time_gg_2 + computed_drift_time_gg_2;

      		  const double gg_hit_time       = anode_time_gg_1;
      		  const double other_gg_hit_time = anode_time_gg_2;

      		  if (gg_hit_time > other_gg_hit_time && gg_hit_time < other_gg_hit_time + _geiger_.get_cell_dead_time())
      		    {
      		      bool geiger_already_hit = true;
      		      if (!GGH_1.get_auxiliaries().has_flag("geiger_already_hit")) GGH_1.grab_auxiliaries().store("geiger_already_hit", geiger_already_hit);
      		    }
      		  else if (other_gg_hit_time > gg_hit_time && other_gg_hit_time < gg_hit_time + _geiger_.get_cell_dead_time())
      		    {
      		      bool geiger_already_hit = true;
      		      if (!GGH_2.get_auxiliaries().has_flag("geiger_already_hit")) GGH_2.grab_auxiliaries().store("geiger_already_hit", geiger_already_hit);
      		    }
      		}
      	    } // end of jhit
      	} // end of ihit

      double number_of_tracker_hits = 0;
      for (std::size_t ihit = 0; ihit < flaged_SD.get_number_of_step_hits(hit_label); ihit++)
	{
	  if (!flaged_SD.grab_step_hit("gg", ihit).get_auxiliaries().has_flag("geiger_already_hit")) number_of_tracker_hits++;
	}


      // Traverse the collection of tracker hits and build the collection of
      for (std::size_t ihit = 0; ihit < number_of_tracker_hits; ihit++)
	{
	  // extract the hit:
	  const mctools::base_step_hit & a_tracker_hit = flaged_SD.get_step_hit(hit_label, ihit);

	  if (a_tracker_hit.get_auxiliaries().has_flag("geiger_already_hit"))
	    {
	      // Skip hits with flag 'already hit'
	      continue;
	    }

	  // extract the corresponding geom ID:
	  const geomtools::geom_id & gid = a_tracker_hit.get_geom_id();

	  // extract the geom info of the corresponding cell:
	  const geomtools::geom_info & ginfo = the_mapping.get_geom_info(gid);

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

	  // NOTE : anode and cathode efficiencies are set to 100%. There is no lost (WIP).

	  // the time of the ion/electron pair creation:
	  const double ionization_time = a_tracker_hit.get_time_start();

	  /*** Anode TDC ***/
	  // Compute the Geiger drift time:
	  const double computed_drift_time = _geiger_.compute_drift_time_from_drift_distance(drift_distance);

	  const double anode_time = ionization_time + computed_drift_time;
	  const double sigma_anode_time = _geiger_.get_sigma_anode_time(anode_time);

	  /*** Cathodes TDCs ***/
	  double bottom_cathode_time;
	  double top_cathode_time;
	  datatools::invalidate(bottom_cathode_time);
	  datatools::invalidate(top_cathode_time);
	  // Maybe use the 'real' sigma cathode time and use it for the mean value for the gaussian shoot.
	  const double l_bottom = longitudinal_position + 0.5 * _geiger_.get_cell_length();
	  const double mean_bottom_cathode_time = l_bottom / _geiger_.get_plasma_longitudinal_speed();
	  bottom_cathode_time = mean_bottom_cathode_time;

	  const double l_top = 0.5 * _geiger_.get_cell_length() - longitudinal_position;
	  const double mean_top_cathode_time = l_top / _geiger_.get_plasma_longitudinal_speed();
	  top_cathode_time = mean_top_cathode_time;

	  // The anodic tracker signal has 2 components (2 subsignals) due to the plasma propagation along the anodic wire.
	  // 2 plasmas are created, one collected by the top cathode and the second collected by the bottom cathode
	  // To emulate this behaviour, the anodic signal is composed by 2 triangle gate signal shape.

	  // Signals beginning (common) :
	  const double t0 = anode_time;
	  const double t1 = t0 + _rise_time_;

	  // Anodic signal 1 :
	  const double sig1_duration = bottom_cathode_time; // No soustraction with t1 because it is a delay because the avalanche must take place before the plasma propagation
	  const double sig1_t2 = t1 + sig1_duration;
	  const double sig1_t3 = sig1_t2 + _fall_time_;
	  const double sig1_amplitude = get_amplitude() / 2.;

	  const geomtools::id_mgr & an_id_mgr = get_geo_manager().get_id_mgr();

	  const geomtools::id_mgr::categories_by_name_col_type & categories = an_id_mgr.categories_by_name();

	  uint32_t anodic_wire_type   = categories.find("drift_cell_anodic_wire")->second.get_type();
	  uint32_t cathodic_ring_type = categories.find("drift_cell_cathodic_ring")->second.get_type();

	  const geomtools::id_mgr::category_info & cathode_info = get_geo_manager().get_id_mgr().get_category_info(cathodic_ring_type);

	  geomtools::geom_id anodic_wire_GID = gid;
	  anodic_wire_GID.set_type(anodic_wire_type);

	  geomtools::geom_id cathodic_ring_bottom_GID = gid;
	  cathodic_ring_bottom_GID.set_type(cathodic_ring_type);
	  cathodic_ring_bottom_GID.set_depth(cathodic_ring_bottom_GID.get_depth() + 1);
	  cathodic_ring_bottom_GID.set(cathode_info.get_subaddress_index("ring"), 0); // 0 is for bottom

	  geomtools::geom_id cathodic_ring_top_GID = gid;
	  cathodic_ring_top_GID.set_type(cathodic_ring_type);
	  cathodic_ring_top_GID.set_depth(cathodic_ring_top_GID.get_depth() + 1);
	  cathodic_ring_top_GID.set(cathode_info.get_subaddress_index("ring"), 1);    // 1 is for top

	  // Build main parameters of the anodic triangle gate signals:
	  mctools::signal::base_signal signal_1;
	  int signal_id = this->get_running_signal_id();
	  signal_1.set_hit_id(signal_id);
	  this->_increment_running_signal_id();
	  signal_1.set_geom_id(anodic_wire_GID);
	  signal_1.set_category(get_signal_category());

	  // What is the time reference for tracker signals ? 1st tracker hit or first PMT ? What happened if there is no PMT hit... (WIP)
	  // For the moment keep 0 as the time reference
	  const double event_time_ref = 0;
	  signal_1.set_time_ref(event_time_ref);
	  signal_1.set_shape_type_id("mctools::signal::triangle_gate_signal_shape");
	  signal_1.set_shape_string_parameter("polarity", "-");
	  signal_1.set_shape_real_parameter_with_explicit_unit("t0", t0, "ns");
	  signal_1.set_shape_real_parameter_with_explicit_unit("t1", t1, "ns");
	  signal_1.set_shape_real_parameter_with_explicit_unit("t2", sig1_t2, "ns");
	  signal_1.set_shape_real_parameter_with_explicit_unit("t3", sig1_t3, "ns");
	  signal_1.set_shape_real_parameter_with_explicit_unit("amplitude", sig1_amplitude, "V");
	  signal_1.grab_auxiliaries().store_string("subcategory", "anodic");
	  signal_1.initialize_simple();
	  // signal_1.tree_dump(std::clog, "Bot signal");

	  // Anodic signal 2 :
	  const double sig2_duration = top_cathode_time; // No soustraction with t1 because it is a delay because the avalanche must take place before the plasma propagation
	  const double sig2_t2 = t1 + sig2_duration;
	  const double sig2_t3 = sig2_t2 + _fall_time_;
	  const double sig2_amplitude = get_amplitude() / 2.;

	  mctools::signal::base_signal signal_2;
	  signal_id = this->get_running_signal_id();
	  signal_2.set_hit_id(signal_id);
	  this->_increment_running_signal_id();
	  signal_2.set_geom_id(anodic_wire_GID);
	  signal_2.set_category(get_signal_category());

	  // What is the time reference for tracker signals ? 1st tracker hit or first PMT ? What happened if there is no PMT hit... (WIP)
	  // For the moment keep 0 as the time reference
	  signal_2.set_time_ref(event_time_ref);
	  signal_2.set_shape_type_id("mctools::signal::triangle_gate_signal_shape");
	  signal_2.set_shape_string_parameter("polarity", "-");
	  signal_2.set_shape_real_parameter_with_explicit_unit("t0", t0, "ns");
	  signal_2.set_shape_real_parameter_with_explicit_unit("t1", t1, "ns");
	  signal_2.set_shape_real_parameter_with_explicit_unit("t2", sig2_t2, "ns");
	  signal_2.set_shape_real_parameter_with_explicit_unit("t3", sig2_t3, "ns");
	  signal_2.set_shape_real_parameter_with_explicit_unit("amplitude", sig2_amplitude, "V");
	  signal_2.grab_auxiliaries().store_string("subcategory", "anodic");
	  signal_2.initialize_simple();
	  // signal_2.tree_dump(std::clog, "Top signal");

	  const double top_cathode_t0 = sig2_t2;
	  const double top_cathode_t1 = top_cathode_t0 + _rise_time_ / 2.;
	  const double top_cathode_t2 = top_cathode_t1 + _fall_time_ / 2.;
	  const double top_cathode_amplitude = sig2_amplitude;

	  const double bottom_cathode_t0 = sig1_t2;
	  const double bottom_cathode_t1 = bottom_cathode_t0 + _rise_time_ / 2.;
	  const double bottom_cathode_t2 = bottom_cathode_t1 + _fall_time_ / 2.;
	  const double bottom_cathode_amplitude = sig1_amplitude;

	  // Construct peak on anodic signal due to cathodic ring collection
	  mctools::signal::base_signal signal_3;
	  signal_id = this->get_running_signal_id();
	  signal_3.set_hit_id(signal_id);
	  this->_increment_running_signal_id();
	  signal_3.set_geom_id(anodic_wire_GID);
	  signal_3.set_category(get_signal_category());
	  signal_3.set_time_ref(event_time_ref);
	  signal_3.set_shape_type_id("mctools::signal::triangle_signal_shape");
	  signal_3.set_shape_string_parameter("polarity", "-");
	  signal_3.set_shape_real_parameter_with_explicit_unit("t0", top_cathode_t0, "ns");
	  signal_3.set_shape_real_parameter_with_explicit_unit("t1", top_cathode_t1, "ns");
	  signal_3.set_shape_real_parameter_with_explicit_unit("t2", top_cathode_t2, "ns");
	  signal_3.set_shape_real_parameter_with_explicit_unit("amplitude", 2 * top_cathode_amplitude, "V");
	  signal_3.grab_auxiliaries().store_string("subcategory", "anodic");
	  signal_3.initialize_simple();

	  mctools::signal::base_signal signal_4;
	  signal_id = this->get_running_signal_id();
	  signal_4.set_hit_id(signal_id);
	  this->_increment_running_signal_id();
	  signal_4.set_geom_id(anodic_wire_GID);
	  signal_4.set_category(get_signal_category());
	  signal_4.set_time_ref(event_time_ref);
	  signal_4.set_shape_type_id("mctools::signal::triangle_signal_shape");
	  signal_4.set_shape_string_parameter("polarity", "-");
	  signal_4.set_shape_real_parameter_with_explicit_unit("t0", bottom_cathode_t0, "ns");
	  signal_4.set_shape_real_parameter_with_explicit_unit("t1", bottom_cathode_t1, "ns");
	  signal_4.set_shape_real_parameter_with_explicit_unit("t2", bottom_cathode_t2, "ns");
	  signal_4.set_shape_real_parameter_with_explicit_unit("amplitude", 2 * bottom_cathode_amplitude, "V");
	  signal_4.grab_auxiliaries().store_string("subcategory", "anodic");
	  signal_4.initialize_simple();

	  std::vector<mctools::signal::base_signal> list_of_atomic_signal;
	  list_of_atomic_signal.push_back(signal_1);
	  list_of_atomic_signal.push_back(signal_2);
	  list_of_atomic_signal.push_back(signal_3);
	  list_of_atomic_signal.push_back(signal_4);


	  // Build a new signal composed by 2 sub signals (1 top 1 bottom)
	  mctools::signal::base_signal & a_signal = sim_signal_data_.add_signal(get_signal_category());
	  signal_id = this->get_running_signal_id();
	  a_signal.set_hit_id(signal_id);
	  this->_increment_running_signal_id();
	  a_signal.set_geom_id(anodic_wire_GID);
	  a_signal.set_category(get_signal_category());
	  a_signal.set_time_ref(event_time_ref);
	  a_signal.set_shape_type_id("mctools::signal::multi_signal_shape");
	  a_signal.grab_auxiliaries().store_string("subcategory", "anodic");

	  // Build the anodic multi signal from atomic signals:
	  build_multi_signal(a_signal, list_of_atomic_signal);

	  // Construct cathodic signals thanks to parameters already computed:
	  // Top cathode signal:
	  mctools::signal::base_signal & top_cathode_signal = sim_signal_data_.add_signal(get_signal_category());
	  signal_id = this->get_running_signal_id();
	  top_cathode_signal.set_hit_id(signal_id);
	  this->_increment_running_signal_id();
	  top_cathode_signal.set_geom_id(cathodic_ring_top_GID);
	  top_cathode_signal.set_category(get_signal_category());
	  top_cathode_signal.set_time_ref(event_time_ref);
	  top_cathode_signal.grab_auxiliaries().store_string("subcategory", "cathodic");
	  top_cathode_signal.set_shape_type_id("mctools::signal::triangle_signal_shape");
	  top_cathode_signal.set_shape_string_parameter("polarity", "+");
	  top_cathode_signal.set_shape_real_parameter_with_explicit_unit("t0", top_cathode_t0, "ns");
	  top_cathode_signal.set_shape_real_parameter_with_explicit_unit("t1", top_cathode_t1, "ns");
	  top_cathode_signal.set_shape_real_parameter_with_explicit_unit("t2", top_cathode_t2, "ns");
	  top_cathode_signal.set_shape_real_parameter_with_explicit_unit("amplitude", top_cathode_amplitude, "V");
	  top_cathode_signal.initialize_simple();

	  // Bottom cathode signal:
	  mctools::signal::base_signal & bottom_cathode_signal = sim_signal_data_.add_signal(get_signal_category());
	  signal_id = this->get_running_signal_id();
	  bottom_cathode_signal.set_hit_id(signal_id);
	  this->_increment_running_signal_id();
	  bottom_cathode_signal.set_geom_id(cathodic_ring_bottom_GID);
	  bottom_cathode_signal.set_category(get_signal_category());
	  bottom_cathode_signal.set_time_ref(event_time_ref);
	  bottom_cathode_signal.grab_auxiliaries().store_string("subcategory", "cathodic");
	  bottom_cathode_signal.set_shape_type_id("mctools::signal::triangle_signal_shape");
	  bottom_cathode_signal.set_shape_string_parameter("polarity", "+");
	  bottom_cathode_signal.set_shape_real_parameter_with_explicit_unit("t0", bottom_cathode_t0, "ns");
	  bottom_cathode_signal.set_shape_real_parameter_with_explicit_unit("t1", bottom_cathode_t1, "ns");
	  bottom_cathode_signal.set_shape_real_parameter_with_explicit_unit("t2", bottom_cathode_t2, "ns");
	  bottom_cathode_signal.set_shape_real_parameter_with_explicit_unit("amplitude", bottom_cathode_amplitude, "V");
	  bottom_cathode_signal.initialize_simple();


	  bool build_signal = true;
	  // Should we consider case where we do not generate the
	  // signal associated to the hit ?
	  if (!build_signal) {
	    continue;
	  }

	} // end of ihit

      DT_LOG_DEBUG(get_logging_priority(), "Signal data: ");
      // sim_signal_data_.tree_dump(std::cerr, "", "[debug] ");
      if (sim_signal_data_.has_signals(get_signal_category()))
	{
	  if (datatools::logger::is_debug(get_logging_priority()))
	    {
	      for (int isig = 0; isig < (int) sim_signal_data_.get_number_of_signals(get_signal_category()); isig++)
		{
		  const mctools::signal::base_signal & sig = sim_signal_data_.get_signal(get_signal_category(), isig);
		  // sig.tree_dump(std::cerr, "Tracker signal #" + std::to_string(sig.get_hit_id()) + " in SSD : ", "[debug] ");
		}
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

      out_ << indent_ << datatools::i_tree_dumpable::skip_tag << datatools::i_tree_dumpable::tag
           << "Rise time  : " << _rise_time_ / CLHEP::ns << " ns"
           << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::skip_tag << datatools::i_tree_dumpable::tag
           << "Fall time  : " << _fall_time_ / CLHEP::ns << " ns"
           << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::skip_tag << datatools::i_tree_dumpable::last_tag
	   << "Amplitude : " << _amplitude_ / CLHEP::volt << " V"
	   << std::endl;

      return;
    }

  } // end of namespace asb

} // end of namespace snemo
