// CopyRight 2019 360. All rights reserved.
// File   custom_aid_exp_mapping.cpp
// Date   2019-10-29 11:22:45
// Brief

#include "custom_aid_exp_mapping.h"
#include <fstream>
#include "util/string_utils.h"

namespace prediction {

CustomAidExpMapping CustomAidExpMapping::_instance;

CustomAidExpMapping::CustomAidExpMapping() { }

CustomAidExpMapping::~CustomAidExpMapping() { }

CustomAidExpMapping& CustomAidExpMapping::GetInstance() {
    return _instance;
}

void CustomAidExpMapping::LoadFromFile(const std::string & file_name) {
    std::ifstream mapping_file(file_name.c_str());
    if (!mapping_file.is_open()) {
        // // LOG_WARNING("[aid_exp_mapping] file open failed!");
        return;
    }

    // get raw list
    std::string line;
    int line_no = 0;

    std::vector<std::string> tmp_vector;
    while (std::getline(mapping_file, line)) {
        if (line.empty()) {
            continue;
        }
        if (line[0] == '#') {
            continue;
        }
        util::StringUtils::Split(line, ' ', tmp_vector);
        if (tmp_vector.size() < 2) {
            continue;
        }
        _aid_exp_mapping[tmp_vector[0]] = tmp_vector[1];
        line_no++;
    }
    mapping_file.close();
    // // LOG_INFO("[aid_exp_mapping] loaded size=%d", line_no);
}

std::string CustomAidExpMapping::GetExp(const std::string & aid) {
    std::string res;

    auto it = _aid_exp_mapping.find(aid);
    if (it != _aid_exp_mapping.end()) {
        res = it->second;
    }

    return res;
}

}
