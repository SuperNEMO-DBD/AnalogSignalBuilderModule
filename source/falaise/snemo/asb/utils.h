// snemo/asb/utils.h
// Author(s): F. Mauger <mauger@lpccaen.in2p3.fr>
// Date: 2017-10-14

#ifndef FALAISE_ASB_PLUGIN_SNEMO_ASB_UTILS_H
#define FALAISE_ASB_PLUGIN_SNEMO_ASB_UTILS_H

// Standard libraries :
#include <string>

// - Bayeux/mctools:
#include <mctools/signal/signal_shape_builder.h>
#include <mctools/signal/base_signal.h>

namespace snemo {

  namespace asb {

    void build_signal_key(const int id_, std::string & name_);

    void build_private_signal_name(const int id_, std::string & name_);

    bool is_private_signal_name(const std::string & name_);

    void build_shape(mctools::signal::signal_shape_builder & builder_,
         const mctools::signal::base_signal & signal_);

  } // end of namespace asb

} // end of namespace snemo

#endif // FALAISE_ASB_PLUGIN_SNEMO_ASB_UTILS_H

// Local Variables: --
// mode: c++ --
// c-file-style: "gnu" --
// tab-width: 2 --
// End: --
