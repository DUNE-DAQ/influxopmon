#ifndef INFLUXOPMON_SRC_JSONINFLUXCONVERTER_HPP_
#define INFLUXOPMON_SRC_JSONINFLUXCONVERTER_HPP_

// * This is part of the DUNE DAQ Application Framework, copyright 2020.
// * Licensing/copyright details are in the COPYING file that you should have received with this code.


#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using json = nlohmann::json;

namespace dunedaq
{
    ERS_DECLARE_ISSUE(influxopmon, IncorrectJSON,
        "JSON input incorrect" << Warning,
        ((std::string)Warning))

    ERS_DECLARE_ISSUE(influxopmon, error_JSON,
        "JSON input error" << Error,
        ((std::string)Error))

    namespace influxopmon
    {
        class JsonConverter
        {
            std::vector<std::string> inserts_vector;


        public:

            /**
             * Convert a nlohmann::json object to an influxDB INSERT string.
             *
             * @param   Param 1 if true, the tags are not added to the querry.
             *          Param 2 is a vector of key-words delimiting tags
             *          Param 3 is the key word delimiting the timestamp
             *          Param 4 is a string formatted flatened json object
             *
             * @return Void, to get call get_inserts_vector
             */
            void set_inserts_vector(json json)
            { 
                try
                {
                    inserts_vector = json_to_influx(json);
                }
                catch (const std::runtime_error& re)
                {
                    // speciffic handling for runtime_error
                    ers::error(error_JSON(ERS_HERE, "Runtime error: " + std::string(re.what())));
                }
                catch (const std::exception& ex)
                {
                    // extending std::exception, except
                    ers::error(error_JSON(ERS_HERE, "Error occurred: " + std::string(ex.what())));
                }
                catch (...)
                {
                    ers::error(error_JSON(ERS_HERE, "Unknown failure occurred. Possible memory corruption" ));
                }
            }
            /**
             * Get a converted vector, to set call set_inserts_vector.
             *
             * @return Vector of string formated influxDB INSERT querries.
             */
            std::vector<std::string> get_inserts_vector()
            {
                return inserts_vector;
            }
        

        private:


        bool errorState = false;
            const std::string parent_tag = "__parent";
            const std::string time_tag = "__time"; 
            const std::string data_tag = "__data";
            const std::string children_tag = "__children";
            const std::string properties_tag = "__properties";
            const std::string tag_tag = "source_id=";
            const std::string tags[5] = { parent_tag, time_tag, data_tag, children_tag, properties_tag };

            int keyIndex = 0;
            std::string field_set = "";
            std::string measurement;
            std::string time_stamp;
            std::vector<std::string> tag_set;
            std::vector<std::string> querries;
            std::vector<std::string> hierarchy;

            std::string convert_time_to_NS(std::string time)
            {
                while (time.size() < 19)
                {
                    time = time + "0";
                }
                return time;
            }

            void check_keyword(std::string inputTag)
            {
                bool validTag = false;
                for (std::string tag : tags)
                {
                    if (inputTag == tag)
                    {
                        validTag = true;
                        break;
                    }
                }
                if (!validTag)
                {
                    ers::warning(IncorrectJSON(ERS_HERE, "Uncorrect tag " + inputTag + ", querry dumped, integrity might be compromised."));
                }
            }

            void build_string(std::string input)
            {
                
                if (hierarchy[hierarchy.size() - 1].substr(0, 2) == "__" && input.substr(0, 2) != "__")
                {
                    check_keyword(hierarchy[hierarchy.size() - 1]);
                    if (hierarchy[hierarchy.size() - 1] == children_tag)
                    {
                        tag_set.push_back("." + input);
                    }
                    else if (hierarchy[hierarchy.size() - 1] == parent_tag)
                    {
                        tag_set.push_back(input);
                    }     
                    else if (hierarchy[hierarchy.size() - 1] == properties_tag)
                    {
                        measurement = input;
                    }
                }
            }

            void build_string(std::string key, std::string data)
            {
                if (hierarchy[hierarchy.size() - 1] == time_tag)
                {
                    time_stamp = convert_time_to_NS(data);
                    std::string fullTag = "";
                    for (std::string tag : tag_set)
                    {
                        fullTag = fullTag + tag;
                    }
                    if (!errorState)
                    {
                        querries.push_back(measurement + "," + tag_tag + fullTag + " " + field_set.substr(0, field_set.size() - 1) + " " + time_stamp);
                    }
                    
                    field_set = "";
                    errorState = false;
                }
                else if (hierarchy[hierarchy.size() - 1] == data_tag)
                {
                    field_set = field_set + key + "=" + data + ",";
                }
                else
                {
                    check_keyword(hierarchy[hierarchy.size() - 1]);
                    ers::warning(IncorrectJSON(ERS_HERE, "Structure error"));
                }
            }
            

            void recursive_iterate_items(const json& j)
            {
                for (auto& item : j.items())
                {
                    if (item.value().begin()->is_structured())
                    {
                        
                        build_string(item.key());
                        hierarchy.push_back(item.key());
                        recursive_iterate_items(item.value());
                        hierarchy.pop_back();
                        if ((hierarchy[hierarchy.size() - 1] == children_tag || hierarchy[hierarchy.size() - 1] == parent_tag) && tag_set.size() > 0)
                        {
                            tag_set.pop_back();
                        }
                    }
                    else
                    {
                        build_string(item.key());
                        hierarchy.push_back(item.key());
                        for (auto& lastItem : item.value().items())
                        {
                            
                            if(lastItem.value().type() == json::value_t::string)
                            {
                                if (lastItem.value().dump()[0] != '"') { "\"" + lastItem.value().dump(); }
                                if (lastItem.value().dump()[lastItem.value().dump().size() - 1] != '"') { lastItem.value().dump() + "\""; }
                                build_string(lastItem.key(), lastItem.value().dump());
                            }
                            else
                            {
                                build_string(lastItem.key(), lastItem.value().dump());
                            }
                        }
                        hierarchy.pop_back();
                    }
                }
            }
        std::vector<std::string> json_to_influx(json json)
            {
                hierarchy.clear();
                hierarchy.push_back("root");
                querries.clear();
                tag_set.clear();
                recursive_iterate_items(json);
                return querries;
            }
        };
    } // namespace influxopmon
} // namespace dunedaq

#endif // INFLUXOPMON_SRC_JSONINFLUXCONVERTER_HPP_
