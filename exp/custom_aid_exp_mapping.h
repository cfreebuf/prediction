#ifndef CUSTOM_AID_EXP_MAPPING_H_
#define CUSTOM_AID_EXP_MAPPING_H_

#include <map>
#include <string>

namespace prediction {

class CustomAidExpMapping {
 private:
  static CustomAidExpMapping _instance;
  std::map<std::string, std::string> _aid_exp_mapping;

 private:
  CustomAidExpMapping();

 public:
  static CustomAidExpMapping & GetInstance();
  void LoadFromFile(const std::string & file_name);
  std::string GetExp(const std::string & aid);
  virtual ~CustomAidExpMapping();
};

}

#endif /* CUSTOM_AID_EXP_MAPPING_H_ */
