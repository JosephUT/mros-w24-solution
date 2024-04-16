#pragma once

#include <string>

#include "nlohmann/json.hpp"

std::string toURI(const std::string &host, int port);

template <typename T>
concept JsonConvertible = requires (T t, nlohmann::json json){
  { t.convert_to_json()} -> std::same_as<nlohmann::json>;
  { t.set_from_json(json)} -> std::same_as<void>;
};
