#include "rainbow_handler.hpp"

#include <fstream>

#include <mfl/out.hpp>

void RainbowHandler::load(TargetHandler & targetHandler) {
  if (cPath.empty() || targetHandler.targets.empty()) {
    return;
  }

  std::ifstream stream(cPath, std::ios::binary);
  if (stream.is_open()) {
    std::string entry;
    mfl::out::println("Loading rainbow table from: {:s}", cPath);

    std::uint64_t count = 0;
    Hash::KeyPair * keyPair;
    std::size_t blockSize = BUFFER_SIZE - (BUFFER_SIZE % sizeof(Hash::KeyPair));
    char data[BUFFER_SIZE];

    while (stream.good()) {
      stream.read(data, blockSize);

      auto targetCount = targetHandler.targets.size();
      for (std::size_t i = 0;
           i < blockSize && count < targetCount;
           i += sizeof(Hash::KeyPair)) {
        keyPair = reinterpret_cast<Hash::KeyPair *>(data + i);
        if (keyPair->value == 0) {
          continue;
        }

        auto search = targetHandler.targets.find(keyPair->hash);
        if (search != targetHandler.targets.end()) {
          search->second = keyPair->value;
          rainbow.emplace(search->first, keyPair->value);
          targetHandler.targets.erase(search);
          count++;
        }
      }
    }

    stream.close();
  } else {
    mfl::out::println(stderr, "Failed to load the rainbow table. Skipping..");
  }
}

void RainbowHandler::save(TargetHandler & targetHandler) {
  if (cPath.empty() || targetHandler.targets.empty()) {
    mfl::out::println("Not saving entries to the rainbow table");
    return;
  }

  std::fstream stream(cPath, std::ios::binary | std::ios::app);
  if (stream.is_open()) {
    mfl::out::println("Saving the rainbow table");

    std::uint64_t count = 0;
    for (const auto & entry : targetHandler.targets) {
      if (entry.second > 0) {
        stream.write((char *) &(entry.first), sizeof(Hash::Hash));
        stream.write((char *) &(entry.second), sizeof(Value));
        count++;
      }
    }

    stream.close();

    mfl::out::println("Saved {:d} new {:s} to the rainbow table at: {:s}",
                      count,
                      count == 1 ? "entry" : "entries",
                      cPath);
  } else {
    mfl::out::println(stderr, "Failed to save the rainbow table. Skipping..");
  }
}
