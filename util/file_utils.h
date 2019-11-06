// CopyRight 2019 360. All rights reserved.
// File   file_utils.h
// Date   2019-10-28 16:30:04
// Brief

#ifndef PREDICTION_SERVER_UTIL_FILE_UTILS_H_
#define PREDICTION_SERVER_UTIL_FILE_UTILS_H_

#include <libgen.h>
#include <string>
#include <vector>

namespace prediction {
namespace util {

class FileUtils {
 public:
  static std::vector<std::string> ListDir(const std::string& dir_name);
  static std::string BaseName(const std::string& file_name);
  static bool FileExists(const std::string& file_name);
  static int SaveFile(const std::string& file_name, const char* buf, int len);
  static int CopyFile(const std::string& srcfile, const std::string& newfile);
};

}  // namespace util 
}  // namespace prediction

#endif  // PREDICTION_SERVER_UTIL_FILE_UTILS_H_
