#include "target_handler.hpp"

#include <regex>
#include <iostream>
#ifndef WIN32
#include <sys/poll.h>
#endif

#include <mfl/out.hpp>

TargetHandler::TargetHandler(const char * targetString) {
  if (targetString) {
    parseAndInsertTargets(targetString);
  }

#ifndef WIN32
  struct pollfd stdin_poll = {
      .fd = 0,
      .events = POLLIN | POLLRDBAND | POLLRDNORM | POLLPRI
  };

  std::string tempString;
  if (poll(&stdin_poll, 1, 0) == 1) {
    while (std::getline(std::cin, tempString)) {
      parseAndInsertTargets(tempString);
    }
  }
#endif // !WIN32
}

void TargetHandler::parseAndInsertTargets(const std::string & targetString) {
  std::regex hashRegex("[a-fA-F0-9]{32}");
  auto matchesBegin = std::sregex_iterator(targetString.begin(),
                                           targetString.end(), hashRegex);
  auto matchesEnd = std::sregex_iterator();

  for (auto i = matchesBegin; i != matchesEnd; ++i) {
    targets.emplace((*i).str(), 0);
  }
}
