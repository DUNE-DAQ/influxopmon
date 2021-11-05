#include "InsertQueryListBuilder.hpp"
#include <string>
#include <iomanip>
#include "opmonlib/JSONTags.hpp"


using json = nlohmann::json;

namespace dunedaq {

using opmonlib::JSONTags;

namespace influxopmon {

// Build insert query objects
InsertQueryListBuilder::InsertQueryListBuilder( const json& j ) {

    std::vector<InsertQuery> queries;
    // Expect only 1 key '__parent'
    if ( j.size() != 2 or j.count(JSONTags::parent) != 1 or j.count(JSONTags::tags) != 1 ) {
        throw OpmonJSONValidationError(ERS_HERE, "Root key '"+std::string(JSONTags::parent)+"' not found" );
    }

    if ( j[JSONTags::parent].size() != 1) {
        throw OpmonJSONValidationError(ERS_HERE, "Expected 1 top-level entry, found " + std::to_string(j[JSONTags::parent].size()) );
    }

    // even easier with structured bindings (C++17)
    for (auto& [key, value] : j[JSONTags::tags].items()) {
      m_tags[key] = value;
    }

    this->parse_json_obj("", j[JSONTags::parent]);

}


void InsertQueryListBuilder::parse_json_obj(std::string path,
                                          const nlohmann::json& j) {
  // even easier with structured bindings (C++17)
  for (auto& [key, obj] : j.items()) {

    auto objpath = (path.size() ? path+m_separator+key : key);

    // validate meta filelds, i.e.
    if (not obj.is_object())
      throw OpmonJSONValidationError(ERS_HERE, key + " is not an object");

    // if properties are here, process them
    if (obj.count(JSONTags::properties)) {
      // Loop over property structures
      for (auto& pstruct : obj[JSONTags::properties].items()) {

        // Make sure that this is a json object
        if (not pstruct.value().is_object())
          throw OpmonJSONValidationError(ERS_HERE, pstruct.key() + " is not an object");

        // And that it contains the required fields
        if (pstruct.value().count(JSONTags::time) == 0 or pstruct.value().count(JSONTags::data) == 0)
          throw OpmonJSONValidationError(ERS_HERE, pstruct.key() + " has no " +
                                                  JSONTags::time + " or " +
                                                  JSONTags::data + " tag");

        InsertQuery iq;
        iq.type = pstruct.key();
        iq.source_id = objpath;
        iq.tags = m_tags;
        iq.time = pstruct.value().at(JSONTags::time).get_to(iq.time);
        // Loop over propery objects
        for (auto& pobj : pstruct.value().at(JSONTags::data).items()) {
          iq.fields[pobj.key()] = pobj.value().dump();
        }

        m_queries.push_back(iq);
      }
    }

    // and then go through children
    if (obj.count(JSONTags::children)) {
      // Recurse over children
      this->parse_json_obj( objpath, obj[JSONTags::children]
      );
    }
  }
}

} // namespace influxopmon
} // namespace dunedaq
