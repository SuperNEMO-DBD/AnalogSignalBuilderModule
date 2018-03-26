//! \file    snemo/asb/resource.h
//! \brief   Utilities for accessing ASB Falaise plugin resource files
//
// Copyright (c) 2018 by  F. Mauger <mauger@lpccaen.in2p3.fr>
//
// This file is part of ASB Falaise plugin.
//
// ASB Falaise plugin is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ASB Falaise plugin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ASB Falaise plugin.  If not, see <http://www.gnu.org/licenses/>.

#ifndef FALAISE_ASB_PLUGIN_SNEMO_ASB_RESOURCE_H
#define FALAISE_ASB_PLUGIN_SNEMO_ASB_RESOURCE_H

// Standard Library
#include <stdexcept>
#include <string>

namespace snemo {

  namespace asb {

    //! Exception class for bad resource initialization
    class ResourceInitializationException
      : public std::runtime_error
    {
    public:
      ResourceInitializationException(const std::string & msg_)
        : std::runtime_error(msg_) {}
    };

    //! Exception class for unknown resources
    class UnknownResourceException
      : public std::runtime_error
    {
    public:
      UnknownResourceException(const std::string & msg_)
        : std::runtime_error(msg_) {}
    };

    //! Initialize the library resource paths
    //! \throw ResourceInitializationException when library cannot self locate
    void init_resources();

    //! Return URL, i.e. a path, to the  base directory where resource files are installed
    std::string get_resource_dir();

    //! Return URL, i.e. a path, to named resource
    std::string get_resource(const std::string & rname_);

  }  // namespace asb

}  // namespace snemo

#endif  // FALAISE_ASB_PLUGIN_SNEMO_ASB_RESOURCE_H

// Local Variables: --
// mode: c++ --
// c-file-style: "gnu" --
// tab-width: 2 --
// End: --
