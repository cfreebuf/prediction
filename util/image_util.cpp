// CopyRight 2019 360. All rights reserved.
// File   image_util.cpp
// Date   2019-11-05 22:04:27
// Brief

#include <stdio.h>
#include <curl/curl.h>
// #include <curl/types.h>
#include <curl/easy.h>
#include <string>
#include <iostream>

#include "prediction/util/image_util.h"
#include "prediction/util/logging.h"

namespace prediction {

static size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
  size_t written;
  written = fwrite(ptr, size, nmemb, stream);
  return written;
}

util::Status DownloadImageFromURL(std::string url, std::string output) {
  int ret = -1;
  FILE* fp = NULL;
  CURL* curl = curl_easy_init();
  util::Status status;
  if (curl) {
    fp = fopen(output.c_str(), "wb");
    if (fp) {
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

      CURLcode rc = curl_easy_perform(curl);
      if (rc) {
        LOG(ERROR) << "Failed to download image, check url, url:" << url;
        status = util::Status(error::DOWNLOAD_IMAGE_ERROR, "Download image from internet error, check image url");
      }
    } else {
      LOG(ERROR) << "Failed to create output local file:" << output;
      status = util::Status(error::DOWNLOAD_IMAGE_ERROR, "Failed to create local output file");
    }
  } else {
    LOG(ERROR) << "Failed to init curl, url:" << url;
    status = util::Status(error::DOWNLOAD_IMAGE_ERROR, "Failed to init curl");
  }
  if (fp) {
    fclose(fp);
  }
  if (curl) {
    curl_easy_cleanup(curl);
  }
  return status;
}

}  // namespace prediction
