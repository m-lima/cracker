#pragma once

#include <string>

#include <mfl/cl/program.hpp>

class CrackerProgram : public mfl::cl::Program {
private:
  const std::string cPrefix;
  const unsigned int cRange;
  const unsigned int cPrefixRange;
  const unsigned int cTargetCount;

public:
  static constexpr unsigned int BINARY_THRESHOLD = 10;
  static constexpr unsigned int EYTZINGER_THRESHOLD = 256;

  CrackerProgram(const std::string & prefix,
                 unsigned int length,
                 unsigned int targetCount,
                 std::uint64_t maxValue,
                 unsigned int range);

  const std::string getSource() const override;

  inline const char * path() const override {
    return "opencl/cracker/cracker.cl";
  }

  inline std::string getPrefix() const {
    return cPrefix;
  }

  inline unsigned int getRange() const {
    return cRange;
  }

  inline unsigned int getPrefixRange() const {
    return cPrefixRange;
  }

  inline const char * name() const override {
    return "cracker";
  }

  inline static auto NAME() {
    return "cracker";
  }
};
