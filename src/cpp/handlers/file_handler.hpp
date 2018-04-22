#pragma once

#include <string>

#include "target_handler.hpp"
#include "rainbow_handler.hpp"

class FileHandler {
private:
  const std::string cPath;
public:
  FileHandler(const char * path) : cPath(path ? path : "") {}

  void load(TargetHandler & targetHandler);
  void save(const TargetHandler & targetHandler,
            const RainbowHandler & rainbow,
            const char * outputPath,
            int length);
};
