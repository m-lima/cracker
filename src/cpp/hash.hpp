#pragma once

#include <ostream>

#include <mfl/string.hpp>

using Value = std::uint64_t;

namespace Hash {
#pragma pack(push, 1)

  struct Hash {
    union {
      uint8_t hashBytes[16];
      uint64_t hashLongs[2];
    };

    std::size_t operator()(const Hash & hash) const {
      return hash.hashLongs[0];
    }

    bool operator<(const Hash & hash) const {
      return hashLongs[0] < hash.hashLongs[0];
    }

    bool operator>(const Hash & hash) const {
      return hashLongs[0] > hash.hashLongs[0];
    }

    bool operator==(const Hash & hash) const {
      return hashLongs[0] == hash.hashLongs[0];
    }

    friend std::ostream & operator<<(std::ostream & out, Hash hash) {
      char buffer[33];
      for (int i = 0; i < 16; i++) {
        std::sprintf(buffer + i * 2, "%02x", hash.hashBytes[i]);
      }
      buffer[32] = 0;

      return out << buffer;
    }

    Hash() = default;

    Hash(const unsigned char * bytes) {
      std::memcpy(hashBytes, bytes, 16);
    }

    Hash(const std::string & hashString) {
      for (int i = 0; i < 16; ++i) {
        hashBytes[i] =
            (mfl::string::toUint8(hashString.at(i * 2)) << 4)
            + mfl::string::toUint8(hashString.at(i * 2 + 1));
      }
    }

  };

  struct KeyPair {
    Hash hash;
    Value value;
  };
#pragma pack(pop)

  struct HashComparator {
    bool operator()(const Hash & hash1, const Hash & hash2) const {
      return hash1.hashLongs[0] == hash2.hashLongs[0]
             && hash1.hashLongs[1] == hash2.hashLongs[1];
    }
  };
}
