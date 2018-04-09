// geiger_analogic_regime.cc
//
// Copyright (c) 2018 F. Mauger <mauger@lpccaen.in2p3.fr>
// Copyright (c) 2018 G. Oliviéro <goliviero@lpccaen.in2p3.fr>
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

// Standard library:
#include <cmath>
#include <sstream>

// Third party:
// - Bayeux/datatools:
#include <datatools/clhep_units.h>
#include <datatools/properties.h>
#include <datatools/utils.h>
// - Bayeux/mygsl:
#include <mygsl/rng.h>

// Ourselves:
#include <snemo/asb/geiger_analogic_regime.h>

namespace snemo {

  namespace asb {

    bool geiger_analogic_regime::is_initialized() const
    {
      return _initialized_;
    }

    void geiger_analogic_regime::reset()
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");
      _init_defaults_();
      _initialized_ = false;
      return;
    }

    void geiger_analogic_regime::initialize(const datatools::properties & config_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Already initialized !");

      double length_unit = CLHEP::mm;
      double time_unit = CLHEP::microsecond;
      double drift_speed_unit = (CLHEP::cm / CLHEP::microsecond);

      if (config_.has_key("cell_diameter"))
	{
	  _cell_diameter_ = config_.fetch_real("cell_diameter");
	  if (!config_.has_explicit_unit("cell_diameter"))
	    {
	      _cell_diameter_ *= length_unit;
	    }
	}

      if (config_.has_key("cell_length"))
	{
	  _cell_length_ = config_.fetch_real("cell_length");
	  if (!config_.has_explicit_unit("cell_length"))
	    {
	      _cell_length_ *= length_unit;
	    }
	}

      if (config_.has_key("cell_dead_time"))
	{
	  _cell_dead_time_ = config_.fetch_real("cell_dead_time");
	  if (!config_.has_explicit_unit("cell_dead_time"))
	    {
	      _cell_dead_time_ *= time_unit;
	    }
	}

      if (config_.has_key("tcut"))
	{
	  _tcut_ = config_.fetch_real("tcut");
	  if (!config_.has_explicit_unit("tcut"))
	    {
	      _tcut_ *= time_unit;
	    }
	}

      if (config_.has_key("sigma_anode_time"))
	{
	  _sigma_anode_time_ = config_.fetch_real("sigma_anode_time");
	  if (!config_.has_explicit_unit("sigma_anode_time"))
	    {
	      _sigma_anode_time_ *= time_unit;
	    }
	}

      if (config_.has_key("sigma_cathode_time"))
	{
	  _sigma_cathode_time_ = config_.fetch_real("sigma_cathode_time");
	  if (!config_.has_explicit_unit("sigma_cathode_time"))
	    {
	      _sigma_cathode_time_ *= time_unit;
	    }
	}

      if (config_.has_key("base_anode_efficiency"))
	{
	  _base_anode_efficiency_ = config_.fetch_real("base_anode_efficiency");
	}

      if (config_.has_key("base_cathode_efficiency"))
	{
	  _base_cathode_efficiency_ = config_.fetch_real("base_cathode_efficiency");
	}

      if (config_.has_key("plasma_longitudinal_speed"))
	{
	  _plasma_longitudinal_speed_ = config_.fetch_real("plasma_longitudinal_speed");
	  if (!config_.has_explicit_unit("plasma_longitudinal_speed"))
	    {
	      _plasma_longitudinal_speed_ *= drift_speed_unit;
	    }
	}

      if (config_.has_key("sigma_plasma_longitudinal_speed"))
	{
	  _sigma_plasma_longitudinal_speed_ = config_.fetch_real("sigma_plasma_longitudinal_speed");
	  if (!config_.has_explicit_unit("sigma_plasma_longitudinal_speed"))
	    {
	      _sigma_plasma_longitudinal_speed_ *= drift_speed_unit;
	    }
	}

      if (config_.has_key("sigma_z"))
	{
	  _sigma_z_ = config_.fetch_real("sigma_z");
	  if (!config_.has_explicit_unit("sigma_z"))
	    {
	      _sigma_z_ *= length_unit;
	    }
	}

      if (config_.has_key("sigma_z_missing_cathode"))
	{
	  _sigma_z_missing_cathode_ = config_.fetch_real("sigma_z_missing_cathode");
	  if (!config_.has_explicit_unit("sigma_z_missing_cathode"))
	    {
	      _sigma_z_missing_cathode_ *= length_unit;
	    }
	}

      if (config_.has_key("sigma_r_a"))
	{
	  _sigma_r_a_ = config_.fetch_real("sigma_r_a");
	  if (!config_.has_explicit_unit("sigma_r_a"))
	    {
	      _sigma_r_a_ *= length_unit;
	    }
	}

      if (config_.has_key("sigma_r_b"))
	{
	  _sigma_r_b_ = config_.fetch_real("sigma_r_b");
	}

      if (config_.has_key("sigma_r_r0"))
	{
	  _sigma_r_r0_ = config_.fetch_real("sigma_r_r0");
	  if (!config_.has_explicit_unit("sigma_r_r0"))
	    {
	      _sigma_r_r0_ *= length_unit;
	    }
	}

      DT_THROW_IF(_tcut_ < 8 * time_unit,
		  std::range_error,
		  "Cut drift time is too short (" << _tcut_ / time_unit << " us < 8 us) !");

      const double r_cell = 0.5 * _cell_diameter_;
      _r0_ = r_cell;
      _rdiag_ = r_cell * sqrt(2.0);

      // Associated t0 to r0 = to 4 us :
      _t0_ = 4 * time_unit;

      // Construct the T<->R tabulated function :
      const double step_drift_time = 0.1 * time_unit;
      for (double drift_time = 0.0 * time_unit; drift_time < (_tcut_ + 0.5 * step_drift_time); drift_time += step_drift_time) {
	const double drift_radius = base_t_2_r(drift_time);
	_base_rt_.add_point(drift_radius, drift_time, false);
      }
      _base_rt_.lock_table("linear");

      _initialized_ = true;
      return;
    }

    void geiger_analogic_regime::_init_defaults_()
    {
      // Default cell size:
      _cell_diameter_ = 44. * CLHEP::mm;
      _cell_length_ = 2900. * CLHEP::mm;

      // Default cell dead time:
      _cell_dead_time_ = 1 * CLHEP::ms;

      // Default TDC electronics resolution:
      _sigma_anode_time_ = 12.5 * CLHEP::ns;
      _sigma_cathode_time_ = 100.0 * CLHEP::ns;

      // Default resolution parameters (see I.Nasteva's work):
      _sigma_z_ = 1.0 * CLHEP::cm;
      _sigma_z_missing_cathode_ = 5.0 * CLHEP::cm;
      _sigma_r_a_ = 0.425 * CLHEP::mm;
      _sigma_r_b_ = 0.0083;  // dimensionless
      _sigma_r_r0_ = 12.25 * CLHEP::mm;

      _base_anode_efficiency_ = 1.0;
      _base_cathode_efficiency_ = 1.0;
      _plasma_longitudinal_speed_ = 5.0 * CLHEP::cm / CLHEP::microsecond;
      _sigma_plasma_longitudinal_speed_ = 0.5 * CLHEP::cm / CLHEP::microsecond;

      // Reset internals:
      _tcut_ = 13. * CLHEP::microsecond;
      datatools::invalidate(_t0_);
      datatools::invalidate(_r0_);
      datatools::invalidate(_rdiag_);
      _base_rt_.reset();

      return;
    }

    geiger_analogic_regime::geiger_analogic_regime()
    {
      _initialized_ = false;
      _init_defaults_();
      return;
    }

    geiger_analogic_regime::~geiger_analogic_regime()
    {
      return;
    }

    double geiger_analogic_regime::get_cell_diameter() const
    {
      return _cell_diameter_;
    }

    double geiger_analogic_regime::get_cell_radius() const
    {
      return 0.5 * _cell_diameter_;
    }

    double geiger_analogic_regime::get_cell_length() const
    {
      return _cell_length_;
    }

    double geiger_analogic_regime::get_cell_dead_time() const
    {
      return _cell_dead_time_;
    }

    double geiger_analogic_regime::get_sigma_anode_time(double /* anode_time_ */) const
    {
      return _sigma_anode_time_;
    }

    double geiger_analogic_regime::get_sigma_cathode_time() const
    {
      return _sigma_cathode_time_;
    }

    double geiger_analogic_regime::get_t0() const
    {
      return _t0_;
    }

    double geiger_analogic_regime::get_tcut() const
    {
      return _tcut_;
    }

    double geiger_analogic_regime::get_r0() const
    {
      return _r0_;
    }

    double geiger_analogic_regime::get_rdiag() const
    {
      return _rdiag_;
    }

    double geiger_analogic_regime::get_base_anode_efficiency() const
    {
      return _base_anode_efficiency_;
    }

    double geiger_analogic_regime::get_base_cathode_efficiency() const
    {
      return _base_cathode_efficiency_;
    }

    /** Value computed from I.Nasteva's plot in DocDB #843:
     *  Fit obtain from:
     *   shell> cd <sncore source dir>/documentation/physics/geiger/anode_efficiency.gp
     *   shell> gnuplot anode_efficiency.gp
     */
    double geiger_analogic_regime::get_anode_efficiency(double r_, int mode_) const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");
      const double max_eff = _base_anode_efficiency_;
      if (mode_ == 0)
	{
	  // Window mode
	  if (r_ <= _r0_) return max_eff;
	  else return 0.0;
	}

      if (mode_ == 1)
	{
	  // Sigmoid function representing the unefficiency outside rcell
	  // Parameters can be changed if it's needed
	  const double A1 = _r0_ + 1 * CLHEP::mm; // Sigmoid centre
	  const double B1 = 3 * CLHEP::mm;        // Sigmoid width

	  if (r_ <= _r0_ - B1) return max_eff;
	  else if (r_ > _r0_ - B1 && r_ < _rdiag_) return  (max_eff - (max_eff / (1 +  exp(-B1 * (r_ - A1)))));
	  else  return 0.0;
	}
    }

    double geiger_analogic_regime::get_cathode_efficiency() const
    {
      return _base_cathode_efficiency_;
    }

    double geiger_analogic_regime::get_plasma_longitudinal_speed() const
    {
      return _plasma_longitudinal_speed_;
    }

    double geiger_analogic_regime::get_sigma_plasma_longitudinal_speed() const
    {
      return _sigma_plasma_longitudinal_speed_;
    }

    mygsl::tabulated_function & geiger_analogic_regime::grab_base_rt()
    {
      return _base_rt_;
    }

    /** Value computed from I.Nasteva's plot in DocDB #843
     *  Fit obtain from:
     *   shell> cd <sncore source dir>/documentation/physics/geiger/cathode_efficiency.gp
     *   shell> gnuplot cathode_efficiency.gp
     */
    double geiger_analogic_regime::get_sigma_z(double /* z_ */, size_t missing_cathode_) const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");
      if (missing_cathode_ == 0) return _sigma_z_;
      if (missing_cathode_ == 1) return _sigma_z_missing_cathode_;
      return 0.5 * _cell_length_;
    }

    /** Value computed from I.Nasteva's plot in DocDB #843:
     *  Fit obtained from:
     *   shell> cd <sncore source dir>/documentation/physics/geiger/drift_time_calibration.gp
     *   shell> gnuplot drift_time_calibration.gp
     */
    double geiger_analogic_regime::get_sigma_r(double r_) const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");
      const double a = _sigma_r_a_;
      const double b = _sigma_r_b_;
      const double r0 = _sigma_r_r0_;
      const double sr = a * (1.0 + b * std::pow((r_ - r0) / CLHEP::mm, 2));
      return sr * CLHEP::mm;
    }

    double geiger_analogic_regime::base_t_2_r(double time_) const
    {
      DT_THROW_IF(time_ < 0.0, std::range_error, "Invalid drift time !");
      /* Fit obtained from:
       *   shell> cd <sncore source dir>/documentation/physics/geiger/drift_time_calibration.gp
       *   shell> gnuplot drift_time_calibration.gp
       */

      // Parameters and function for t [0:4] us :
      const double A1 = 0.561467153108633;
      const double B1 = 0.580448313540993;
      const double C1 = 1.69887483468611;

      const double t_usec = time_ / CLHEP::microsecond;
      const double ut = 10. * t_usec;
      double r = 0;

      if (time_ > 0 && time_ <= _t0_)
	{
	  r = A1 * ut / (std::pow(ut, B1) + C1);
	}
      else if (time_ > _t0_ && time_ <= _tcut_)
	{
	  // If time is over 4 us (== to over rcell), use an other function :
	  // Linear approximation [t >= _t0_ : t < _tcut_]
	  const double A2 = (_rdiag_ - _r0_) / (_tcut_ - _t0_) * 100;
	  const double B2 = _r0_ / CLHEP::cm;
	  r = A2 * ((time_ - _t0_) / CLHEP::microsecond) + B2;
	}
      else r = 0;

      r *= CLHEP::cm;

      return r;
    }

    double geiger_analogic_regime::compute_drift_time_from_drift_distance(double drift_distance_) const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");
      DT_THROW_IF(drift_distance_ < 0.0, std::range_error, "Invalid drift distance !");
      datatools::logger::priority local_priority = datatools::logger::PRIO_WARNING;

      double drift_time;
      datatools::invalidate(drift_time);

      const double rcut = _base_rt_.x_max();
      if (drift_distance_ <= rcut)
	{
	  drift_time = _base_rt_(drift_distance_);
	}
      else datatools::invalidate(drift_time);

      DT_LOG_TRACE(local_priority, "drift_time = " << drift_time);

      return drift_time;
    }

    void geiger_analogic_regime::tree_dump(std::ostream & out_,
					   const std::string & title_,
					   const std::string & indent_,
					   bool inherit_) const
    {
      std::string indent;
      if (!indent_.empty())
	{
	  indent = indent_;
	}
      if (!title_.empty())
	{
	  out_ << indent << title_ << std::endl;
	}
      out_ << indent << datatools::i_tree_dumpable::tag
	   << "Initialized   = " << is_initialized() << std::endl;
      out_ << indent << datatools::i_tree_dumpable::tag
	   << "Cell diameter = " << _cell_diameter_ / CLHEP::mm << " mm" << std::endl;
      out_ << indent << datatools::i_tree_dumpable::tag
	   << "Cell length   = " << _cell_length_ / CLHEP::cm << " cm" << std::endl;
      out_ << indent << datatools::i_tree_dumpable::tag
	   << "Cell dead time   = " << _cell_dead_time_ / CLHEP::ms << " ms" << std::endl;
      out_ << indent << datatools::i_tree_dumpable::tag << "Sigma z       = " << _sigma_z_ / CLHEP::mm
	   << " mm" << std::endl;
      out_ << indent << datatools::i_tree_dumpable::tag << "Sigma r/a     = " << _sigma_r_a_ / CLHEP::mm
	   << " mm" << std::endl;
      out_ << indent << datatools::i_tree_dumpable::tag << "Sigma r/b     = " << _sigma_r_b_
	   << std::endl;
      out_ << indent << datatools::i_tree_dumpable::tag
	   << "Sigma r/r0    = " << _sigma_r_r0_ / CLHEP::mm << " mm" << std::endl;
      out_ << indent << datatools::i_tree_dumpable::tag << "t0            = " << _t0_ / CLHEP::ns
	   << " ns" << std::endl;
      out_ << indent << datatools::i_tree_dumpable::tag << "tcut          = " << _tcut_ / CLHEP::ns
	   << " ns" << std::endl;
      out_ << indent << datatools::i_tree_dumpable::tag << "r0            = " << _r0_ / CLHEP::mm
	   << " mm" << std::endl;
      out_ << indent << datatools::i_tree_dumpable::inherit_tag(inherit_)
	   << "rdiag         = " << _rdiag_ / CLHEP::mm << " mm" << std::endl;
      return;
    }

  }  // end of namespace asb

}  // end of namespace snemo
