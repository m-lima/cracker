#pragma once

#include <string>
#include <vector>

#include "../handlers/target_handler.hpp"

class AbstractCracker {
protected:
  std::vector<std::pair<std::string, std::string>> mHeaders;
  std::vector<std::pair<std::string, std::string>> mRunningStats;

public:
  AbstractCracker() = default;
  virtual ~AbstractCracker() = default;

  virtual void crack(TargetHandler & targetHandler) = 0;

  inline const std::vector<std::pair<std::string, std::string>> * getHeaders() const {
    return &mHeaders;
  };

  inline const std::vector<std::pair<std::string, std::string>> * getRunningStats() const {
    return &mRunningStats;
  };
};
