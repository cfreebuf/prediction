// CopyRight 2019 360. All rights reserved.
// File   common_define.h
// Date   2019-10-28 17:28:57
// Brief

#ifndef PREDICTION_SERVER_COMMON_COMMON_DEFINE_H_
#define PREDICTION_SERVER_COMMON_COMMON_DEFINE_H_

#include <memory>
#include <functional>
#include <unordered_map>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include "third_party/cityhash/include/city.h"

typedef boost::shared_mutex SharedMutex;
typedef boost::shared_lock<SharedMutex> ReadLock;
typedef boost::unique_lock<SharedMutex> WriteLock;

namespace prediction {

#define MAKE_HASH(key) CityHash64(key.c_str(), key.size())
#define MAKE_HASH2(key_buf, sz) CityHash64(key_buf, sz)
#define GEN_HASH2(h1, h2) (((h1) << 1) ^ (h2))
#define GEN_HASH3(h1, h2, h3) (((GEN_HASH2(h1, h2)) << 1) ^ (h3))

#define NUM2STR(num) StringUtils::Num2Str(num)

// Generate enum
#define GENERATE_ENUM(ENUM) ENUM,
// Generate text(upper case) array
#define GENERATE_STRING(STRING) #STRING,

#define STR(str) #str

#define TO_UPPER(str) \
do { \
    for_each(str.begin(), str.end(), [](char& in) { in = std::toupper(in); }); \
} while (0)
#define TO_LOWER(str) \
do { \
    for_each(str.begin(), str.end(), [](char& in) { in = std::tolower(in); }); \
} while (0)

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#ifdef __GNUC__
#define NOT_USED __attribute__ ((unused))
#else
#define NOT_USED
#endif

#define PREDICTION_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;         \
  void operator=(const TypeName&) = delete

}  // namespace prediction

#endif  // PREDICTION_SERVER_COMMON_COMMON_DEFINE_H_
