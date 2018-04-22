#pragma once

#include "abstract_cracker.hpp"

#include <atomic>

class CpuCracker : public AbstractCracker {
private:

  // Constants
  const std::string cPrefix;
  const std::string cFormat;
  const int cThreadCount;
  const int cLengthToCrack;

  // Threading stuff
  std::atomic<int> mTargetsNotFoundCount;
  std::atomic<std::size_t> mHashCount{0};

  // Private functions
  void threadCrack(const std::uint64_t initial,
                   const std::uint64_t final,
                   const char * format,
                   TargetHandler * targetHandler);

public:
  CpuCracker(const std::string & prefix,
             const int length,
             unsigned int targetCount,
             const int threadCount)
      : cPrefix(prefix),
        cFormat(prefix + "{:0" + std::to_string(length - prefix.size()) + "d}"),
        cThreadCount(threadCount),
        mTargetsNotFoundCount(targetCount),
        cLengthToCrack(length - prefix.size()) {
    mHeaders.emplace_back("Prefix", prefix);
    mHeaders.emplace_back("Length", std::to_string(length));
    mHeaders.emplace_back("Target count", std::to_string(targetCount));
    mHeaders.emplace_back("Thread count", std::to_string(threadCount));
    mHeaders.emplace_back("Format", cFormat);
  }


  void crack(TargetHandler & targetHandler) override;
};
