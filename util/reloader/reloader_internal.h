// CopyRight 2019 360. All rights reserved.
// File   reloader_internal.h
// Date   2019-10-29 11:02:24
// Brief

#ifndef RELOADER_RELOADER_INTERNAL_H_
#define RELOADER_RELOADER_INTERNAL_H_

#include <memory>
#include <fstream>
#include <vector>
#include <unordered_map>

#include "util/string_utils.h"
#include "util/redis_client.h"
#include "util/reloader/reloader_parser.h"

namespace prediction {
namespace util {
namespace reloader {

using util::RedisClient;
using util::StringUtils;

template<typename KeyType, typename ValueType>
class DBInterface {
  public:
    DBInterface() { }
};

template<typename KeyType, typename ValueType>
class VectorDB : public DBInterface<KeyType, ValueType> {
 public:
  VectorDB() {
    _db = std::make_shared<std::vector<KeyType>>();
  }

  std::shared_ptr<std::vector<KeyType>> GetDB() {
    return _db;
  }

 private:
  std::shared_ptr<std::vector<KeyType>> _db;
};

template<typename KeyType, typename ValueType>
class MapDB : public DBInterface<KeyType, ValueType> {
 public:
  MapDB() {
    _db = std::make_shared<std::unordered_map<KeyType, ValueType>>();
  }

  std::shared_ptr<std::unordered_map<KeyType, ValueType>> GetDB() {
    return _db;
  }

 private:
  std::shared_ptr<std::unordered_map<KeyType, ValueType>> _db;
};

template<typename KeyType, typename ValueType>
class MapMapDb : public DBInterface<KeyType, ValueType> {
 public:
  using DBType = std::unordered_map<KeyType, std::unordered_map<int, ValueType>>;
  MapMapDb() {
    _db = std::make_shared<DBType>();
  }

  std::shared_ptr<DBType> GetDB() {
    return _db;
  }

 private:
  std::shared_ptr<DBType> _db;
};

template<typename KeyType, typename ValueType>
class MapVectorDb : public DBInterface<KeyType, ValueType> {
 public:
  using DBType = std::unordered_map<KeyType, std::vector<ValueType>>;

  MapVectorDb() {
    _db = std::make_shared<DBType>();
  }

  std::shared_ptr<DBType> GetDB() {
    return _db;
  }

  private:
    std::shared_ptr<DBType> _db;
};

template<typename KeyType, typename ValueType, typename ParserType>
class VectorReloader {
 public:
  VectorReloader(ReloaderMeta& reloader_meta)
      : val_sep_(reloader_meta.value_seprator),
        file_name_(reloader_meta.file_name) {
    _common_db = std::make_shared<VectorDB<KeyType, ValueType>>();
  }

  typedef std::vector<KeyType> DBType;

  std::shared_ptr<DBType> GetDB() {
    return _common_db->GetDB();
  }

  int Load() {
    if (file_name_.empty()) {
      LOG(INFO) << "file name is empty" << file_name_;
      return -1;
    }

    std::ifstream ifs(file_name_);
    if (!ifs.is_open()) {
      ifs.close();
      LOG(INFO) << "open error file:" << file_name_;
      return -1;
    }

    LOG(INFO) << "load file:" << file_name_;

    std::shared_ptr<DBType> db = _common_db->GetDB();

    KeyType key;
    ValueType value;

    ParserType parser;
    std::string line;
    std::vector<std::string> line_vec;
    while (std::getline(ifs, line)) {
      if (line.empty()) { continue; }

      line_vec.clear();
      StringUtils::Split(line, val_sep_, line_vec);

      if (line_vec.size() < 2) { continue; }

      if (parser.Parse(line_vec, key, value) != PARSER_OK) {
        continue;
      }

      // LOG(INFO) << "load value:" << key;

      db->emplace_back((key));
    }
    ifs.close();

    return 0;
  }

 private:
  std::shared_ptr<VectorDB<KeyType, ValueType>> _common_db;
  std::string val_sep_;
  std::string file_name_;
};

template<typename KeyType, typename ValueType, typename ParserType>
class VectorRedisReloader {
 public:
  typedef VectorDB<KeyType, ValueType> VectorType;
  typedef std::vector<KeyType> DBType;
  VectorRedisReloader(ReloaderMeta& reloader_meta)
      : kv_sep_(reloader_meta.key_seprator),
        val_sep_(reloader_meta.value_seprator),
        redis_key_(reloader_meta.redis_key) {
    _common_db = std::make_shared<VectorType>();
    redis_client_ = std::unique_ptr<RedisClient>(
        new RedisClient(reloader_meta.redis_host, reloader_meta.redis_port));
  }

  std::shared_ptr<DBType> GetDB() {
    return _common_db->GetDB();
  }

  int Load() {
    int list_len = redis_client_->llen(redis_key_);
    LOG(INFO) << "list:" << redis_key_ << " len:" << list_len;

    int ret_len = 0;
    int start = 0;
    int range = 1000;
    int count = 0;
    std::vector<std::string> items;
    std::vector<std::string> tmp_items;

    while (true) {
      tmp_items.clear();
      tmp_items = redis_client_->lrange(redis_key_, start, start + range);
      if (tmp_items.empty()) {
        break;
      }
      ret_len = tmp_items.size();
      count += ret_len;
      start += ret_len;
      items.insert(items.end(), tmp_items.begin(), tmp_items.end());
    }

    KeyType key;
    ValueType val;
    std::shared_ptr<DBType> db = _common_db->GetDB();

    ParserType parser(val_sep_);
    std::string line;
    std::vector<std::string> line_vec;

    // int debug_count = 5;
    for (auto& line : items) {
      if (line.empty()) {
        continue;
      }

      line_vec.clear();
      StringUtils::Split(line, kv_sep_, line_vec);
      val.clear();
      if (parser.Parse(line_vec, key, val) != PARSER_OK) {
        continue;
      }
      db->emplace_back(std::move(key));
    }

    return 0;
  }

 private:
  std::shared_ptr<VectorType> _common_db;
  std::string kv_sep_;
  std::string val_sep_;
  std::string redis_key_;
  std::unique_ptr<RedisClient> redis_client_;

};  // VectorRedisReloader


template<typename KeyType, typename ValueType, typename ParserType>
class MapReloader {
 public:
  MapReloader(std::string file_name)
      : file_name_(file_name) {
    _common_db = std::make_shared<MapDB<KeyType, ValueType>>();
  }

  typedef std::unordered_map<KeyType, ValueType> DBType;

  std::shared_ptr<DBType> GetDB() {
    return _common_db->GetDB();
  }

  int Load() {
    if (file_name_.empty()) {
      return -1;
    }

    std::ifstream ifs(file_name_);
    if (!ifs.is_open()) {
      ifs.close();
      return -1;
    }

    std::shared_ptr<DBType> db = _common_db->GetDB();

    KeyType key;
    ValueType value;

    ParserType parser;
    std::string line;
    std::vector<std::string> line_vec;
    while (std::getline(ifs, line)) {
      if (line.empty()) { continue; }

      line_vec.clear();
      StringUtils::Split(line, "\t", line_vec);

      if (line_vec.size() < 2) { continue; }

      if (parser.Parse(line_vec, key, value) != PARSER_OK) {
        continue;
      }
      db->insert(std::make_pair(key, value));
    }
    ifs.close();

    return 0;
  }

 private:
  std::shared_ptr<MapDB<KeyType, ValueType>> _common_db;
  std::string file_name_;
};

template<typename KeyType, typename ValueType, typename ParserType>
class MapVectorReloader {
 public:
  typedef MapVectorDb<KeyType, ValueType> MapType;
  typedef std::unordered_map<KeyType, std::vector<ValueType>> DBType;
  MapVectorReloader(ReloaderMeta& reloader_meta)
      : _kv_sep(reloader_meta.key_seprator),
        _val_sep(reloader_meta.value_seprator),
        file_name_(reloader_meta.file_name) {
    _common_db = std::make_shared<MapType>();
  }

  std::shared_ptr<DBType> GetDB() {
    return _common_db->GetDB();
  }

  int Load() {
    LOG(INFO) << "in map vactor load file:" << file_name_;
    if (file_name_.empty()) {
      LOG(ERROR) << "map vector load error file name is empty";
      return -1;
    }

    std::ifstream ifs(file_name_);
    if (!ifs.is_open()) {
      ifs.close();
      LOG(ERROR) << "map vector open file failed file:" << file_name_;
      return -1;
    }

    std::shared_ptr<DBType> db = _common_db->GetDB();
    KeyType key;
    std::vector<ValueType> val;
    ParserType parser(_val_sep);
    std::string line;
    std::vector<std::string> line_vec;
    while (std::getline(ifs, line)) {
      if (line.empty()) {
        continue;
      }

      line_vec.clear();
      StringUtils::Split(line, _kv_sep, line_vec);
      val.clear();
      if (parser.Parse(line_vec, key, val) != PARSER_OK) {
        continue;
      }

      db->insert(std::make_pair(key, std::move(val)));
    }

    ifs.close();

    return 0;
  }

 private:
  std::shared_ptr<MapType> _common_db;
  std::string _kv_sep;
  std::string _val_sep;
  std::string file_name_;
};

template<typename KeyType, typename ValueType, typename ParserType>
class MapVectorRedisReloader {
 public:
  typedef MapVectorDb<KeyType, ValueType> MapType;
  typedef std::unordered_map<KeyType, std::vector<ValueType>> DBType;
  MapVectorRedisReloader(ReloaderMeta& reloader_meta)
      : kv_sep_(reloader_meta.key_seprator),
        val_sep_(reloader_meta.value_seprator),
        redis_key_(reloader_meta.redis_key) {
    _common_db = std::make_shared<MapType>();
    // c++14 support make_unique
    // redis_client_ = std::make_unique<RedisClient>(
    redis_client_ = std::unique_ptr<RedisClient>(
        new RedisClient(reloader_meta.redis_host, reloader_meta.redis_port));
  }

  std::shared_ptr<DBType> GetDB() {
    return _common_db->GetDB();
  }

  int Load() {
    int list_len = redis_client_->llen(redis_key_);
    LOG(INFO) << "list:" << redis_key_ << " len:" << list_len;

    int ret_len = 0;
    int start = 0;
    int range = 1000;
    int count = 0;
    std::vector<std::string> items;
    std::vector<std::string> tmp_items;

    while (true) {
      tmp_items.clear();
      tmp_items = redis_client_->lrange(redis_key_, start, start + range);
      if (tmp_items.empty()) {
        break;
      }
      ret_len = tmp_items.size();
      count += ret_len;
      start += ret_len;
      items.insert(items.end(), tmp_items.begin(), tmp_items.end());
    }

    KeyType key;
    std::shared_ptr<DBType> db = _common_db->GetDB();
    std::vector<ValueType> val;

    ParserType parser(val_sep_);
    std::string line;
    std::vector<std::string> line_vec;

    // int debug_count = 5;
    for (auto& line : items) {
      if (line.empty()) {
        continue;
      }

      line_vec.clear();
      StringUtils::Split(line, kv_sep_, line_vec);
      val.clear();
      if (parser.Parse(line_vec, key, val) != PARSER_OK) {
        continue;
      }
      db->insert(std::make_pair(key, std::move(val)));
    }

    return 0;
  }

 private:
  std::shared_ptr<MapType> _common_db;
  std::string kv_sep_;
  std::string val_sep_;
  std::string redis_key_;
  std::unique_ptr<RedisClient> redis_client_;

};  // MapVectorRedisReloader

}  // namespace reloader
}  // namespace util
}  // namespace prediction

#endif  // RELOADER_RELOADER_INTERNAL_H_
