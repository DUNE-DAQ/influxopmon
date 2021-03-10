#ifndef OPMONLIB_INCLUDE_OPMONLIB_OPMONSERVICE_HPP_
#define OPMONLIB_INCLUDE_OPMONLIB_OPMONSERVICE_HPP_

#include <iostream>

#include <cetlib/BasicPluginFactory.h>
#include <cetlib/compiler_macros.h>
#include <nlohmann/json.hpp>

#include "logging/Logging.hpp"
#include "Issues.hpp"

#ifndef EXTERN_C_FUNC_DECLARE_START
#define EXTERN_C_FUNC_DECLARE_START                                                                                    \
  extern "C"                                                                                                           \
{
#endif

  /**
   * @brief Declare the function that will be called by the plugin loader
   * @param klass Class to be defined as a Operational Monitoring Service
   */

#define DEFINE_DUNE_OPMON_SERVICE(klass)                                                   \
  EXTERN_C_FUNC_DECLARE_START                                                              \
  std::unique_ptr<dunedaq::opmonlib::OpmonService> make()                                  \
  {                                                                                        \
    return std::unique_ptr<dunedaq::opmonlib::OpmonService>(new klass());                  \
  }                                                                                        \
} 


namespace dunedaq::opmonlib 
{

  class OpmonService 
  {
    public:
      explicit OpmonService(std::string /*service*/) {}
      ~OpmonService();
      OpmonService(const OpmonService&) = 
	delete; ///< OpmonService is not copy-constructible
      OpmonService& operator=(const OpmonService&) =
	delete; ///< OpmonService is not copy-assignable
      OpmonService(OpmonService&&) =
	delete; ///< OpmonService is not move-constructible
      OpmonService& operator=(OpmonService&&) =
	delete; ///< OpmonService is not move-assignable

      // Publish information
      virtual void publish( nlohmann::json j ) = 0;

    private:

  };

  std::shared_ptr<OpmonService>
    makeOpmonService(std::string const& service)
    {
      TLOG() << "SERVICE = " << service;
      auto sep = service.find("://");
      std::string scheme;
      if (sep == std::string::npos) { // simple path
	scheme = "stdout";
      } else { // with scheme
	scheme = service.substr(0, sep);
      }
      std::string plugin_name = scheme + "OpmonService";
      static cet::BasicPluginFactory bpf("duneOpmonService", "make");
      std::shared_ptr<OpmonService> os_ptr;
      try {
	os_ptr = bpf.makePlugin<std::shared_ptr<OpmonService>>(plugin_name, service);
      } catch (const cet::exception &cexpt) {
	throw OpmonServiceCreationFailed(ERS_HERE, service, cexpt);
      } catch (const ers::Issue &iexpt) {
	throw OpmonServiceCreationFailed(ERS_HERE, service, iexpt);
      } catch (...) {  // NOLINT JCF Jan-27-2021 violates letter of the law but not the spirit
	throw OpmonServiceCreationFailed(ERS_HERE, service, "Unknown error.");
      }
      return os_ptr;
    }


} // namespace dunedaq::opmonlib

#endif // OPMONLIB_INCLUDE_OPMONLIB_OPMONSERVICE_HPP_
