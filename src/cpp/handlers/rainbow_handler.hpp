#pragma once

#include <string>

#include "target_handler.hpp"

class RainbowHandler {
private:
  static constexpr std::size_t BUFFER_SIZE = 8192;
  const std::string cPath;

public:
  std::unordered_map<Hash::Hash, Value, Hash::Hash, Hash::HashComparator> rainbow;

  RainbowHandler(const char * path) : cPath(path ? path : "") {}

  void load(TargetHandler & targetHandler);
  void save(TargetHandler & targetHandler);
};
