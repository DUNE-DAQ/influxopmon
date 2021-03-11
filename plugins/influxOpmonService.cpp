#include <string>
#include <nlohmann/json.hpp>
#include "opmonlib/OpmonService.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>



using json = nlohmann::json;

namespace dunedaq {

    ERS_DECLARE_ISSUE(influxopmon, CannotPostToDB,
        "Cannot post to Influx DB " << name,
        ((std::string)name))

}


namespace dunedaq::influxopmon {

    class influxOpmonService : public dunedaq::opmonlib::OpmonService
    {
    public:

        std::string exec(const char* cmd) {
            std::array<char, 128> buffer;
            std::string result;
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
            if (!pipe) {
                throw std::runtime_error("popen() failed!");
            }
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }
            return result;
        }


        explicit influxOpmonService(std::string uri) : dunedaq::opmonlib::OpmonService(uri) {
            // FIXME: Get the DB connection string from the URI.
            // The URI will be something like influx://db_host:db_port:db_name....
            uri = uri.substr(uri.find("/") + 2);
            std::string tmp;
            std::stringstream ss(uri);
            std::vector<std::string> ressource;
            while (getline(ss, tmp, ':'))
            {
                ressource.push_back(tmp);
            }

            //std::string uri = "influx://dbod-testinfluxyd.cern.ch:8095:db1:admin:admin"
            m_host = ressource[0];
            m_port = ressource[1];
            m_dbname = ressource[2];
            m_dbaccount = ressource[3];
            m_dbpassword = ressource[4];

            tagSetVector.push_back(".class_name=");
        }

        void publish(nlohmann::json j)
        {
            setInsertsVector(false, tagSetVector, timeVariableName, j);
            insertsVector = getInsertsVector();
            
            for (int i = 0; i < insertsVector.size(); i++)
            {
                std::cout << insertsVector[i];
                querry = "curl -i -XPOST 'https://" + m_host + ":" + m_port + "/write?db=" + m_dbname + "' --header 'Authorization: Token " + m_dbaccount + ":" + m_dbpassword + "'  --data-binary '" + insertsVector[i] + "'";
                charPointer = querry.c_str();
                std::cout << exec(charPointer);
            }
        }

        std::string checkDataType(std::string line)
        {
            std::string lineOriginal = line;
            line = line.substr(line.find("=") + 1);


            if ((line.find_first_not_of("0123456789") == std::string::npos) || line == "true" || line == "false")
            {
                lineOriginal;
            }
            else
            {
                lineOriginal = lineOriginal.substr(0, lineOriginal.find("=") + 1);
                lineOriginal = lineOriginal + "\""  + line + "\"";
            }

            return lineOriginal;
        }

        std::vector<std::string> jsonToInfluxFunction(bool ignoreTags, std::vector<std::string> tagSetVector, std::string timeVariableName, json jsonStream)
        {


            //flatten json, convert to string the json, then breaks the string into an array
            std::string jsonFlattenedString = jsonStream.flatten().dump();
            //remove first and last characters as they are {}
            jsonFlattenedString = jsonFlattenedString.substr(1, jsonFlattenedString.size() - 2);
            std::vector<std::string> vectorItems;
            std::stringstream data(jsonFlattenedString);
            int numOfCharacters;
            std::string applicationName;

            bool classdefined = false;

            while (std::getline(data, jsonFlattenedString, ','))
            {
                //Breaks the string to array
                jsonFlattenedString.erase(std::remove(jsonFlattenedString.begin(), jsonFlattenedString.end(), '"'), jsonFlattenedString.end());

                //Find delimiter between position and data and count its position
                std::string::size_type pos = jsonFlattenedString.find(":");
                int numOfCharacters = static_cast<int>(pos);

                //erase characters before separator
                jsonFlattenedString = jsonFlattenedString.substr(1);
                std::replace(jsonFlattenedString.begin(), jsonFlattenedString.end(), '/', '.');
                std::replace(jsonFlattenedString.begin(), jsonFlattenedString.end(), ':', '=');

                //Remove the class, saves it
                if (!classdefined) { applicationName = jsonFlattenedString.substr(0, jsonFlattenedString.find(".")); }
                jsonFlattenedString = jsonFlattenedString.substr(jsonFlattenedString.find(".") + 1);

                //add to vector
                vectorItems.push_back(jsonFlattenedString);

            }

            std::string insertCommandTag = applicationName + ",";
            std::string insertCommandField = "";
            std::string time;

            // different member versions of find in the same order as above:
            std::size_t found;

            //list of insert commands
            std::vector<std::string> vectorInserts;

            //
            bool isTag = false;

            //Shaping to InfluxDB 
            for (int i = 0; i < vectorItems.size(); i++)
            {
                //is the time variable 
                found = vectorItems[i].find(timeVariableName);
                if (found == std::string::npos)
                {
                    //if className should be ignored then do not add any tag that has this description
                    if (ignoreTags)
                    {
                        for (int j = 0; j < tagSetVector.size(); j++)
                        {
                            if (vectorItems[i].find(tagSetVector[j]) != std::string::npos)
                            {
                                isTag = true;
                            }
                        }

                        //if the tag is not in the ignore list
                        if (!isTag)
                        {
                            insertCommandField = insertCommandField + checkDataType(vectorItems[i]) + ",";
                        }

                        isTag = false;
                    }
                    else
                    {
                        //if labelled as the tag, goes in the tag variable
                        for (int j = 0; j < tagSetVector.size(); j++)
                        {
                            if (vectorItems[i].find(tagSetVector[j]) != std::string::npos)
                            {
                                isTag = true;
                            }
                        }

                        //if the tag is not in the ignore list
                        if (!isTag)
                        {
                            insertCommandField = insertCommandField + checkDataType(vectorItems[i]) + ",";
                        }
                        else
                        {
                            insertCommandTag = insertCommandTag + checkDataType(vectorItems[i]) + ",";
                        }
                        isTag = false;


                    }

                }
                else
                {
                    time = vectorItems[i].substr(vectorItems[i].find("=") + 1);
                    //remove the last character which is a ","
                    insertCommandTag = insertCommandTag.substr(0, insertCommandTag.size() - 1);
                    insertCommandField = insertCommandField.substr(0, insertCommandField.size() - 1);

                    vectorInserts.push_back(insertCommandTag + " " + insertCommandField + " " + time);
                    //std::cout << insertCommandTag + " " + insertCommandField + " " + time << "\n";

                    insertCommandTag = applicationName + ",";
                    insertCommandField = "";
                }

            }

            return vectorInserts;
        }

        void setInsertsVector(bool ignoreTags, std::vector<std::string> tagSetVector, std::string timeVariableName, nlohmann::json jsonStream)
        {
            try
            {
                insertsVector = jsonToInfluxFunction(ignoreTags, tagSetVector, timeVariableName, jsonStream);
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

    protected:
        typedef OpmonService inherited;
    private:
        std::string m_host;
        std::string m_port;
        std::string m_dbname;
        std::string m_dbaccount;
        std::string m_dbpassword;
        // FIXME: add here utility methods
        
        std::vector<std::string> tagSetVector;
        std::string timeVariableName = ".time=";
        std::vector<std::string> insertsVector;
        
        std::string querry;
        const char* charPointer;
    };

}

extern "C" {
    std::shared_ptr<dunedaq::opmonlib::OpmonService> make(std::string service) {
        return std::shared_ptr<dunedaq::opmonlib::OpmonService>(new dunedaq::influxopmon::influxOpmonService(service));
    }
}

