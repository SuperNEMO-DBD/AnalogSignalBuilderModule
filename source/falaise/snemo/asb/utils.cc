// utils.cc - Implementation of Falaise ASB plugin utilities
//
// Copyright (c) 2017 F. Mauger <mauger@lpccaen.in2p3.fr>
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
#include <snemo/asb/utils.h>

// Standard library:
#include <sstream>
#include <stdexcept>

// Third party:
// - Boost:
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
// - Bayeux/datatools :
#include <bayeux/datatools/properties.h>
#include <bayeux/datatools/exception.h>

namespace snemo {

  namespace asb {

    void build_signal_name(const int id_,
                           std::string & name_)
    {
      name_ = "shape." + boost::lexical_cast<std::string>(id_);
      return;
    }

    void build_private_signal_name(const int id_,
                                   std::string & name_)
    {
      name_ = "__shape." + boost::lexical_cast<std::string>(id_);
      return;
    }

    bool is_private_signal_name(const std::string & name_)
    {
      return boost::starts_with(name_, "__shape.");
    }

    void build_shape(mctools::signal::signal_shape_builder & builder_,
                     const mctools::signal::base_signal & signal_)
    {
      std::string signal_key;
      build_signal_name(signal_.get_hit_id(), signal_key);
      if (signal_.has_private_shapes_config()) {
        const datatools::multi_properties & priv_mp = signal_.get_private_shapes_config();
        const datatools::multi_properties::entries_ordered_col_type & oentries
          = priv_mp.ordered_entries();
        for (const auto * entry_ptr : oentries) {
          std::string priv_signal_key = entry_ptr->get_key();
          const std::string & priv_signal_meta = entry_ptr->get_meta();
          const datatools::properties & priv_signal_params = entry_ptr->get_properties();
          builder_.create_signal_shape(priv_signal_key,
                                       priv_signal_meta,
                                       priv_signal_params);
        }
      }
      datatools::properties signal_shape_params;
      signal_.get_auxiliaries().export_and_rename_starting_with(signal_shape_params,
                                                                mctools::signal::base_signal::shape_parameter_prefix(),
                                                                "");
      builder_.create_signal_shape(signal_key,
                                   signal_.get_shape_type_id(),
                                   signal_shape_params);
      return;
    }



  } // end of namespace asb

} // end of namespace snemo
