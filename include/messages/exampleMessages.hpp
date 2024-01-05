#ifndef EXAMPLE_MESSAGES_HPP
#define EXAMPLE_MESSAGES_HPP

#include <cstdint>
#include <string>
#include <nlohmann/json.hpp>


using Json = nlohmann::json;

namespace Messages {
/**
 * Sample string message
 */
    struct String {
      /**
       * String message to be sent
       */
        std::string base;

        /**
         * Name of message. Must be known at compile time (static) and must be in each message definition
         * @return
         */
        static std::string messageName() {
            // return the exact name of the struct
            // idk how I want to handle namespaces
            // i.e. Messages::String
            return "String";
        }

        /**
         * Conversion to and from Json MACRO
         * @param nlohmann_json_j name of struct
         * @param nlohmann_json_t list of type unsafe names in order
         */
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(String, base)
    };
}


#endif