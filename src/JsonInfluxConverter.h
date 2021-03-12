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

class JsonConverter
{
    std::vector<std::string> insertsVector;

	private: 
        std::string checkDataType(std::string line);
        std::vector<std::string> jsonToInfluxFunction(bool ignoreTags, std::vector<std::string> tagSetVector, std::string timeVariableName, std::string jsonFlattenedString);
	public:
        /**
         * Convert a nlohmann::json object to an influxDB INSERT string.
         *
         * @param   Param 1 if true, the tags are not added to the querry.
         *          Param 2 is a vector of key-words delimiting tags
         *          Param 3 is the key word delimiting the timestamp
         *          Param 4 is a nlohmann::json object
         *
         * @return Void, to get call getInsertsVector
         */
        void setInsertsVector(bool ignoreTags, std::vector<std::string> tagSetVector, std::string timeVariableName, std::string jsonFlattenedString)
        {
            try
            {
                insertsVector = jsonToInfluxFunction(ignoreTags, tagSetVector, timeVariableName, jsonFlattenedString);
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
        void HW(std::string hw)
        {
            std::cout << hw;
        }
};

#endif