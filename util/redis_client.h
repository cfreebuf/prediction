// CopyRight 2019 360. All rights reserved.
// File   redis_client.h
// Date   2019-10-29 10:53:00
// Brief

#ifndef PREDICTION_UTIL_REDIS_CLIENT_H_
#define PREDICTION_UTIL_REDIS_CLIENT_H_

#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "prediction/third_party/hiredis/include/hiredis.h"

namespace prediction {
namespace util {

struct KeyValue{
    std::string key;
    std::string value;
};

class RedisClient {
public:
    RedisClient(std::string host, int port,
                std::string password = "", int timeout_ms = 1000);
    virtual ~RedisClient();

    std::string get(const std::string& key);
    int ping();
    int set(const std::string & key, const std::string& value);
    int setex(const std::string& key, const std::string& value,
              const int ttl_s);
    int setnx(const std::string& key, const std::string& value);
    int mset(const std::map<std::string, std::string> & values);
    int get(const std::string & key, std::string &value);

    int mget(const std::string& prefix,
             const std::vector<std::string>& keys,
             std::unordered_map<std::string, std::string> & res);

    int xmget(const std::string& prefix,
              const std::vector<std::string>& keys,
              std::unordered_map<std::string, std::string> & res);

    int smembers(const std::string & key, std::unordered_set<std::string>& res);

    // set binary data
    int set(const std::string& key, unsigned char * buffer, int size);
    int del(const std::string& key);

    std::vector<std::string> lrange(const std::string& key,
                                    int start, int stop);

    int llen(const std::string& key);

    int zcard(const std::string& key);
    int zcount(const std::string& key, const long min, const long max);

    int zadd(const std::string& key,
             std::unordered_map<std::string, std::string>& members);
    int zadd(const std::string& key,
             std::unordered_map<std::string, long>& members,
             std::unordered_map<std::string, int>& ret);
    int zadd(const std::string& key,
             std::vector<std::string>& members, long value, int ttl);

    int lpush(const std::string& key, std::vector<std::string>& values);
    int rpush(const std::string & key, const std::string & value);
    int set_expire(const std::string & key, int seconds);
    int ttl(const std::string& key);
    int select_db(int db_number);

    int hincrby(const std::string& key, const std::string & field, int inc);
    int hdel(const std::string& key, const std::string& field); 
    int hset(const std::string& key, const std::string& field,
             const std::string & value);
    int hset(const std::string& key, const std::string& field,
             unsigned char* buffer, int size);

    std::string hget(const std::string& key, const std::string& field);
    int hkeys(const std::string& hash_name, std::vector<std::string>& res);
    int hmset(const std::string& hash_name,
              std::unordered_map<std::string, std::string>& res);

    int hmget(const std::string& key, const std::vector<std::string>& fields,
              std::unordered_map<std::string, std::string>& res);

    int hgetall(const std::string& hash_name,
                std::unordered_map<std::string, std::string>& res);

    int hscan_count(const std::string& hash_name, std::string& cursor,
                    const std::string& count,
                    std::unordered_map<std::string, std::string>& res); 

    int zincrby(const std::string & key, const std::string & field, int inc);

    int zrange(const std::string & key, const int start, const int stop,
               std::vector<std::string> & result);
    int zrange(const std::string& key, const int start, const int stop,
               std::unordered_set<std::string> & result);

    int zrevrange(const std::string& key, const int start, const int stop,
                  std::vector<std::string> & result);

    int zrange_withscores(const std::string& key, const int start,
                          const int stop,
                          std::unordered_map<std::string, std::string>& result);
    int zrevrange_withscores(const std::string & key, const int start,
                             const int stop,
                             std::map<std::string, std::string> & result);

    int zrange_byscore(const std::string & key, const long min, const long max,
                       std::vector<std::string> & result, int limit = -1);
    int zrange_byscore(const std::string & key, const long min, const long max,
                       std::unordered_set<std::string> & result, int limit = -1);

    int zrevrange_byscore(const std::string& key, const long min,
                          const long max, std::vector<std::string>& result,
                          int limit = -1);
    int zrevrange_byscore(const std::string& key, const long min,
                          const long max,
                          std::unordered_set<std::string>& result,
                          int limit = -1);
    int zrem_byscore(const std::string& key, const long min, const long max);
    int zrem_byrank(const std::string& key, const long start, const long end);
    int zrem(const std::string& key, const std::vector<std::string> members);

    int zrange_byscore_withscores(const std::string& key, const long min,
                                  const long max,
                                  std::map<std::string, std::string>& result,
                                  int limit = -1, bool min_exclusive = false,
                                  bool max_exclusive = false);

    int zrevrange_byscore_withscores(const std::string& key, const long min,
                                     const long max,
                                     std::map<std::string, std::string>& result,
                                     int limit = -1, bool min_exclusive = false,
                                     bool max_exclusive = false);

    int zscore(const std::string & key, const std::string& member,
               std::string& score);

    int zscan_count(const std::string sortedset_name, std::string& cursor,
                    const std::string& count,
                    std::map<std::string, std::string> & res);
    int zscan_count(const std::string sortedset_name, std::string& cursor,
                    const std::string& count,
                    std::vector<KeyValue> & res);

    int hget(const std::string& key_predix, std::vector<std::string>& uids,
             std::string& field,
             std::unordered_map<std::string, std::string>& uids_tags);
    friend class redis_client_pool_t;
    int Connect();

private:

#if 0
    redisContext * _context;
#endif

    redisContext* _context;

    std::string _host;
    int _port;
    std::string _password;
    int _timeout_ms;
    static const int retry_num = 2;

    int CheckConnection();
    int ReConnect();
    int Auth(const std::string & password);
};  // RedisClient

}  // namespace util
}  // namespace prediction

#endif  // PREDICTION_UTIL_REDIS_CLIENT_H_
