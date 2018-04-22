#pragma once

#include "abstract_cracker.hpp"

#include "../cl/cracker_program.hpp"
#include "../handlers/target_handler.hpp"

class GpuCracker : public AbstractCracker {
private:
  const CrackerProgram cCrackerProgram;
  const std::uint64_t cRange;

public:
  GpuCracker(const std::string & prefix,
             unsigned int length,
             unsigned int targetCount,
             std::uint64_t maxValue,
             unsigned int range)
      : AbstractCracker(),
        cCrackerProgram(prefix,
                        length,
                        targetCount,
                        maxValue,
                        range),
        cRange(range) {
    mHeaders.emplace_back("Prefix", prefix);
    mHeaders.emplace_back("Length", std::to_string(length));
    mHeaders.emplace_back("Target count", std::to_string(targetCount));
    mHeaders.emplace_back("Parallel kernels", std::to_string(cCrackerProgram.getRange()));
    mHeaders.emplace_back("Iterations", std::to_string(cCrackerProgram.getPrefixRange()));
    mHeaders.emplace_back("Build defines", cCrackerProgram.buildString());
  }

  void crack(TargetHandler & targetHandler) override;

};
