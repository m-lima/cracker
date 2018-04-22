#include "cpu_cracker.hpp"

#include <thread>

#include <mfl/timer.hpp>
#include <mfl/out.hpp>

#include "../md5/md5.hpp"

void CpuCracker::threadCrack(const std::uint64_t initial,
                             const std::uint64_t final,
                             const char * format,
                             TargetHandler * targetHandler) {

  std::uint64_t i = initial;
  while (mTargetsNotFoundCount > 0 && i <= final) {
    Hash::Hash hash(md5Hash(fmt::format(format, i)));
    auto search = targetHandler->targets.find(hash);
    if (search != targetHandler->targets.end() && search->second == 0) {
      mTargetsNotFoundCount--;
      search->second = std::stoull(fmt::format(format, i));
    }
    ++i;
  }
  mHashCount += (i - initial);
}

void CpuCracker::crack(TargetHandler & targetHandler) {
  const std::uint64_t maxValue = std::pow(10, cLengthToCrack) - 1;
  const std::uint64_t step = maxValue / cThreadCount;
#ifdef WIN32
  std::vector<std::thread> threads;
  threads.reserve(cThreadCount);
#else
  std::thread threads[cThreadCount];
#endif // WIN32

  int threadsLaunched = cThreadCount;

  mfl::Timer timer;
  timer.start();

  for (int i = 0; i < cThreadCount; ++i) {
    if (mTargetsNotFoundCount > 0) {
      const std::uint64_t final = i == cThreadCount - 1
                                  ? maxValue
                                  : (i + 1) * step - 1;
#ifdef WIN32
      threads.emplace_back(&CpuCracker::threadCrack, this, i * step, final, cFormat.c_str(), &targetHandler);
#else
      threads[i] =
          std::thread(&CpuCracker::threadCrack, this, i * step, final, cFormat.c_str(), &targetHandler);
#endif // WIN32

    } else {
      threadsLaunched = i;
      break;
    }
  }

  for (int i = 0; i < threadsLaunched; ++i) {
    threads[i].join();
  }

  timer.stop();

  const auto duration = timer.getDuration();
  mRunningStats.emplace_back("Threads launched", fmt::format("{:d} ({:d}%)",
                                                             threadsLaunched,
                                                             threadsLaunched * 100 / cThreadCount));
  mRunningStats.emplace_back("Time elapsed cracking hashes", mfl::string::durationToString(duration));
  mRunningStats.emplace_back("Average hashes per millisecond",
                             (duration > 0 && mHashCount > 0)
                             ? mfl::string::simplifyValue(static_cast<double>(mHashCount) / duration)
                             : "N/A");
}
