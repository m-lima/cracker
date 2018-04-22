#include "cracker_program.hpp"

#include <regex>
#include <cmath>

#include <fmt/format.h>

namespace {
  constexpr char SOURCE[] =
#include "cracker/cracker.cl"
  ;

  constexpr unsigned int MAX_RANGE = 1000000000;

  inline unsigned char buildPrefixDecimalPlaces(std::uint64_t maxValue,
                                                unsigned int range) {
    unsigned char prefixDecimalPlaces = 0;
    while (maxValue > range) {
      ++prefixDecimalPlaces;
      maxValue /= 10;
    }

    return prefixDecimalPlaces;
  }

  inline unsigned int buildPrefixRange(std::uint64_t maxValue,
                                       unsigned int range) {
    unsigned int prefixRange = 1;
    while (maxValue > range) {
      prefixRange *= 10;
      maxValue /= 10;
    }

    return prefixRange;
  }

  inline unsigned int buildRange(unsigned lengthToCrack, unsigned int requestedRange) {
    auto range = std::min(lengthToCrack, requestedRange);
    return MAX_RANGE < range ? MAX_RANGE : range;
  }

  inline std::string buildBuildString(const std::string & prefix,
                                      unsigned int length,
                                      unsigned int targetCount,
                                      unsigned char prefixDecimalPlaces) {

    if (prefixDecimalPlaces > 0) {
      return fmt::format("-D CONST_BEGIN={:d}"
                             " -D CONST_END={:d}"
                             " -D CONST_SUFFIX={:d}ul"
                             " -D CONST_PREFIX_DECIMAL_PLACES={:d}"
                             " -D CONST_TARGET_COUNT={:d}",
                         prefix.length(),
                         length,
                         length * 8,
                         prefixDecimalPlaces,
                         targetCount);
    } else {
      return fmt::format("-D CONST_BEGIN={:d}"
                             " -D CONST_END={:d}"
                             " -D CONST_SUFFIX={:d}ul"
                             " -D CONST_TARGET_COUNT={:d}",
                         prefix.length(),
                         length,
                         length * 8,
                         targetCount);
    }
  }
}

CrackerProgram::CrackerProgram(const std::string & prefix,
                               unsigned int length,
                               unsigned int targetCount,
                               std::uint64_t maxValue,
                               unsigned int range)
    : cPrefix(prefix),
      cRange(buildRange(std::pow(10, length - prefix.length()), range)),
      cPrefixRange(buildPrefixRange(maxValue, cRange)),
      cTargetCount(targetCount),
      Program(buildBuildString(prefix,
                               length,
                               targetCount,
                               buildPrefixDecimalPlaces(maxValue,
                                                        buildRange(std::pow(10, length - prefix.length()),
                                                                   range)))) {}

const std::string CrackerProgram::getSource() const {
  if (cPrefix.empty()) {
    return SOURCE;
  }

  std::string replacement;
  for (std::size_t i = 0; i < cPrefix.length(); ++i) {
    replacement.append(fmt::format("value.bytes[{:d}] = \'{:c}\';", i, cPrefix[i]));
  }
  std::regex regex("// %%PREFIX%%");
  return std::regex_replace(SOURCE, regex, replacement);
}

