#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>


#include "opmonlib/OpmonService.hpp"
#include "JsonInfluxConverter.h"
#include "influxdb.hpp"

namespace dunedaq {

    ERS_DECLARE_ISSUE(influxopmon, CannotPostToDB,
        "Cannot post to Influx DB " << name,
        ((std::string)name))

}


namespace dunedaq::influxopmon {

    class influxOpmonService : public dunedaq::opmonlib::OpmonService
    {
    public:
        explicit influxOpmonService(std::string uri) : dunedaq::opmonlib::OpmonService(uri) {
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
            m_port = stoi(ressource[1]);
            m_dbname = ressource[2];
            m_dbaccount = ressource[3];
            m_dbpassword = ressource[4];

            // influxdb_cpp::server_info si(m_host, m_port, .....);
            tagSetVector.push_back(".class_name=");
        }

        void publish(nlohmann::json j)
        {
            influxdb_cpp::server_info si(m_host, m_port, m_dbname, m_dbaccount, m_dbpassword);
            //influxdb_cpp::server_info si("dbod-testinfluxyd.cern.ch", 8095, "pyexample", "admin", "admin");
            std::string resp;

            influxdb_cpp::query(resp, "CREATE DATABASE mydbX", si);

            int ret = influxdb_cpp::builder()
                .meas("test")
                .tag("k", "v")
                .tag("x", "y")
                .field("x", 10)
                .field("y", 10.3, 2)
                .field("b", !!10)
                .timestamp(1512722735522840439)
                .post_http(si, &resp);

            //jsonConverter.setInsertsVector(false, tagSetVector, timeVariableName, j);
            //std::vector<std::string> insertsVector = jsonConverter.getInsertsVector();

            for (int i = 0; i < insertsVector.size(); i++)
            {
                influxdb_cpp::query(resp, insertsVector[i], si);
                std::cout << resp << std::endl;
            }
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
        std::vector<std::string> getInsertsVector()
        {
            return insertsVector;
        }

    protected:
        typedef OpmonService inherited;
    private:
        //std::ofstream m_ofs;
        std::string m_host;
        int32_t m_port;
        std::string m_dbname;
        std::string m_dbaccount;
        std::string m_dbpassword;

        std::vector<std::string> tagSetVector;
        std::string timeVariableName = ".time=";
        //JsonConverter jsonConverter;



        using json = nlohmann::json;

        std::string JsonConverter::checkDataType(std::string line)
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
                lineOriginal = lineOriginal + '"' + line + '"';
            }

            return lineOriginal;
        }



        std::vector<std::string> JsonConverter::jsonToInfluxFunction(bool ignoreTags, std::vector<std::string> tagSetVector, std::string timeVariableName, json jsonStream)
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

            std::string insertCommandTag = "INSERT " + applicationName + ",";
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
                    std::cout << insertCommandTag + " " + insertCommandField + " " + time << "\n";

                    insertCommandTag = "INSERT " + applicationName + ",";
                    insertCommandField = "";
                }

            }

            return vectorInserts;
        }

    };

}

extern "C" {
    std::shared_ptr<dunedaq::opmonlib::OpmonService> make(std::string service) {
        return std::shared_ptr<dunedaq::opmonlib::OpmonService>(new dunedaq::influxopmon::influxOpmonService(service));
    }
}
