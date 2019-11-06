// CopyRight 2019 360. All rights reserved.
// File   redis_client.cpp
// Date   2019-11-06 17:01:11
// Brief

#include "redis_client.h"
#include <cstdlib>
#include <sstream>
#include <string.h>

namespace prediction {
namespace util {

RedisClient::RedisClient(std::string host, int port,
    std::string password, int timeout_ms)
  :_context(NULL),  _host(host), _port(port),
  _password(password), _timeout_ms(timeout_ms) {
}

int RedisClient::CheckConnection() {
  if (_context == NULL || _context->err) {
    return  ReConnect();
  }
  return 0;
}

int RedisClient::ReConnect() {
  if (_context != NULL) {
    redisFree(_context);
    _context = NULL;
  }
  int res = Connect();
  if (res == 0 && !_password.empty()) {
    res = Auth(_password);
  }

  return res;
}

int RedisClient::Connect() {
  if (_context != NULL) {
    redisFree(_context);
    _context = NULL;
  }

  struct timeval tv = { _timeout_ms / 1000, _timeout_ms % 1000};

#if 0
  _context = redisConnectWithTimeout(_host.c_str(), _port, tv);
#endif

  _context = redisConnectWithTimeout(_host.c_str(), _port, tv);

  if (_context == NULL) {
    printf("failed to connect redis %s:%d", _host.c_str(), _port);
    return -1;
  }

  if (_context->err) {
    printf("failed to connect redis %s:%d", _host.c_str(), _port);
    redisFree(_context);
    _context = NULL;
    return -1;
  }

#if 0
  if (redisSetTimeout(_context,tv) != REDIS_OK) {
    printf("failed to set timeout");
    redisFree(_context);
    _context = NULL;
    return -1;
  }
#endif

  return 0;
}

int RedisClient::Auth(const std::string & password)
{
  if (_context == NULL || _context->err) {
    return -1;
  }

  redisReply * reply = (redisReply *) redisCommand(_context, "AUTH %s", password.c_str());
  if (reply == NULL) {
    return -1;
  }

  int res = -1;
  if(reply->str != NULL && strcmp(reply->str, "OK") == 0) {
    res = 0;
  }
  freeReplyObject(reply);

  return res;
}

int RedisClient::ping() {
  for (int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "PING");
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_STATUS || reply->str == NULL || strcmp(reply->str, "PONG") != 0) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

std::string RedisClient::get(const std::string & key)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "GET %s", key.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type == REDIS_REPLY_ERROR || (reply->type != REDIS_REPLY_STRING && reply->type != REDIS_REPLY_NIL)) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    std::string val = std::string(reply->str, reply->len);
    freeReplyObject(reply);
    return val;
  }

  return "";
}

int RedisClient::get(const std::string & key, std::string &value)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "GET %s", key.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type == REDIS_REPLY_ERROR || (reply->type != REDIS_REPLY_STRING && reply->type != REDIS_REPLY_NIL)) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    value.assign(reply->str, reply->len);
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}
int RedisClient::hincrby(const std::string & key, const std::string & field, int inc) {
  for (int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "HINCRBY %s %s %d", key.c_str(), field.c_str(), inc);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    int ret = reply->integer;
    freeReplyObject(reply);
    return ret;
  }
  return -1;
}
int RedisClient::hdel(const std::string & key, const std::string & field) {
  for (int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "HDEL %s %s", key.c_str(), field.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    int ret = reply->integer;
    freeReplyObject(reply);
    return ret;
  }
  return -1;
}
int RedisClient::hset(const std::string & key, const std::string & field, const std::string & value) {
  return hset(key, field, (unsigned char *) value.c_str(), value.size());
}

int RedisClient::hset(const std::string & key, const std::string & field, unsigned char * buffer, int size)
{
  if (buffer == NULL || size <= 0) {
    return -1;
  }

  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context,
        "HSET %s %s %b", key.c_str(), field.c_str(), buffer, size);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if(reply->integer != 1 && reply->integer != 0) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;

}

std::string RedisClient::hget(const std::string & key, const std::string & field)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "HGET %s %s", key.c_str(), field.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type == REDIS_REPLY_ERROR || (reply->type != REDIS_REPLY_STRING && reply->type != REDIS_REPLY_NIL)) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    std::string val = std::string(reply->str, reply->len);
    freeReplyObject(reply);
    return val;
  }

  return "";

}

int RedisClient::hmget(const std::string& key,const std::vector<std::string> & fields, std::unordered_map<std::string, std::string> & res)
{
  std::string cmd_str = "HMGET " + key;
  for (size_t i = 0; i < fields.size(); ++i) {
    cmd_str += " " + fields[i];
  }

  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, cmd_str.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    if (reply->elements != fields.size()) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
      redisReply * sub_reply = reply->element[i];
      if (sub_reply->type == REDIS_REPLY_STRING) {
        res[fields[i]] = std::string(sub_reply->str, sub_reply->len);
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;

}


int RedisClient::hkeys(const std::string& hash_name, std::vector<std::string> & res)
{
  std::string cmd_str = "HKEYS " + hash_name;
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, cmd_str.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      printf("result for HGETALL is not an array");
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i + 1 < reply->elements; i+=1) {
      redisReply * key = reply->element[i];
      if (key->type == REDIS_REPLY_STRING) {
        res.push_back(std::string(key->str, key->len));
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}


int RedisClient::hgetall(const std::string& hash_name, std::unordered_map<std::string, std::string> & res)
{
  std::string cmd_str = "HGETALL " + hash_name;
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, cmd_str.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      printf("result for HGETALL is not an array");
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i + 1 < reply->elements; i+=2) {
      redisReply * key = reply->element[i];
      redisReply * val = reply->element[i+1];
      if (key->type == REDIS_REPLY_STRING && val->type == REDIS_REPLY_STRING) {
        res[std::string(key->str, key->len)] = std::string(val->str, val->len);
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::hscan_count(const std::string& hash_name, std::string& cursor, const std::string& count,  std::unordered_map<std::string, std::string> & res) 
{
  if (hash_name.empty() || cursor.empty() || count.empty()) {
    return 0;
  }
  std::vector<const char *> argv;
  static char hscan[] = "HSCAN";
  static char command_count[] = "COUNT";
  argv.push_back(hscan);
  argv.push_back(hash_name.c_str());
  argv.push_back(cursor.c_str());
  argv.push_back(command_count);
  argv.push_back(count.c_str());

  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommandArgv(_context, argv.size(), argv.data(), NULL);
    if (NULL == reply) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 2) {
      printf("result for HSCAN is not an array and size = 2");
      freeReplyObject(reply);
      ReConnect();;
      continue;
    }

    redisReply * cur = reply->element[0];
    redisReply * hash_res = reply->element[1];
    if (cur->type != REDIS_REPLY_STRING || hash_res->type != REDIS_REPLY_ARRAY) {
      printf("result for HSCAN is invalid");
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    // result
    cursor = std::string(cur->str,cur->len);
    for (size_t j = 0; j + 1 < hash_res->elements; j+=2){
      redisReply * value = hash_res->element[j];
      redisReply * score = hash_res->element[j+1];
      if (value->type == REDIS_REPLY_STRING && score->type == REDIS_REPLY_STRING) {
        res[std::string(value->str, value->len)] = std::string(score->str, score->len);
      }
    }
    freeReplyObject(reply);
    return 0;
  }
  return -1;
}

int RedisClient::hmset(const std::string& hash_name, std::unordered_map<std::string, std::string> & vals)
{
  std::vector<const char *> argv;
  static char hmset[] = "HMSET";
  argv.push_back(hmset);
  argv.push_back(hash_name.c_str());

  std::unordered_map<std::string, std::string>::const_iterator it = vals.begin();
  for (; it != vals.end(); ++it) {
    argv.push_back(it->first.c_str());
    argv.push_back(it->second.c_str());
  }

  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommandArgv(_context, argv.size(), argv.data(), NULL);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if(reply->type == REDIS_REPLY_ERROR) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}


int RedisClient::mget(const std::string& key_prefix, const std::vector<std::string> & keys, std::unordered_map<std::string, std::string> & res)
{
  std::string cmd_str = "MGET";
  for (size_t i = 0; i < keys.size(); ++i) {
    cmd_str += " " + key_prefix +  keys[i];
  }

  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, cmd_str.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    if (reply->elements != keys.size()) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
      redisReply * sub_reply = reply->element[i];
      if (sub_reply->type == REDIS_REPLY_STRING) {
        res[keys[i]] = std::string(sub_reply->str, sub_reply->len);
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::setnx(const std::string & key, const std::string & value)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "SETNX %s %b", key.c_str(), value.c_str(), value.size());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if(reply->type != REDIS_REPLY_INTEGER) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    int res = reply->integer;
    freeReplyObject(reply);
    return res;
  }

  return -1;
}

int RedisClient::mset(const std::map<std::string, std::string> & values)
{
  if (values.size() == 0) {
    return -1;
  }
  const int splitter_count = 100;
  std::vector<const char*> argv;
  std::vector<std::string> params;
  std::map<std::string, std::string>::const_iterator it = values.begin();
  std::string cmd = "";
  int count = 0;
  while (it != values.end()) {
    if (params.size() == 0) {
      params.push_back("MSET");
    }
    std::string key = it->first;
    std::string value = it->second;
    count += 1;
    params.push_back(key);
    params.push_back(value);
    it++;
    if (count % splitter_count == 0 || it == values.end()) {
      // write to redis
      for (size_t i = 0; i < params.size(); ++i) {
        argv.push_back(params[i].c_str());
      }
      for(int i = 0; i < retry_num; ++i) {
        if (CheckConnection() != 0) {
          continue;
        }
        redisReply * reply = (redisReply *) redisCommandArgv(_context, argv.size(), &(argv[0]), NULL);
        if (reply == NULL) {
          ReConnect();
          continue;
        }
        if(strcmp(reply->str, "OK") != 0) {
          freeReplyObject(reply);
          ReConnect();
          continue;
        } else {
          freeReplyObject(reply);
          break;
        }
      }
      params.clear();
      argv.clear();
    }
  }

  return -1;
}

// note if a key has space in it, like "MGET 'Daily Mail'", should use redisCommandArgv, 
// and the keys need to stored when passing to redisCommandArgv
int RedisClient::xmget(const std::string& key_prefix, const std::vector<std::string> & keys, std::unordered_map<std::string, std::string> & res)
{
  std::vector<const char*> argv(keys.size()+1);
  std::vector<std::string> params;
  params.push_back("MGET");
  for (size_t i = 0; i < keys.size(); ++i) {
    std::string full_key = key_prefix + keys[i];
    params.push_back(full_key);
  }

  for (size_t i = 0; i < params.size(); ++i) {
    argv[i] = params[i].c_str();
  }

  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommandArgv(_context, argv.size(), &(argv[0]), NULL);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    if (reply->elements != keys.size()) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
      redisReply * sub_reply = reply->element[i];
      if (sub_reply->type == REDIS_REPLY_STRING) {
        res[keys[i]] = std::string(sub_reply->str, sub_reply->len);
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}
int RedisClient::smembers(const std::string & key, std::unordered_set<std::string>& res)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "SMEMBERS %s", key.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
      if (reply->element[i]->str != NULL) {
        res.insert(reply->element[i]->str);
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::set(const std::string & key, unsigned char * buffer, int size)
{
  if (buffer == NULL || size <= 0) {
    return -1;
  }

  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "SET %s %b", key.c_str(), buffer, size);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if(reply->str == NULL || strcmp(reply->str, "OK") != 0) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::set(const std::string & key, const std::string & value)
{
  return set(key, (unsigned char *) value.c_str(), value.size());
}

int RedisClient::setex(const std::string& key, const std::string& value, const int ttl_s) {
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context,
        "SETEX %s %d %b", key.c_str(), ttl_s, value.c_str(), value.size());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if(reply->str == NULL || strcmp(reply->str, "OK") != 0) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::del(const std::string & key)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "DEL %s", key.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_INTEGER) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    int len = reply->integer;
    freeReplyObject(reply);
    return len;
  }

  return -1;
}

std::vector<std::string> RedisClient::lrange(const std::string & key, int start, int stop)
{
  std::vector<std::string> res;
  for(int i = 0; i < retry_num; ++i) {
    res.clear();
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "LRANGE %s %d %d", key.c_str(), start, stop);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
      if (reply->element[i]->str != NULL) {
        res.push_back(reply->element[i]->str);
      }
    }
    freeReplyObject(reply);
    break;
  }

  return res;
}

int RedisClient::llen(const std::string &key)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "LLEN %s", key.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_INTEGER) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    int len = reply->integer;
    freeReplyObject(reply);
    return len;
  }

  return -1;
}

int RedisClient::zcard(const std::string& key)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "ZCARD %s", key.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_INTEGER) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    int len = reply->integer;
    freeReplyObject(reply);
    return len;
  }

  return -1;
}

int RedisClient::zcount(const std::string& key, long min, long max) {
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "ZCOUNT %s %ld %ld", key.c_str(), min, max);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_INTEGER) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    int len = reply->integer;
    freeReplyObject(reply);
    return len;
  }

  return -1;
}

int RedisClient::zincrby(const std::string & key, const std::string & field, int inc) {
  for (int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "ZINCRBY %s %d %s", key.c_str(), inc, field.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    int ret = reply->integer;
    freeReplyObject(reply);
    return ret;
  }
  return -1;
}

int RedisClient::zadd(const std::string & key, std::unordered_map<std::string, std::string>& members)
{
  if (key.empty() || members.empty()) {
    return 0;
  }

  std::vector<const char *> argv;
  static char zadd[] = "ZADD";
  argv.push_back(zadd);
  argv.push_back(key.c_str());

  std::unordered_map<std::string, std::string>::const_iterator it = members.begin();
  for (; it != members.end(); ++it) {
    argv.push_back(it->second.c_str());
    argv.push_back(it->first.c_str());
  }

  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommandArgv(_context, argv.size(), argv.data(), NULL);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if(reply->type == REDIS_REPLY_ERROR) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::zadd(const std::string & key, std::unordered_map<std::string, long>& members, std::unordered_map<std::string, int>& ret)
{
  if (key.empty() || members.empty()) {
    return 0;
  }

  std::vector<std::string> id_vec;
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }

    id_vec.clear();
    int res = 0;
    for(auto it = members.begin(); it != members.end(); ++it) {
      id_vec.push_back(it->first);
      res = redisAppendCommand(_context, "ZADD %s %d %s", key.c_str(), it->second, it->first.c_str());
      if (res != 0) {
        ReConnect();
        break;
      }
    }

    if (res != 0) {
      continue;
    }

    redisReply * reply;
    for(auto it = id_vec.begin(); it != id_vec.end(); ++it) {
      reply = NULL;
      res = redisGetReply(_context, (void**)&reply);
      if (res != 0 || reply == NULL) {
        if (reply != NULL) {
          freeReplyObject(reply);
        }
        ReConnect();
        res = -1;
        break;
      }
      if (reply->type != REDIS_REPLY_INTEGER) {
        freeReplyObject(reply);
        ReConnect();
        res = -1;
        break;
      }
      ret[*it] = reply->integer;
      freeReplyObject(reply);
    }

    if (res == 0) {
      return 0;
    }
  }

  return -1;
}

int RedisClient::zadd(const std::string& key, std::vector<std::string>& members, long value, int ttl)
{
  if (key.empty() || members.empty()) {
    return 0;
  }

  std::vector<std::string> id_vec;
  id_vec.reserve(members.size()+3);
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }

    id_vec.clear();
    int res = 0;
    for(auto it = members.begin(); it != members.end(); ++it) {
      id_vec.push_back(*it);
      res = redisAppendCommand(_context, "ZADD %s %ld %s", key.c_str(), value, it->c_str());
      if (res != 0) {
        ReConnect();
        break;
      }
    }
    res = redisAppendCommand(_context, "EXPIRE %s %d", key.c_str(), ttl);
    id_vec.push_back("ttl");
    if (res != 0) {
      continue;
    }

    redisReply * reply;
    for(auto it = id_vec.begin(); it != id_vec.end(); ++it) {
      reply = NULL;
      res = redisGetReply(_context, (void**)&reply);
      if (res != 0 || reply == NULL) {
        if (reply != NULL) {
          freeReplyObject(reply);
        }
        ReConnect();
        res = -1;
        break;
      }
      if (reply->type != REDIS_REPLY_INTEGER) {
        freeReplyObject(reply);
        ReConnect();
        res = -1;
        break;
      }
      freeReplyObject(reply);
    }

    if (res == 0) {
      return 0;
    }
  }

  return -1;
}

int RedisClient::lpush(const std::string& key, std::vector<std::string>& values) {
  std::vector<const char *> argv;
  static char lpush[] = "LPUSH";
  argv.push_back(lpush);
  argv.push_back(key.c_str());

  std::vector<std::string>::const_iterator it = values.begin();
  for (; it != values.end(); ++it) {
    argv.push_back(it->c_str());
  }

  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommandArgv(_context, argv.size(), argv.data(), NULL);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_INTEGER) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::rpush(const std::string & key, const std::string & value)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "RPUSH %s %s", key.c_str(), value.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_INTEGER) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::set_expire(const std::string & key, int seconds)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "EXPIRE %s %d", key.c_str(), seconds);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_INTEGER) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::ttl(const std::string& key)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "TTL %s ", key.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_INTEGER) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    int ttl = reply->integer;
    freeReplyObject(reply);
    return ttl;
  }

  return -1;
}

int RedisClient::select_db(int db_number)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "SELECT %d", db_number);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if(reply->str == NULL || strcmp(reply->str, "OK") != 0) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::zrange(const std::string & key, const int start, const int stop, std::vector<std::string> & result)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "ZRANGE %s %d %d", key.c_str(), start, stop);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
      if (reply->element[i]->str != NULL) {
        result.push_back(reply->element[i]->str);
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::zrange(const std::string & key, const int start, const int stop, std::unordered_set<std::string> & result)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "ZRANGE %s %d %d", key.c_str(), start, stop);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
      if (reply->element[i]->str != NULL) {
        result.insert(reply->element[i]->str);
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::zrange_withscores(const std::string & key, const int start, const int stop, std::unordered_map<std::string, std::string> & result)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "ZRANGE %s %d %d withscores", key.c_str(), start, stop);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i + 1 < reply->elements; i += 2) {
      if (reply->element[i]->str != NULL && reply->element[i+1]->str != NULL) {
        result[reply->element[i]->str] = reply->element[i+1]->str;
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::zrange_byscore(const std::string & key, const long min, const long max, std::vector<std::string> & result, int limit)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = NULL;
    if (limit > 0) {
      reply = (redisReply *) redisCommand(_context, "ZRANGEBYSCORE %s %d %d LIMIT 0 %d", key.c_str(), min, max, limit);
    } else {
      reply = (redisReply *) redisCommand(_context, "ZRANGEBYSCORE %s %d %d", key.c_str(), min, max);
    }
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
      if (reply->element[i]->str != NULL) {
        result.push_back(reply->element[i]->str);
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}
int RedisClient::zrange_byscore(const std::string & key, const long min, const long max, std::unordered_set<std::string> & result, int limit)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = NULL;
    if (limit > 0) {
      reply = (redisReply *) redisCommand(_context, "ZRANGEBYSCORE %s %d %d LIMIT 0 %d", key.c_str(), min, max, limit);
    } else {
      reply = (redisReply *) redisCommand(_context, "ZRANGEBYSCORE %s %d %d", key.c_str(), min, max);
    }
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
      if (reply->element[i]->str != NULL) {
        result.insert(reply->element[i]->str);
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::zrevrange(const std::string & key, const int start, const int stop, std::vector<std::string> & result)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "ZREVRANGE %s %d %d", key.c_str(), start, stop);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
      if (reply->element[i]->str != NULL) {
        result.push_back(reply->element[i]->str);
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::zrevrange_withscores(const std::string & key, const int start, const int stop, std::map<std::string, std::string> & result)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "ZREVRANGE %s %d %d withscores", key.c_str(), start, stop);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i + 1 < reply->elements; i += 2) {
      if (reply->element[i]->str != NULL && reply->element[i+1]->str != NULL) {
        result[reply->element[i]->str] = reply->element[i+1]->str;
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::zrevrange_byscore(const std::string & key, const long min, const long max, std::vector<std::string> & result, int limit)
{
  redisReply * reply = NULL;
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    if (limit > 0) {
      reply = (redisReply *) redisCommand(_context, "ZREVRANGEBYSCORE %s %d %d LIMIT 0 %d", key.c_str(), max, min, limit);
    } else {
      reply = (redisReply *) redisCommand(_context, "ZREVRANGEBYSCORE %s %d %d", key.c_str(), max, min);
    }
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    result.clear();
    result.reserve(reply->elements);
    for (size_t i = 0; i < reply->elements; ++i) {
      if (reply->element[i]->str != NULL) {
        result.push_back(reply->element[i]->str);
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}
int RedisClient::zrevrange_byscore(const std::string & key, const long min, const long max, std::unordered_set<std::string> & result, int limit)
{
  redisReply * reply = NULL;
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    if (limit > 0) {
      reply = (redisReply *) redisCommand(_context, "ZREVRANGEBYSCORE %s %d %d LIMIT 0 %d", key.c_str(), max, min, limit);
    } else {
      reply = (redisReply *) redisCommand(_context, "ZREVRANGEBYSCORE %s %d %d", key.c_str(), max, min);
    }
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    result.clear();
    result.reserve(reply->elements);
    for (size_t i = 0; i < reply->elements; ++i) {
      if (reply->element[i]->str != NULL) {
        result.insert(reply->element[i]->str);
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::zrem_byscore(const std::string& key, const long min, const long max) {
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "ZREMRANGEBYSCORE %s %ld %ld", key.c_str(), min, max);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::zrem_byrank(const std::string& key, const long start, const long end) {
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "ZREMRANGEBYRANK %s %ld %ld", key.c_str(), start, end);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::zrem(const std::string& key, const std::vector<std::string> members) {
  if (key.empty() || members.empty()) {
    return 0;
  }

  std::vector<const char *> argv;
  static char zrem[] = "ZREM";
  argv.push_back(zrem);
  argv.push_back(key.c_str());

  auto it = members.begin();
  for (; it != members.end(); ++it) {
    argv.push_back((*it).c_str());
  }

  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommandArgv(_context, argv.size(), argv.data(), NULL);
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if(reply->type == REDIS_REPLY_ERROR) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::zrange_byscore_withscores(const std::string & key, const long min, const long max, std::map<std::string, std::string> & result, int limit, bool min_exclusive, bool max_exclusive)
{
  for (int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    std::string cmd_str = "ZRANGEBYSCORE %s %ld %ld withscores";
    if (min_exclusive && !max_exclusive) {
      cmd_str = "ZRANGEBYSCORE %s (%ld %ld withscores";
    } else if (!min_exclusive && max_exclusive) {
      cmd_str = "ZRANGEBYSCORE %s %ld (%ld withscores";
    } else if (min_exclusive && max_exclusive) {
      cmd_str = "ZRANGEBYSCORE %s (%ld (%ld withscores";
    }
    redisReply * reply = NULL;
    if (limit > 0) {
      cmd_str += " LIMIT 0 %d";
      reply = (redisReply *) redisCommand(_context, cmd_str.c_str(), key.c_str(), min, max, limit);
    } else {
      reply = (redisReply *) redisCommand(_context, cmd_str.c_str(), key.c_str(), min, max);
    }
    if (reply == NULL) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i + 1 < reply->elements; i += 2) {
      if (reply->element[i]->str != NULL && reply->element[i+1]->str != NULL) {
        result[reply->element[i]->str] = reply->element[i+1]->str;
      }
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::zrevrange_byscore_withscores(const std::string & key, const long min, const long max, std::map<std::string, std::string> & result, int limit, bool min_exclusive, bool max_exclusive)
{
  for (int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    std::string cmd_str = "ZREVRANGEBYSCORE %s %ld %ld withscores";
    if (min_exclusive && !max_exclusive) {
      cmd_str = "ZREVRANGEBYSCORE %s %ld (%ld withscores";
    } else if (!min_exclusive && max_exclusive) {
      cmd_str = "ZREVRANGEBYSCORE %s (%ld %ld withscores";
    } else if (min_exclusive && max_exclusive) {
      cmd_str = "ZREVRANGEBYSCORE %s (%ld (%ld withscores";
    }
    redisReply * reply = NULL;
    if (limit > 0) {
      cmd_str += " LIMIT 0 %d";
      reply = (redisReply *) redisCommand(_context, cmd_str.c_str(), key.c_str(), max, min, limit);
    } else {
      reply = (redisReply *) redisCommand(_context, cmd_str.c_str(), key.c_str(), max, min);
    }
    if (reply->type != REDIS_REPLY_ARRAY) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    for (size_t i = 0; i + 1 < reply->elements; i += 2) {
      if (reply->element[i]->str != NULL && reply->element[i+1]->str != NULL) {
        result[reply->element[i]->str] = reply->element[i+1]->str;
      }
    }
    freeReplyObject(reply);
    return 0;
  }
  return -1;
}
int RedisClient::zscore(const std::string & key, const std::string & member, std::string & score)
{
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommand(_context, "ZSCORE %s %s", key.c_str(), member.c_str());
    if (reply == NULL) {
      ReConnect();
      continue;
    }

    if(reply->type == REDIS_REPLY_ERROR) {
      freeReplyObject(reply);
      ReConnect();
      continue;
    }

    if (reply->type == REDIS_REPLY_NIL) {
      score = "REDIS_NIL";
    } else if (reply->type == REDIS_REPLY_STRING) {
      score = std::string(reply->str, reply->len);
    }
    freeReplyObject(reply);
    return 0;
  }

  return -1;
}

int RedisClient::zscan_count(const std::string sortedset_name, std::string& cursor, const std::string& count, std::map<std::string, std::string> & res) {
  if (sortedset_name.empty() || cursor.empty() || count.empty()) {
    return 0;
  }
  std::vector<const char *> argv;
  static char zscan[] = "ZSCAN";
  static char command_count[] = "COUNT";
  argv.push_back(zscan);
  argv.push_back(sortedset_name.c_str());
  argv.push_back(cursor.c_str());
  argv.push_back(command_count);
  argv.push_back(count.c_str());
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommandArgv(_context, argv.size(), argv.data(), NULL);
    if (NULL == reply) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 2) {
      printf("result for ZSCAN is not an array and size = 2");
      freeReplyObject(reply);
      ReConnect();;
      continue;
    }

    redisReply * cur = reply->element[0];
    redisReply * sortedset_res = reply->element[1];
    if (cur->type != REDIS_REPLY_STRING || sortedset_res->type != REDIS_REPLY_ARRAY) {
      printf("result for ZSCAN is invalid");
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    // result
    cursor = std::string(cur->str,cur->len);
    for (size_t j = 0; j + 1 < sortedset_res->elements; j+=2){
      redisReply * key = sortedset_res->element[j];
      redisReply * val = sortedset_res->element[j+1];
      if (key->type == REDIS_REPLY_STRING && val->type == REDIS_REPLY_STRING) {
        res[std::string(key->str, key->len)] = std::string(val->str, val->len);
      }
    }
    freeReplyObject(reply);
    return 0;
  }
  return -1;

}
int RedisClient::zscan_count(const std::string sortedset_name, std::string& cursor, const std::string& count, std::vector<KeyValue> & res)
{
  if (sortedset_name.empty() || cursor.empty() || count.empty()) {
    return 0;
  }
  std::vector<const char *> argv;
  static char zscan[] = "ZSCAN";
  static char command_count[] = "COUNT";
  argv.push_back(zscan);
  argv.push_back(sortedset_name.c_str());
  argv.push_back(cursor.c_str());
  argv.push_back(command_count);
  argv.push_back(count.c_str());
  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }
    redisReply * reply = (redisReply *) redisCommandArgv(_context, argv.size(), argv.data(), NULL);
    if (NULL == reply) {
      ReConnect();
      continue;
    }
    if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 2) {
      printf("result for ZSCAN is not an array and size = 2");
      freeReplyObject(reply);
      ReConnect();;
      continue;
    }

    redisReply * cur = reply->element[0];
    redisReply * sortedset_res = reply->element[1];
    if (cur->type != REDIS_REPLY_STRING || sortedset_res->type != REDIS_REPLY_ARRAY) {
      printf("result for ZSCAN is invalid");
      freeReplyObject(reply);
      ReConnect();
      continue;
    }
    // result
    cursor = std::string(cur->str,cur->len);
    for (size_t j = 0; j + 1 < sortedset_res->elements; j+=2){
      redisReply * key = sortedset_res->element[j];
      redisReply * val = sortedset_res->element[j+1];
      KeyValue keyvalue;
      keyvalue.key = std::string(key->str, key->len);
      keyvalue.value = std::string(val->str, key->len);
      if (key->type == REDIS_REPLY_STRING && val->type == REDIS_REPLY_STRING) {
        res.emplace_back(keyvalue);
        //res[std::string(key->str, key->len)] = std::string(val->str, val->len);
      }
    }
    freeReplyObject(reply);
    return 0;
  }
  return -1;
  return 0;
}
int RedisClient::hget(const std::string& key_predix, std::vector<std::string>& uids, std::string& field, std::unordered_map<std::string, std::string>& uids_tags)
{
  if (uids.empty() || field.empty()) {
    return 0;
  }

  for(int i = 0; i < retry_num; ++i) {
    if (CheckConnection() != 0) {
      continue;
    }

    int res = 0;
    for(int it = 0; it < uids.size(); ++it) {
      std::string redis_key = key_predix + uids[it];
      res = redisAppendCommand(_context, "HGET %s %s",
          redis_key.c_str(), field.c_str());
      if (res != 0) {
        ReConnect();
        break;
      }
    }

    if (res != 0) {
      continue;
    }

    redisReply * reply;
    for(int it = 0; it < uids.size(); ++it) {
      reply = NULL;
      res = redisGetReply(_context, (void**)&reply);
      if (res != 0 || reply == NULL) {
        if (reply != NULL) {
          freeReplyObject(reply);
        }
        ReConnect();
        res = -1;
        break;
      }
      if (reply->type == REDIS_REPLY_ERROR ||
          (reply->type != REDIS_REPLY_STRING
           && reply->type != REDIS_REPLY_NIL)) {
        freeReplyObject(reply);
        ReConnect();
        res = -1;
        break;
      }
      uids_tags[uids[it]] = std::string(reply->str, reply->len);
      freeReplyObject(reply);
    }

    if (res == 0) {
      return 0;
    }
  }
  return -1;
}

RedisClient::~RedisClient() {
  if (_context != NULL) {
    redisFree(_context);
    _context = NULL;
  }
}

}  // namespace util
}  // namespace prediction
