#pragma once

#include <string>

#include "nlohmann/json.hpp"

struct StringMessage {
  std::string data;

  void set_from_json(nlohmann::json json) {
    const std::string thing = json["data"];
    data = thing;
  }

  nlohmann::json convert_to_json() {
    nlohmann::json json{{"data", data}};
    return json;
  }
};
