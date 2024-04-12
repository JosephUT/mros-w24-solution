#ifndef EXAMPLE_MESSAGES_HPP
#define EXAMPLE_MESSAGES_HPP

#include <cstdint>
#include <string>
#include <nlohmann/json.hpp>


using Json = nlohmann::json;

namespace Messages {
    struct String {
        std::string base;

        static std::string messageName() {
            // return the exact name of the struct
            // idk how I want to handle namespaces
            // i.e. messages::String
            return "String";
        }

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(String, base)
    };
}


#endif