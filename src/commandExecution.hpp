#ifndef INFLUXOPMON_INCLUDE_INFLUXOPMON_JSONINFLUXCONVERTER_H_
#define INFLUXOPMON_INCLUDE_INFLUXOPMON_JSONINFLUXCONVERTER_H_

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

class CommandExecution
{

    std::string consoleOutput;

    private:
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
    public:
        std::string getSetInsertsVector()
        {
            return exec(const char* cmd);
        }
};


#endif
