/**
 * @brief Using namespace for convenience
 */
#include "InsertQueryListBuilder.hpp"

#include <fstream>

using json = nlohmann::json;

int main(int argc, char const *argv[])
{

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <opmonlib sample>.json" << std::endl;
        exit(-1);
    }

    std::ifstream file((argv[1]));
    json j = json::parse(file);

    std::cout << j << std::endl;

    std::cout << "------" << std::endl;
    auto iqb = dunedaq::influxopmon::InsertQueryListBuilder(j);

    for( auto str : iqb.get()) {
        std::cout << str << std::endl;
    }

    /* code */
    return 0;
}