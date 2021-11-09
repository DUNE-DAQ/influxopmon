#ifndef INFLUXOPMON_SRC_INFLUXINSERTQUERIESBUILDER_HPP_
#define INFLUXOPMON_SRC_INFLUXINSERTQUERIESBUILDER_HPP_

#include <ers/Issue.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>


namespace dunedaq {

ERS_DECLARE_ISSUE(influxopmon, OpmonJSONValidationError,
                  "JSON input incorrect" << error, ((std::string)error))

namespace influxopmon {

struct InsertQuery {
  std::string type;
  std::string source_id;
  std::map<std::string, std::string> tags;
  std::map<std::string, std::string> fields;
  uint64_t time;
};

class InsertQueryListBuilder {
 public:
  inline static constexpr char m_source_id_tag[] = "source_id";
  inline static constexpr char m_separator[] = ".";

  InsertQueryListBuilder(const nlohmann::json& j);
  
  const std::vector<InsertQuery>& get() const { return m_queries; }

 private:
  void parse_json_obj(std::string path, const nlohmann::json& j);

  std::map<std::string, std::string> m_tags;
  std::vector<InsertQuery> m_queries;
};


std::ostream& operator<<(std::ostream& os,
                         const dunedaq::influxopmon::InsertQuery& iq) {
  os << iq.type
    << ","
    << InsertQueryListBuilder::m_source_id_tag << "=" << iq.source_id;

  for (auto it = iq.tags.begin(); it != iq.tags.end(); ++it) {
    os << "," 
      << it->first+"="+it->second;
  }
  os  << " ";

  if ( iq.fields.size() ) {
    auto it = iq.fields.begin();
    os << it->first+"="+it->second;

    for (it = std::next(iq.fields.begin()); it != iq.fields.end(); ++it) {
      os << "," 
        << it->first+"="+it->second;
    }
    os << " ";
  }

  os << (iq.time*1000000000); // from sec to ns
  return os;
}

}  // namespace influxopmon
}  // namespace dunedaq

#endif /* INFLUXOPMON_SRC_INFLUXINSERTQUERIESBUILDER_HPP_ */