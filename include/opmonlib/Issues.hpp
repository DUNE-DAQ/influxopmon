#ifndef OPMONLIB_INCLUDE_OPMONLIB_ISSUES_HPP_ 
#define OPMONLIB_INCLUDE_OPMONLIB_ISSUES_HPP_ 

#include <ers/Issue.hpp>
#include <string>

namespace dunedaq 
{

  ERS_DECLARE_ISSUE(opmonlib, OpmonServiceCreationFailed,
      "OpmonServiceCreationFailed: " << error,
      ((std::string)error))

} //namespace dunedaq

#endif // OPMONLIB_INCLUDE_OPMONLIV_ISSUES_HPP
