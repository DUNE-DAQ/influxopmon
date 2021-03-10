#ifndef OPMONLIB_INCLUDE_OPMONLIB_INFOPROVIDER_HPP_
#define OPMONLIB_INCLUDE_OPMONLIB_INFOPROVIDER_HPP_


#include <iostream>
#include "opmonlib/InfoCollector.hpp"

namespace dunedaq::opmonlib {

  class InfoProvider {

    public:

      virtual void gather_stats( opmonlib::InfoCollector &ic, int level ) = 0;

    private:

  };

}


#endif
