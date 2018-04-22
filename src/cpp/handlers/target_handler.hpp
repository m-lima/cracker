#pragma once

#include <string>
#include <unordered_map>

#include "../hash.hpp"

class TargetHandler {
public:
  std::unordered_map<Hash::Hash, Value, Hash::Hash, Hash::HashComparator> targets;
  TargetHandler(const char * targetString);
  void parseAndInsertTargets(const std::string & targetString);
};
