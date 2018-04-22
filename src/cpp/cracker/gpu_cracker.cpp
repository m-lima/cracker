#include "gpu_cracker.hpp"

#include <iostream>

#include <mfl/cl/runner.hpp>
#include <mfl/timer.hpp>

namespace {
  inline std::vector<Hash::Hash> buildList(const TargetHandler & targetHandler) {
    const auto targetCount = targetHandler.targets.size();

    std::vector<Hash::Hash> targetsMissing;
    targetsMissing.reserve(targetCount);
    for (const auto & target : targetHandler.targets) {
      targetsMissing.push_back(target.first);
    }
    return targetsMissing;
  }

  inline std::vector<Hash::Hash> buildSortedList(const TargetHandler & targetHandler) {
    std::vector<Hash::Hash> sortedList = buildList(targetHandler);
    std::sort(sortedList.begin(), sortedList.end());
    return sortedList;
  }

  inline std::vector<Hash::Hash> buildEytzinger(const TargetHandler & targetHandler) {
    const auto targetCount = targetHandler.targets.size();

    // Quick sort first
    std::vector<Hash::Hash> sortedHashes = buildSortedList(targetHandler);

    // Pre compute indices
    unsigned int sequence[targetCount];
    unsigned int depth = std::log2(targetCount);
    unsigned int lastIndex = ((targetCount % static_cast<unsigned int>(std::pow(2, depth))) + 1) * 2 - 1;

    unsigned int levelIndices[depth + 1];
    for (unsigned int i = 0; i <= depth; ++i) {
      levelIndices[depth - i] = std::pow(2, i) - 1;
    }

    for (unsigned int i = 0; i < targetCount; ++i) {
      unsigned int offset = 1;
      if (i > lastIndex) {
        offset = i - lastIndex + 1;
      } else {
        if (i % 2 == 0) {
          sequence[i] = 0;
          continue;
        }
      }
//      unsigned int offset = i > lastIndex ? i - lastIndex + 1 : 1;
      for (int j = depth; j >= 0; --j) {
        if ((i + offset) % static_cast<unsigned int>(std::pow(2, j)) == 0) {
          sequence[i] = j;
          break;
        }
      }
    }

    // Actually place them in a flat binary serach order with left most balance (Eytzinger)
    std::vector<Hash::Hash> targetsMissing(targetCount);
    for (unsigned int i = 0; i < targetCount; ++i) {
      targetsMissing[levelIndices[sequence[i]]++] = sortedHashes[i];
    }

    return targetsMissing;
  }
}

void GpuCracker::crack(TargetHandler & targetHandler) {
  mfl::cl::Runner runner(CL_DEVICE_TYPE_GPU);

  const auto targetCount = targetHandler.targets.size();

  // Populate local vector for buffer
  auto command = runner.commandQueues(1)[0];

  mfl::out::println("Preparing");
  std::vector<Hash::Hash> targetsMissing = targetCount < CrackerProgram::BINARY_THRESHOLD
                                           ? buildList(targetHandler)
                                           : targetCount < CrackerProgram::EYTZINGER_THRESHOLD
                                             ? buildSortedList(targetHandler)
                                             : buildEytzinger(targetHandler);
  mfl::out::println("Done preparing");

  cl::Buffer targetsBuffer(runner, CL_MEM_READ_ONLY, sizeof(Hash::Hash) * targetCount);
  cl::copy(command, targetsMissing.begin(), targetsMissing.end(), targetsBuffer);

  cl::Buffer outputBuffer(runner, CL_MEM_WRITE_ONLY, (2 * sizeof(::cl_uint)) * targetCount);

  runner.loadProgram(cCrackerProgram);
  auto kernel = runner.makeKernel(CrackerProgram::NAME(), "crack");

  kernel.setArg(0, targetsBuffer);
  kernel.setArg(1, outputBuffer);

  mfl::out::println("Executing");
  mfl::Timer timer;
  timer.start();
  if (cCrackerProgram.getPrefixRange() > 1) {
    for (unsigned int prefix = 0; prefix < cCrackerProgram.getPrefixRange(); ++prefix) {
      kernel.setArg(2, prefix);
      fmt::print(std::cout, "\rProcessed  {:02d}%", prefix * 100 / cCrackerProgram.getPrefixRange());
      std::cout.flush();
      command.enqueueNDRangeKernel(kernel, cl::NDRange(0), cl::NDRange(cCrackerProgram.getRange()));
      command.finish();
    }
    mfl::out::println("\rProcessed  {:02d}%", 100);
  } else {
    kernel.setArg(2, 0);
    command.enqueueNDRangeKernel(kernel, cl::NDRange(0), cl::NDRange(cCrackerProgram.getRange()));
  }

  std::vector<::cl_uint> values(targetCount << 1);
  cl::copy(command, outputBuffer, values.begin(), values.end());
  mfl::out::println("Done");
  timer.stop();

  const std::uint64_t totalHashes =
      static_cast<std::uint64_t>(cCrackerProgram.getRange()) * cCrackerProgram.getPrefixRange();
  const std::uint64_t numericPrefix = cCrackerProgram.getPrefix().empty()
                                      ? 0
                                      : (std::stoull(cCrackerProgram.getPrefix()) * totalHashes);

  for (std::size_t i = 0; i < values.size(); i += 2) {
    if (values[i] > 0 || values[i + 1] > 0) {
      auto search = targetHandler.targets.find(targetsMissing[i >> 1]);
      if (search != targetHandler.targets.end() && search->second == 0) {
        search->second = values[i]
                         + (static_cast<std::uint64_t>(values[i + 1]) * cCrackerProgram.getRange())
                         + numericPrefix;
      }
    }
  }

  const auto duration = timer.getDuration();
  mRunningStats.emplace_back("Time elapsed cracking hashes", mfl::string::durationToString(duration));
  mRunningStats.emplace_back("Average hashes per millisecond",
                             (duration > 0 && totalHashes > 0)
                             ? mfl::string::simplifyValue(static_cast<double>(totalHashes) / duration)
                             : "N/A");
}
