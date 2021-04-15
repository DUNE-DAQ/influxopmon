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
#include "JsonInfluxConverter.hpp"
//#include "cpr/cpr.h"
#include <curl/curl.h>

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

        std::string executionCommand(const char* cmd) {
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
        void executionCommandSilent(const char* cmd) {
            system(cmd);
        }

        explicit influxOpmonService(std::string uri) : dunedaq::opmonlib::OpmonService(uri) {

            uri = uri.substr(uri.find("/") + 2);
            std::string tmp;
            std::stringstream ss(uri);
            std::vector<std::string> ressource;
            int countArgs = 0;
            while (getline(ss, tmp, ':'))
            {
                ressource.push_back(tmp);
                countArgs++;
            }

            if (countArgs < 3)
            {
                std::cout << "invalid URI, follow: influx://proxyAdress:database:Delimiter:Tags(0..N) \n Example: influx://188.185.88.195:db1:.time=:.class_name=\n";
                exit(0);
            }

	    m_host = ressource[0];
            m_dbname = ressource[1];
            m_delimiter = ressource[2];

            for (unsigned long int i = 3; i < ressource.size(); i++)
            {
                tagSetVector.push_back(ressource[i]);
            }


		std::cout << "\n curl code";

  		CURL *curl;
		CURLcode res;
 
  		curl_global_init(CURL_GLOBAL_DEFAULT);

		curl = curl_easy_init();
	
  		if(curl) {
    			/* First set the URL that is about to receive our POST. This URL can
 			*        just as well be a https:// URL if that is what should receive the
			 *               data. */ 
    			curl_easy_setopt(curl, CURLOPT_URL, "http://postit.example.com/moo.cgi");
    			/* Now specify the POST data */ 
    			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");
 
    			/* Perform the request, res will get the return code */ 
    			res = curl_easy_perform(curl);
    			/* Check for errors */ 
    			if(res != CURLE_OK)
      				fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
 
		    	/* always cleanup */ 
		  	curl_easy_cleanup(curl);
  		  }	
		  curl_global_cleanup();




		

	//	cpr::Response r = cpr::Get(cpr::Url{"https://google.com"});
		std::cout << "\n end curl code\n\n";




	}

        void publish(nlohmann::json j)
        {
            
            jsonConverter.setInsertsVector(false, tagSetVector, m_delimiter, j.flatten().dump());
            insertsVector = jsonConverter.getInsertsVector();
            
            
            querry = "curl "  + m_host +  "/?db=" + m_dbname + " --data-binary '";
            
            for (unsigned long int i = 0; i < insertsVector.size(); i++)
            {
                //std::cout << insertsVector[i] + "\n" ;
                querry = querry + insertsVector[i] + "\n" ;
            }

            //silent output
            querry = querry + "' >nul 2>nul";
            charPointer = querry.c_str();
            executionCommand(charPointer);

            //output
/*           
    	    querry = querry + "\'";
            std::cout << querry;
	    charPointer = querry.c_str();
            std::cout << executionCommand(charPointer);
  */          
            
        }

    protected:
        typedef OpmonService inherited;
    private:
        std::string m_host;
        std::string m_dbname;
        
        std::vector<std::string> tagSetVector;
        std::string m_delimiter;
        std::vector<std::string> insertsVector;
        
        std::string querry;
        const char* charPointer;
        influxopmon::JsonConverter jsonConverter;

    };

}

extern "C" {
    std::shared_ptr<dunedaq::opmonlib::OpmonService> make(std::string service) {
        return std::shared_ptr<dunedaq::opmonlib::OpmonService>(new dunedaq::influxopmon::influxOpmonService(service));
    }
}

