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

namespace dunedaq::influxopmon
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
        const std::string tags[5] = { parentTag, timeTag, dataTag, childrenTag, propertiesTag };

        int keyIndex = 0;
        std::string nearestParent = "";
        std::string fieldSet = "";
        std::string measurement;
        std::string timeStamp;
        std::vector<std::string> tagSet;
        std::vector<std::string> querries;

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
                std::cout << "Uncorrect tag " << inputTag << ", querry dumped." << '\n';
                errorState = true;
            }
        }

        void BuildString(std::string input)
        {
            
            if (nearestParent.substr(0, 2) == "__" && input.substr(0, 2) != "__")
            {
                CheckKeyword(nearestParent);
                if (nearestParent == childrenTag)
                {
                    tagSet.push_back("." + input);
                }
                else if (nearestParent == parentTag)
                {
                    tagSet.push_back(input);
                }     
                else
                {
                    tagSet.push_back("");
                } 

                if (nearestParent == propertiesTag)
                {
                    measurement = input;
                }
            }
            if (input != "")
            {
                nearestParent = input;
            }
            std::cout << "level " << std::to_string(keyIndex) << " input: " << input << '\n';
        }

        void BuildString(std::string key, std::string data)
        {
            if (nearestParent == timeTag)
            {
                timeStamp = convertTimeToNS(data);
                std::string fullTag = "";
                for (std::string tag : tagSet)
                {
                    fullTag = fullTag + tag;
                }
                if (!errorState)
                {
                    querries.push_back(measurement + "," + "source_id=" + fullTag + " " + fieldSet.substr(0, fieldSet.size() - 1) + " " + timeStamp);
                }
                nearestParent = "";
                fieldSet = "";
                errorState = false;
            }
            else if (nearestParent == dataTag)
            {
                fieldSet = fieldSet + key + "=" + data + ",";
                std::cout << "key " << key << " data: " << data << '\n';
            }
            else
            {
                CheckKeyword(nearestParent);
                std::cout << "Structure error" + '\n';
            }
        }
        

        void RecursiveIterateItems(const json& j)
        {
            for (auto& item : j.items())
            {
                if (item.value().begin()->is_structured())
                {
                    BuildString(item.key());
                    keyIndex++;
                    RecursiveIterateItems(item.value());
                    keyIndex--;
                    tagSet.pop_back();
                }
                else
                {   

                    BuildString(item.key());
                    tagSet.push_back("");
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
                }
            }
        }

        std::vector<std::string> jsonToInfluxFunction(json json)
        {
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
                std::cerr << "Runtime error: " << re.what() << std::endl;
            }
            catch (const std::exception& ex)
            {
                // extending std::exception, except
                std::cerr << "Error occurred: " << ex.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
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

#endif
