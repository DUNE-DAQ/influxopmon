#ifndef INFLUXOPMON_INCLUDE_INFLUXOPMON_JSONINFLUXCONVERTER_H_
#define INFLUXOPMON_INCLUDE_INFLUXOPMON_JSONINFLUXCONVERTER_H_

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <iostream>
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

    ERS_DECLARE_ISSUE(influxopmon, ErrorJSON,
        "JSON input error" << Error,
        ((std::string)Error))

    namespace influxopmon
    {
        class JsonConverter
        {
            std::vector<std::string> insertsVector;

        private:


        bool errorState = false;
            const std::string parentTag = "__parent";
            const std::string timeTag = "__time"; 
            const std::string dataTag = "__data";
            const std::string childrenTag = "__children";
            const std::string propertiesTag = "__properties";
            const std::string tagName = "source_id=";
            const std::string tags[5] = { parentTag, timeTag, dataTag, childrenTag, propertiesTag };

            int keyIndex = 0;
            std::string fieldSet = "";
            std::string measurement;
            std::string timeStamp;
            std::vector<std::string> tagSet;
            std::vector<std::string> querries;
            std::vector<std::string> hierarchy;

            std::string convertTimeToNS(std::string time)
            {
                long unsigned int stringLenNS = 19;
                while (time.size() < stringLenNS)
                {
                    time = time + "0";
                }
                return time;
            }

            void CheckKeyword(std::string inputTag)
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

            void BuildString(std::string input)
            {
                
                if (hierarchy[hierarchy.size() - 1].substr(0, 2) == "__" && input.substr(0, 2) != "__")
                {
                    CheckKeyword(hierarchy[hierarchy.size() - 1]);
                    if (hierarchy[hierarchy.size() - 1] == childrenTag)
                    {
                        tagSet.push_back("." + input);
                    }
                    else if (hierarchy[hierarchy.size() - 1] == parentTag)
                    {
                        tagSet.push_back(input);
                    }     
                    else if (hierarchy[hierarchy.size() - 1] == propertiesTag)
                    {
                        measurement = input;
                    }
                }
            }

            void BuildString(std::string key, std::string data)
            {
                if (hierarchy[hierarchy.size() - 1] == timeTag)
                {
                    timeStamp = convertTimeToNS(data);
                    std::string fullTag = "";
                    for (std::string tag : tagSet)
                    {
                        fullTag = fullTag + tag;
                    }
                    if (!errorState)
                    {
                        querries.push_back(measurement + "," + tagName + fullTag + " " + fieldSet.substr(0, fieldSet.size() - 1) + " " + timeStamp);
                    }
                    
                    fieldSet = "";
                    errorState = false;
                }
                else if (hierarchy[hierarchy.size() - 1] == dataTag)
                {
                    fieldSet = fieldSet + key + "=" + data + ",";
                }
                else
                {
                    CheckKeyword(hierarchy[hierarchy.size() - 1]);
                    ers::warning(IncorrectJSON(ERS_HERE, "Structure error"));
                }
            }
            

            void RecursiveIterateItems(const json& j)
            {
                for (auto& item : j.items())
                {
                    if (item.value().begin()->is_structured())
                    {
                        
                        BuildString(item.key());
                        hierarchy.push_back(item.key());
                        RecursiveIterateItems(item.value());
                        hierarchy.pop_back();
                        if ((hierarchy[hierarchy.size() - 1] == childrenTag || hierarchy[hierarchy.size() - 1] == parentTag) && tagSet.size() > 0)
                        {
                            tagSet.pop_back();
                        }
                    }
                    else
                    {
                        BuildString(item.key());
                        hierarchy.push_back(item.key());
                        for (auto& lastItem : item.value().items())
                        {
                            
                            if(lastItem.value().type() == json::value_t::string)
                            {
                                if (lastItem.value().dump()[0] != '"') { "\"" + lastItem.value().dump(); }
                                if (lastItem.value().dump()[lastItem.value().dump().size() - 1] != '"') { lastItem.value().dump() + "\""; }
                                BuildString(lastItem.key(), lastItem.value().dump());
                            }
                            else
                            {
                                BuildString(lastItem.key(), lastItem.value().dump());
                            }
                        }
                        hierarchy.pop_back();
                    }
                }
            }
        std::vector<std::string> jsonToInfluxFunction(json json)
            {
                hierarchy.clear();
                hierarchy.push_back("root");
                querries.clear();
                tagSet.clear();
                RecursiveIterateItems(json);
                return querries;
            }

        public:

            /**
             * Convert a nlohmann::json object to an influxDB INSERT string.
             *
             * @param   Param 1 if true, the tags are not added to the querry.
             *          Param 2 is a vector of key-words delimiting tags
             *          Param 3 is the key word delimiting the timestamp
             *          Param 4 is a string formatted flatened json object
             *
             * @return Void, to get call getInsertsVector
             */
            void setInsertsVector(json json)
            { 
                try
                {
                    insertsVector = jsonToInfluxFunction(json);
                }
                catch (const std::runtime_error& re)
                {
                    // speciffic handling for runtime_error
                    ers::error(ErrorJSON(ERS_HERE, "Runtime error: " + std::string(re.what())));
                }
                catch (const std::exception& ex)
                {
                    // extending std::exception, except
                    ers::error(ErrorJSON(ERS_HERE, "Error occurred: " + std::string(ex.what())));
                }
                catch (...)
                {
                    ers::error(ErrorJSON(ERS_HERE, "Unknown failure occurred. Possible memory corruption" ));
                }
            }
            /**
             * Get a converted vector, to set call setInsertsVector.
             *
             * @return Vector of string formated influxDB INSERT querries.
             */
            std::vector<std::string> getInsertsVector()
            {
                return insertsVector;
            }
        };
    }
}

#endif
