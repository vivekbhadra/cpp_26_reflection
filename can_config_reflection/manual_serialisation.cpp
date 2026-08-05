// manual_serialisation.cpp
// build: g++ -std=c++17 manual_serialisation.cpp -o can_config_manual \
//        $(pkg-config --cflags --libs poco-json) -lPocoJSON -lPocoFoundation

#include "domain.h"

#include <iostream>

int main()
{
    const CanDataSource source{"can-source-1",
                               "can1",
                               250000,
                               true,
                               {0xFEF1, 0xFEEE},
                               RetryPolicy{3, 500, true}};

    source.toJson().stringify(std::cout, 4);
    std::cout << "\n";

    return 0;
}