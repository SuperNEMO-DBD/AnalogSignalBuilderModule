// geiger_analogic_regime.h
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

#ifndef FALAISE_ASB_PLUGIN_SNEMO_ASB_GEIGER_ANALOGIC_REGIME_H
#define FALAISE_ASB_PLUGIN_SNEMO_ASB_GEIGER_ANALOGIC_REGIME_H 1

// Standard library:
#include <iostream>
#include <string>

// Third party:
// - Bayeux/datatools
#include <datatools/i_tree_dump.h>
// - Bayeux/mygsl:
#include <mygsl/tabulated_function.h>

namespace datatools
{
  class properties;
}

namespace snemo {

  namespace asb {

    /// \brief Modelling of the Geiger regime of the SuperNEMO drift cell
    class geiger_analogic_regime : public datatools::i_tree_dumpable
    {

    public:

      /// Return the diameter of the cell
      double get_cell_diameter() const;

      /// Return the radius of the cell
      double get_cell_radius() const;

      /// Return the length of the cell
      double get_cell_length() const;

      /// Return the dead time of the cell
      double get_cell_dead_time() const;

      /// Return the error on anode drift time
      double get_sigma_anode_time(double anode_time_) const;

      /// Return the error on cathode drift time
      double get_sigma_cathode_time() const;

      /// Return the reference time
      double get_t0() const;

      /// Return the maximum drift time to be calibrated
      double get_tcut() const;

      /// Radius of a cell
      double get_r0() const;

      /// Maximum sensitive distance from the anode wire
      double get_rdiag() const;

      /// Return the base anode efficiency
      double get_base_anode_efficiency() const;

      /// Return the base cathode efficiency
      double get_base_cathode_efficiency() const;

      /// Return the anode efficiency
      double get_anode_efficiency(double r_, int mode_) const;

      /// Return the cathode efficiency
      double get_cathode_efficiency() const;

      /// Return the plasma longitudinal speed
      double get_plasma_longitudinal_speed() const;

      /// Return the error on the plasma longitudinal speed
      double get_sigma_plasma_longitudinal_speed() const;

      /// Return the tabulated radius/time calibration function
      mygsl::tabulated_function & grab_base_rt();

      /// Check initialization flag
      bool is_initialized() const;

      /// Default constructor
      geiger_analogic_regime();

      /// Destructor
      virtual ~geiger_analogic_regime();

      /// Initialization from parameters
      void initialize(const datatools::properties & config_);

      /// Reset
      void reset();

      /// Compute the drift time from the drift distance of a Geiger hit
      double compute_drift_time_from_drift_distance(double drift_distance_) const;

      /// Compute the drift radius from the drift time
      double base_t_2_r(double time_) const;

      /// Return the error on longitudinal position
      double get_sigma_z(double z_, size_t missing_cathodes_ = 0) const;

      /// Return the error on the drift distance
      double get_sigma_r(double r_) const;;

      /// Smart print
      virtual void tree_dump(std::ostream & a_out = std::clog,
			     const std::string & a_title = "",
			     const std::string & a_indent = "",
			     bool a_inherit = false) const;

    private:
      bool _initialized_;  //!< Initalization flag

      double _cell_diameter_;            //!< Fiducial drift diameter of a cell
      double _cell_length_;              //!< Fiducial drift length of a cell
      double _cell_dead_time_;           //!< Geiger cell dead time
      double _sigma_anode_time_;         //!< Anode TDC resolution
      double _sigma_cathode_time_;       //!< Cathode TDC resolution
      double _sigma_z_;                  //!< Longitudinal resolution
      double _sigma_z_missing_cathode_;  //!< Longitudinal resolution for one missing cathode

      // parameters for transverse resolution:
      double _sigma_r_a_;   //!< Parameters 0 for transverse resolution
      double _sigma_r_b_;   //!< Parameters 1 for transverse resolution
      double _sigma_r_r0_;  //!< Parameters 2 for transverse resolution

      double _base_anode_efficiency_;            //!< Base anode efficiency
      double _base_cathode_efficiency_;          //!< Base cathode efficiency
      double _plasma_longitudinal_speed_;        //!< Plasma longitudinal speed
      double _sigma_plasma_longitudinal_speed_;  //!< Error on plasma longitudinal speed

      // internals:
      mygsl::tabulated_function _base_rt_;   //!< Tabulated function for drift time<->drift radius association
      double _t0_;     //!< Time (not documented yet)
      double _r0_;     //!< Radius (not documented yet)
      double _rdiag_;  //!< Cut on drift radius (not documented yet)
      double _tcut_;   //!< Cut on drift time (not documented yet)

    private:
      /// Set default values for attributes
      void _init_defaults_();

    };

  } // end of namespace asb

} // end of namespace snemo

#endif // FALAISE_ASB_PLUGIN_SNEMO_ASB_GEIGER_ANALOGIC_REGIME_H
