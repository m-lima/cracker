#include <mfl/args.hpp>
#include <mfl/out.hpp>
#include <mfl/file.hpp>
#include <mfl/string.hpp>

#include "handlers/target_handler.hpp"
#include "handlers/file_handler.hpp"
#include "md5/md5.hpp"
#include "cracker/abstract_cracker.hpp"
#include "cracker/cpu_cracker.hpp"
#include "cracker/gpu_cracker.hpp"

enum Device {
  GPU,
  CPU,
  AUTO
};

void printUsage() {

  mfl::out::println("Usage for cracking:");
  mfl::out::println("cracker [-t TARGETS] [-d DEVICE] [-i INPUT] [-o OUTPUT] [-r RAINBOW] [-p PREFIX] [-l LENGTH]"
                        " [-n ND_RANGE] [-c THREAD_COUNT]");
  mfl::out::println("  {:<15}{}\n"
                        "  {:<15}{}\n"
                        "  {:<15}{}\n"
                        "  {:<15}{}\n"
                        "  {:<15}{}\n"
                        "  {:<15}{}\n"
                        "  {:<15}{}",
                    "TARGETS", "A list of MD5 hashes",
                    "", "The MD5 hash is defined as a 32-digit hex value. Any separators or any kind of symbol",
                    "", "will be ignored",
                    "", "The list may also be provided from a file or, if in a *NIX system, through",
                    "", "piping and streaming",
                    "", "The final list will be the concatenation of all methods",
                    "", "If the list is piped or streamed, the \"-t\" argument is no longer mandatory");
  mfl::out::println("  {:<15}{}\n"
                        "  {:<15}{}"
                        "  {:<15}{}"
                        "  {:<15}{}",
                    "DEVICE", "Device to execute on: GPU, CPU, or AUTO.  (default: AUTO)",
                    "", "GPU - Execute in the GPU using openCL",
                    "", "CPU - Execute in the CPU using multiple threads",
                    "", "AUTO - Decide the device automatically based on the value space");
  mfl::out::println("  {:<15}{}\n"
                        "  {:<15}{}",
                    "INPUT", "A file to scan for MD5 hashes. Will find all 32-digit hex values and set them as TARGETs",
                    "", "regardless of the format");
  mfl::out::println("  {:<15}{}\n"
                        "  {:<15}{}",
                    "OUTPUT", "[INPUT defined only] Output file for the cracked input. All cracked hashes will be",
                    "", "replaced in place and saved to OUTPUT");
  mfl::out::println("  {:<15}{}",
                    "RAINBOW", "Rainbow table. Will be updated if new items are discovered");
  mfl::out::println("  {:<15}{}",
                    "PREFIX", "First digits of the value");
  mfl::out::println("  {:<15}{}",
                    "LENGTH", "Length of the value (default: 12)");
  mfl::out::println("  {:<15}{}\n"
                        "  {:<15}{}",
                    "ND_RANGE", "[GPU only] Maximum size of the ND range (default: 10000000)",
                    "", "Increase this in case the value is larger than 2^56");
  mfl::out::println("  {:<15}{}",
                    "THREAD_COUNT", "[CPU only] Number of threads to launch (default: 256)");
  mfl::out::println();

  mfl::out::println("Usage for hashing:");
  mfl::out::println("cracker -e <VALUE>");
  mfl::out::println("  {:<15}{}",
                    "VALUE", "VALUE to enconde with MD5");
  mfl::out::println();

  mfl::out::println("Usage for help:");
  mfl::out::println("cracker -h");
}

void displayHeaders(const AbstractCracker * cracker) {
  auto headers = cracker->getHeaders();
  if (headers->empty()) {
    return;
  }

  unsigned int maxSize = 0;
  for (const auto & header : *headers) {
    if (maxSize < header.first.length()) {
      maxSize = header.first.length();
    }
  }
  const std::string format("  {:<" + std::to_string(maxSize + 1) + "s} {:s}");

  for (const auto & header : *headers) {
    mfl::out::println(format.c_str(), header.first + ":", header.second);
  }
}

void displayStats(const AbstractCracker * cracker) {
  auto stats = cracker->getRunningStats();
  if (stats->empty()) {
    return;
  }

  unsigned int maxSize = 0;
  for (const auto & header : *stats) {
    if (maxSize < header.first.length()) {
      maxSize = header.first.length();
    }
  }
  const std::string format("  {:<" + std::to_string(maxSize + 1) + "s} {:s}");

  mfl::out::println("Running stats:");
  for (const auto & stat : *stats) {
    mfl::out::println(format.c_str(), stat.first + ":", stat.second);
  }
}

void displayResults(unsigned int length,
                    const TargetHandler & targetHandler,
                    const RainbowHandler & rainbowHandler) {

  const auto count = targetHandler.targets.size() + rainbowHandler.rainbow.size();

  mfl::out::println("Results:");
  const auto format = std::string("{:s} :: {:0" + std::to_string(length) + "d} [{:s}]");

  unsigned int rainbowedCount = 0;
  unsigned int crackedCount = 0;

  for (const auto & item : rainbowHandler.rainbow) {
    if (item.second > 0) {
      mfl::out::println(format.c_str(),
                        item.first,
                        item.second,
                        "rainbowed");
      rainbowedCount++;
    } else {
      mfl::out::println("{:s} :: NOT FOUND", item.first);
    }
  }

  for (const auto & item : targetHandler.targets) {
    if (item.second > 0) {
      mfl::out::println(format.c_str(),
                        item.first,
                        item.second,
                        "cracked");
      crackedCount++;
    } else {
      mfl::out::println("{:s} :: NOT FOUND", item.first);
    }
  }

  mfl::out::println("Rainbow table hits: {:d} ({:d}%)",
                    rainbowedCount, rainbowedCount * 100 / count);
  mfl::out::println("Cracker hits: {:d} ({:d}%)",
                    crackedCount, crackedCount * 100 / count);
  mfl::out::println("Values found: {:d}/{:d} ({:d}%)",
                    crackedCount + rainbowedCount,
                    count,
                    (crackedCount + rainbowedCount) * 100 / count);
}

int main(int argc, char * argv[]) {
  if (mfl::args::findOption(argv, argv + argc, "-h")) {
    printUsage();
    return 0;
  }

  auto hash = mfl::args::extractOption(argv, argv + argc, "-e");
  if (hash) {
    mfl::out::println("{}", md5(hash));
    return 0;
  }

  Device device = AUTO;
  std::string prefix = "";
  int threadCount = 256;
  int length = 12;
  std::uint64_t range = 10000000;

  auto aDevice = mfl::args::extractOption(argv, argv + argc, "-d");
  auto aTarget = mfl::args::extractOption(argv, argv + argc, "-t");
  auto aInput = mfl::args::extractOption(argv, argv + argc, "-i");
  auto aOutput = mfl::args::extractOption(argv, argv + argc, "-o");
  auto aPrefix = mfl::args::extractOption(argv, argv + argc, "-p");
  auto aThreadCount = mfl::args::extractOption(argv, argv + argc, "-c");
  auto aLength = mfl::args::extractOption(argv, argv + argc, "-l");
  auto aRainbow = mfl::args::extractOption(argv, argv + argc, "-r");
  auto aRange = mfl::args::extractOption(argv, argv + argc, "-n");

  if (aOutput) {
    if (!aInput) {
      mfl::out::println(stderr,
                        "A conversion output file was specified, but no input was provided");
      mfl::out::println();
      printUsage();
      return -1;
    }

    if (mfl::file::testFileExists(aOutput)) {
      mfl::out::println(stderr, "The output file \"{:s}\" already exists", aOutput);
      return -1;
    }
  }

  if (aDevice) {
    if (std::strncmp(aDevice, "GPU", 3) == 0) {
      device = GPU;
    } else if (std::strncmp(aDevice, "CPU", 3) == 0) {
      device = CPU;
    } else if (std::strncmp(aDevice, "AUTO", 4) == 0) {
      device = AUTO;
    } else {
      mfl::out::println(stderr, "Invalid device: {:s}", aDevice);
      mfl::out::println();
      printUsage();
      return -1;
    }
  }

  if (aLength) {
    length = std::atoi(aLength);
  }

  if (aPrefix) {
    prefix = aPrefix;
    mfl::string::trim(prefix);
  }

  if (aThreadCount) {
    if (device == GPU) {
      mfl::out::println(stderr, "Invalid argument \"-c\" when running on GPU. Ignoring..");
    } else {
      threadCount = std::atoi(aThreadCount);
    }
  }

  if (aRange) {
    if (device == CPU) {
      mfl::out::println(stderr, "Invalid argument \"-n\" when running on CPU. Ignoring..");
    } else {
      range = std::atoi(aRange);
    }
  }

  if (length < 1) {
    mfl::out::println(stderr, "The length has to be larger than zero");
    mfl::out::println();
    printUsage();
    return -1;
  }

  if (prefix.length() >= static_cast<std::size_t>(length)) {
    mfl::out::println(stderr, "The length has to be larger than the prefix");
    mfl::out::println();
    printUsage();
    return -1;
  }

  // Populate targets
  TargetHandler targetHandler(aTarget);

  FileHandler fileHandler(aInput);
  fileHandler.load(targetHandler);

  if (targetHandler.targets.empty()) {
    mfl::out::println(stderr, "No targets were specified");
    mfl::out::println();
    printUsage();
    return -1;
  }

  // Load rainbow table
  RainbowHandler rainbowHandler(aRainbow);
  rainbowHandler.load(targetHandler);

  if (!targetHandler.targets.empty()) {

    if (device == AUTO) {
      device = std::pow(10, length - prefix.length()) < 10000000 ? CPU : GPU;
    }

    try {
      AbstractCracker * cracker;
      mfl::out::println();
      if (device == GPU) {
        mfl::out::println("Initializing cracker on GPU");
        cracker = new GpuCracker(prefix,
                                 length,
                                 targetHandler.targets.size(),
                                 std::pow(10, length - prefix.length()),
                                 range);
      } else {
        mfl::out::println("Initializing cracker on CPU");
        cracker = new CpuCracker(prefix,
                                 length,
                                 targetHandler.targets.size(),
                                 threadCount);
      }

      // Display configuration
      displayHeaders(cracker);

      // Crack this mofo
      mfl::out::println();
      cracker->crack(targetHandler);

      // Display running statistics
      mfl::out::println();
      displayStats(cracker);

    } catch (const std::exception & ex) {
      mfl::out::println(stderr,
                        "Exception caught. Reason:\n{}", ex.what());
      return -1;
    }
  }

  // Display results
  mfl::out::println();
  displayResults(length, targetHandler, rainbowHandler);

  // Save rainbow table
  mfl::out::println();
  rainbowHandler.save(targetHandler);

  // Save file
  if (aOutput) {
    mfl::out::println();
    fileHandler.save(targetHandler, rainbowHandler, aOutput, length);
  }

  return 0;
}
