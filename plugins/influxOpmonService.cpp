// * This is part of the DUNE DAQ Application Framework, copyright 2020.
// * Licensing/copyright details are in the COPYING file that you should have received with this code.

#include "JsonInfluxConverter.hpp"

#include "cpr/cpr.h"
#include "opmonlib/OpmonService.hpp"
#include <array>
#include <cstdio>
#include <curl/curl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace dunedaq { // namespace dunedaq

ERS_DECLARE_ISSUE(influxopmon, cannot_post_to_DB, "Cannot post to Influx DB " << error, ((std::string)error))

ERS_DECLARE_ISSUE(influxopmon,
                  wrong_URI,
                  "Incorrect URI, use influxdb://<host>:<port>/<endpoint>?db=<mydb> instead of : " << uri,
                  ((std::string)uri))
} // namespace dunedaq

namespace dunedaq::influxopmon { // namespace dunedaq

class influxOpmonService : public dunedaq::opmonlib::OpmonService
{
public:
  explicit influxOpmonService(std::string uri)
    : dunedaq::opmonlib::OpmonService(uri)
  {
    // Regex rescription:
    //"([a-zA-Z]+):\/\/([^:\/?#\s]+)+(?::(\d+))?(\/[^?#\s]+)?(?:\?(?:db=([^?#\s]+)))"
    //* 1st Capturing Group `([a-zA-Z])`: Matches protocol
    //* 2nd Capturing Group `([^:\/?#\s])+`: Matches hostname
    //* 3rd Capturing Group `(\d)`: Matches port, optional
    //* 4th Capturing Group `(\/[^?#\s])?`: Matches endpoint/path
    //* 5th Capturing Group `([^#\s])`: Matches dbname

    std::regex uri_re(R"(([a-zA-Z]+):\/\/([^:\/?#\s]+)+(?::(\d+))?(\/[^?#\s]+)?(?:\?(?:db=([^?#\s]+))))");

    std::smatch uri_match;
    if (!std::regex_match(uri, uri_match, uri_re)) {
      ers::fatal(wrong_URI(ERS_HERE, "Invalid URI syntax: " + uri));
    }

    m_host = uri_match[2];
    m_port = (!uri_match[3].str().empty() ? uri_match[3] : std::string("8086"));
    m_path = uri_match[4];
    m_dbname = uri_match[5];
  }

  void publish(nlohmann::json j) override
  {
    m_json_converter.set_inserts_vector(j);
    m_inserts = m_json_converter.get_inserts_vector();

    m_query = "";

    for (const auto& insert : m_inserts) {
      m_query = m_query + insert + "\n";
    }

    execution_command(m_host + ":" + m_port + m_path + "?db=" + m_dbname, m_query);
  }

protected:
  typedef OpmonService inherited;

private:
  void execution_command(const std::string& adress, const std::string& cmd)
  {

    cpr::Response response = cpr::Post(cpr::Url{ adress }, cpr::Body{ cmd });
    // std::cout << adress << std::endl;
    // std::cout << cmd << std::endl;

    if (response.status_code >= 400) {
      ers::error(cannot_post_to_DB(ERS_HERE, "Error [" + std::to_string(response.status_code) + "] making request"));
    } else if (response.status_code == 0) {
      ers::error(cannot_post_to_DB(ERS_HERE, "Query returned 0"));
    }
  }

  std::string m_host;
  std::string m_port;
  std::string m_path;
  std::string m_dbname;

  std::vector<std::string> m_inserts;

  std::string m_query;
  const char* m_char_pointer;
  influxopmon::JsonConverter m_json_converter;
};

} // namespace dunedaq::influxopmon

extern "C"
{
  std::shared_ptr<dunedaq::opmonlib::OpmonService> make(std::string service)
  { // namespace dunedaq::influxopmon
    return std::shared_ptr<dunedaq::opmonlib::OpmonService>(new dunedaq::influxopmon::influxOpmonService(service));
  }
}
