// File   variable.h
// Author lidongming
// Date   2018-09-26 12:22:31
// Brief

#ifndef PREDICTION_SERVER_CHAIN_VARIABLE_H_
#define PREDICTION_SERVER_CHAIN_VARIABLE_H_

#include <strings.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <sstream>
#include <list>
#include <map>
#include <unordered_map>

// #include "json/json.h"

namespace prediction {

class Variable {
 public:
  enum val_type_t {
    TYPE_NULL = 0,
    TYPE_STR = 1,
    TYPE_INT = 2,
    TYPE_DOUBLE = 3,
    TYPE_BOOLEAN = 4,
    TYPE_LIST = 5,
    TYPE_MAP = 6,
    TYPE_LONG = 7,
    TYPE_UINT64 = 8,
  };

  static const double EPSILON;;
  static Variable NULLVAR;

 private:
  std::string _str_val;
  union {
    bool bool_val;
    int int_val;
    long long_val;
    double double_val;
    uint64_t uint64_val;
  } _scalar_val;

  std::list<Variable> _list_val;

  typedef std::unordered_map<std::string, Variable> variable_map;
  variable_map* _map_val_ptr;

  val_type_t _type;

 public:
  Variable() {
    _type = TYPE_UINT64;
    _scalar_val.uint64_val = 0;
    _map_val_ptr = NULL;
  }

  Variable(val_type_t type) : _type(type) {
    if (type == TYPE_MAP) { _map_val_ptr = new variable_map(); }
    else { _map_val_ptr = NULL; }
  }

  Variable(const std::string& val) {
    _type = TYPE_STR;
    _str_val = val;
    _map_val_ptr = NULL;
  }

  Variable(const char* val) {
    _type = TYPE_STR;
    _str_val = val;
    _map_val_ptr = NULL;
  }

  Variable(const bool bool_val) {
    _type = TYPE_BOOLEAN;
    _scalar_val.bool_val = bool_val;
    _map_val_ptr = NULL;
  }

  Variable(const int val) {
    _type = TYPE_INT;
    _scalar_val.int_val = val;
    _map_val_ptr = NULL;
  }

  Variable(const long val) {
    _type = TYPE_LONG;
    _scalar_val.long_val = val;
    _map_val_ptr = NULL;
  }

  Variable(const double val) {
    _type = TYPE_DOUBLE;
    _scalar_val.double_val = val;
    _map_val_ptr = NULL;
  }

  Variable(const variable_map& vmap) {
    _type = TYPE_MAP;
    _map_val_ptr = new variable_map(vmap);
  }

  Variable(const std::map<std::string, Variable>& vmap) {
    _type = TYPE_MAP;
    _map_val_ptr = new variable_map();
    for (auto& kv : vmap) {
      _map_val_ptr->insert(std::make_pair(kv.first, kv.second));
    }
  }

  Variable(const uint64_t val) {
    _type = TYPE_UINT64;
    _scalar_val.uint64_val = val;
    _map_val_ptr = NULL;
  }

  Variable(const Variable& val) {
    _type = val._type;
    _scalar_val = val._scalar_val;
    _str_val = val._str_val;
    if (_type == TYPE_LIST) { _list_val = val._list_val; }

    _map_val_ptr = NULL;
    if (_type == TYPE_MAP) {
      _map_val_ptr = new variable_map(*(val._map_val_ptr));
    }
  }

  ~Variable() {
    if (_map_val_ptr != NULL) {
      delete _map_val_ptr;
    }
  }

  static Variable CreateList() {
    Variable t;
    t._type = TYPE_LIST;
    return t;
  }

  static Variable CreateMap() {
    Variable t;
    t._type = TYPE_MAP;
    t._map_val_ptr = new variable_map();
    return t;
  }

  val_type_t type() const { return _type; }

  operator std::string() const { return ToString(); }

  std::string ToString() const {
    std::string res;

    switch (_type) {
      case TYPE_INT:
        res = std::to_string(_scalar_val.int_val);
        break;

      case TYPE_LONG:
        res = std::to_string(_scalar_val.long_val);
        break;

      case TYPE_DOUBLE:
        res = std::to_string(_scalar_val.double_val);
        break;

      case TYPE_STR:
        res = _str_val;
        break;

      case TYPE_BOOLEAN:
        if (_scalar_val.bool_val) { res = "true"; }
        else { res = "false"; }
        break;

#if 0
      case TYPE_LIST:
        {
          Json::Value json_node;
          for (auto& it : _list_val) {
            json_node.append(it.ToString());
          }
          Json::FastWriter fast_writer;
          res = fast_writer.write(json_node);
        }
        break;

      case TYPE_MAP:
        {
          Json::Value json_node;
          for (auto& kv : *_map_val_ptr) {
            json_node[kv.first] =  kv.second.ToString();
          }
          Json::FastWriter fast_writer;
          res = fast_writer.write(json_node);
        }
        break;

      case TYPE_UINT64:
        res = StringUtils::Num2Str(_scalar_val.uint64_val);
        break;
#endif

      default:
        break;
    }
    return res;
  }

#if 0
  operator Json::Value() const { return ToJson(); }

  Json::Value ToJson() const {
    Json::Value res;

    switch (_type) {
      case TYPE_INT:
        res = StringUtils::Num2Str(_scalar_val.int_val);
        break;

      case TYPE_LONG:
        res = StringUtils::Num2Str(_scalar_val.long_val);
        break;

      case TYPE_DOUBLE:
        res = StringUtils::Num2Str(_scalar_val.double_val);
        break;

      case TYPE_STR:
        res = _str_val;
        break;

      case TYPE_BOOLEAN:
        if (_scalar_val.bool_val) { res = "true"; }
        else { res = "false"; }
        break;

      case TYPE_LIST:
        {
          res = Json::Value(Json::arrayValue);
          for (auto& it : _list_val) {
            res.append(it.ToJson());
          }
        }
        break;

      case TYPE_MAP:
        {
          for (auto& kv : *_map_val_ptr) {
            res[kv.first] = kv.second.ToJson();
          }
        }
        break;

      case TYPE_UINT64:
        res = StringUtils::Num2Str(_scalar_val.uint64_val);
        break;

      default:
        break;
    }
    return res;
  }
#endif

  operator int() const {
    int res = 0;
    switch (_type) {
      case TYPE_INT:
        res = _scalar_val.int_val;
        break;

      case TYPE_LONG:
        res = (int) _scalar_val.long_val;
        break;

      case TYPE_DOUBLE:
        res = (int) _scalar_val.double_val;
        break;
      case TYPE_STR:
        res = atoi(_str_val.c_str());
        break;
      case TYPE_BOOLEAN:
        if (_scalar_val.bool_val) { res = 1; }
        else { res = 0; }
        break;
      case TYPE_UINT64:
        res = (uint64_t) _scalar_val.uint64_val;
        break;
      case TYPE_LIST:
      case TYPE_MAP:
      default:
        break;
    }
    return res;
  }

  operator double() const {
    double res = 0;
    switch (_type) {
      case TYPE_INT:
        res = _scalar_val.int_val;
        break;
      case TYPE_LONG:
        res = _scalar_val.long_val;
        break;
      case TYPE_DOUBLE:
        res = _scalar_val.double_val;
        break;
      case TYPE_STR:
        res = atof(_str_val.c_str());
        break;
      case TYPE_BOOLEAN:
        if (_scalar_val.bool_val) { res = 1; }
        else { res = 0; }
        break;
      case TYPE_UINT64:
        res = _scalar_val.uint64_val;
        break;
      case TYPE_LIST:
      case TYPE_MAP:
      default:
        break;
    }
    return res;
  }

  operator bool() const {
    double res = 0;

    switch (_type) {
      case TYPE_INT:
        res = (_scalar_val.int_val != 0);
        break;
      case TYPE_LONG:
        res = (_scalar_val.long_val != 0);
        break;
      case TYPE_DOUBLE:
        res = (_scalar_val.double_val != 0);
        break;
      case TYPE_STR:
        res = (_str_val != "0" && _str_val != ""
            && 0 != strcasecmp(_str_val.c_str(), "false"));
        break;
      case TYPE_BOOLEAN:
        res = _scalar_val.bool_val;
        break;
      case TYPE_LIST:
        res = !_list_val.empty();
        break;
      case TYPE_MAP:
        res = !_map_val_ptr->empty();
        break;
      case TYPE_UINT64:
        res = (_scalar_val.uint64_val != 0);
        break;
      default:
        break;
    }
    return res;
  }

  operator long() const {
    long res = 0;
    switch (_type) {
      case TYPE_INT:
        res = _scalar_val.int_val;
        break;

      case TYPE_LONG:
        res = _scalar_val.long_val;
        break;

      case TYPE_DOUBLE:
        res = (long) _scalar_val.double_val;
        break;
      case TYPE_STR:
        res = atol(_str_val.c_str());
        break;
      case TYPE_BOOLEAN:
        if (_scalar_val.bool_val) {
          res = 1;
        } else {
          res = 0;
        }
        break;
      case TYPE_UINT64:
        res = (uint64_t) _scalar_val.uint64_val;
        break;
      case TYPE_LIST:
      case TYPE_MAP:
      default:
        break;
    }
    return res;
  }

  operator uint64_t() const {
    uint64_t res = 0;
    switch (_type) {
      case TYPE_INT:
        res = _scalar_val.int_val;
        break;
      case TYPE_LONG:
        res = _scalar_val.long_val;
        break;
      case TYPE_DOUBLE:
        res = (uint64_t) _scalar_val.double_val;
        break;
      case TYPE_STR:
        res = stoull(_str_val);
        break;
      case TYPE_BOOLEAN:
        if (_scalar_val.bool_val) { res = 1; }
        else { res = 0; }
        break;
      case TYPE_UINT64:
        res = (uint64_t) _scalar_val.uint64_val;
        break;
      case TYPE_LIST:
      case TYPE_MAP:
      default:
        break;
    }
    return res;
  }

  operator std::list<Variable>() const {
    if (_type == TYPE_LIST) { return _list_val; }
    else { return std::list<Variable>(); }
  }

  operator const std::unordered_map<std::string, Variable>&() const {
    if (_type == TYPE_MAP) {
      return *_map_val_ptr;
    } else {
      static std::unordered_map<std::string, Variable> empty_map;
      return empty_map;
    }
  }

  bool operator == (const Variable& val) {
    bool res = false;
    if (_type == val._type) {
      switch (_type) {
        case TYPE_INT:
          res = (_scalar_val.int_val == val._scalar_val.int_val);
          break;
        case TYPE_DOUBLE:
          res = fabs(_scalar_val.double_val
              - val._scalar_val.double_val) < EPSILON;
          break;
        case TYPE_STR:
          res = (_str_val == val._str_val);
          break;
        case TYPE_BOOLEAN:
          res = (_scalar_val.bool_val == val._scalar_val.bool_val);
          break;
        case TYPE_UINT64:
          res = (_scalar_val.uint64_val == val._scalar_val.uint64_val);
          break;
        case TYPE_LIST:
        case TYPE_MAP:
        default:
          break;
      }
    }

    return res;
  }

  bool operator == (const bool bool_val) {
    return ((bool) *this == bool_val);
  }

  bool operator == (const int int_val) {
    return ((int) *this == int_val);
  }

  bool operator == (const double double_val) {
    double t = (double) *this;
    return (fabs(t - double_val) < EPSILON);
  }

  bool operator == (const std::string& str_val) {
    return ((std::string) *this == str_val);
  }

  bool operator == (const long long_val) {
    return ((long) *this == long_val);
  }

  bool operator == (const uint64_t uint64_val) {
    return ((uint64_t) *this == uint64_val);
  }

  Variable& operator = (const Variable& val) {
    _type = val._type;
    _scalar_val = val._scalar_val;
    _str_val = val._str_val;
    if (_type == TYPE_LIST) { _list_val = val._list_val; }
    if (_type == TYPE_MAP) {
      if (_map_val_ptr != NULL) { delete _map_val_ptr; }
      _map_val_ptr = new variable_map(*(val._map_val_ptr));
    }
    return *this;
  }

  bool Append(const Variable& val) {
    if (_type != TYPE_LIST) { return false; }
    _list_val.push_back(val);
    return true;
  }

  bool Set(const std::string& key, const Variable& val) {
    if (_type != TYPE_MAP) { return false; }
    (*_map_val_ptr)[key] = val;
    return true;
  }

  Variable& operator [](const std::string& key) {
    static Variable invalid_value((int) 0);
    if (_type != TYPE_MAP) { return invalid_value; }
    else { return (*_map_val_ptr)[key]; }
  }

  bool IsNull() const { return _type == TYPE_NULL; }
};  // Variable

}  // namespace prediction

#endif  // PREDICTION_SERVER_CHAIN_VARIABLE_H_
