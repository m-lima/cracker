#include "file_handler.hpp"

#include <regex>
#include <fstream>

#include <mfl/out.hpp>

void FileHandler::load(TargetHandler & targetHandler) {
  if (cPath.empty()) {
    return;
  }

  std::ifstream file(cPath);
  if (file.is_open()) {
    mfl::out::println("Loading file: {:s}", cPath);
    std::string tempString;
    while (std::getline(file, tempString)) {
      targetHandler.parseAndInsertTargets(tempString);
    }

    file.close();
  } else {
    throw std::invalid_argument(fmt::format("the file given \"{:s}\" is invalid", cPath));
  }
}

void FileHandler::save(const TargetHandler & targetHandler,
                       const RainbowHandler & rainbowHandler,
                       const char * outputPath,
                       int length) {
  if (cPath.empty()) {
    throw std::invalid_argument(fmt::format("the file given \"{:s}\" is invalid", cPath));
  }

  if (!outputPath) {
    throw std::invalid_argument(fmt::format("the output file given \"{:s}\" is invalid", outputPath));
  }

  std::ofstream outFile;
  outFile.open(outputPath, std::ios::out);
  if (!outFile.is_open()) {
    throw std::invalid_argument(fmt::format("the output file given \"{:s}\" is invalid", outputPath));
  }

  std::ifstream file(cPath);
  if (!file.is_open()) {
    throw std::invalid_argument(fmt::format("the file given \"{:s}\" is invalid", cPath));
  }

  mfl::out::println("Converting file into: {:s}", outputPath);

  std::regex hashRegex("([a-fA-F0-9]{32})");
  std::smatch match;
  std::string outString;
  std::string tempString;
  std::string format("{:" + std::to_string(length) + "d}");

  while (std::getline(file, outString)) {
    tempString = outString;
    while (std::regex_search(tempString, match, hashRegex)) {
      for (std::size_t i = 1; i < match.size(); ++i) {
        const std::string matchString = match[i];
        Hash::Hash hash(matchString);
        auto entry = targetHandler.targets.find(hash);
        if (entry != targetHandler.targets.end() && entry->second > 0) {
          outString = std::regex_replace(outString,
                                         std::regex(matchString),
                                         fmt::format(format, entry->second));
        } else {
          entry = rainbowHandler.rainbow.find(hash);
          if (entry != rainbowHandler.rainbow.end() && entry->second > 0) {
            outString = std::regex_replace(outString,
                                           std::regex(matchString),
                                           fmt::format(format, entry->second));
          }
        }
        tempString = match.suffix();
      }

      outFile << outString << std::endl;
    }
  }

  mfl::out::println("Conversion completed");

  file.close();
  outFile.close();
}
