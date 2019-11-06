// CopyRight 2019 360. All rights reserved.
// File   monitor.cpp
// Date   2019-10-28 16:44:38
// Brief

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <error.h>
#include <thread>
#include <sstream>
#include <iostream>
#include "util/time_utils.h"
#include "util/monitor.h"

namespace prediction {
namespace util {
namespace monitor {

Monitor::Monitor() : monitor_data_(NULL), init_flag_(false) {
    last_update_minute_ = TimeUtils::GetCurrentMinute();
}

Monitor::~Monitor(){ 
    if (monitor_data_ != NULL) {
        monitor_data_ = NULL;
    }
    UpdatePara* data = NULL;
    while (!queue_.empty()) {
        if (queue_.pop(data) && data) {
            delete data;
            data = NULL;
        }
    }
}

int Monitor::InitMonitorData(MonitorData*& monitor_data) {
    int ret = 0;
    monitor_data = NULL; 
    FILE* f = fopen(lock_file_.c_str(), "w+");
    if (f == NULL) {
      std::cerr << "open lock file faild:" << lock_file_;
        return -1; 
    }     
    int flag = -1;
    flag = flock(f->_fileno, LOCK_EX);
    if (flag != 0) {
      std::cerr << "can't flock file:" << lock_file_ << " flag:" << flag;
        fclose(f);
        return -1;
    }
    std::string file_name = mmap_file_;
    int fd = open(file_name.c_str(), O_CREAT|O_RDWR, 0644);
    if (fd < 0) {
      std::cerr << "mmap file open faild"; 
        fclose(f); 
        return -1;
    }

    std::cout << "checking mmap file:" << mmap_file_;

    lseek(fd, sizeof(MonitorData), SEEK_SET);

    char md5[100] = "\nsomemd5sum : a3ccded2bb22c2e57c5406a7146f72b9\n";
    char buf[100] = { 0 };
    size_t md5_len = strlen(md5);
    size_t read_len = read(fd, buf, md5_len); 
    if (read_len != md5_len || strcmp(buf, md5) != 0) {
         // LOG_INFO("----------reset mmap %d---------", getpid());
         // write new monitor data
         lseek(fd, 0, SEEK_SET);
         MonitorData* d = (MonitorData*)calloc(1, sizeof(MonitorData));
         d->Init();
         write(fd, (void*)d, sizeof(MonitorData));
         free(d);
         d = NULL;
         // write md5 code
         lseek(fd, sizeof(MonitorData), SEEK_SET);
         write(fd, md5, strlen(md5));
     } else {
         // LOG_INFO("mmap md5 check is OK [%d]", getpid());
     }
            
     // get mmap data
     MonitorData* data = (MonitorData*)mmap(NULL, sizeof(MonitorData),
             PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
     if (data == MAP_FAILED) {
       std::cerr << "mmap data get failed";
         ret = -1; 
     } 
     data->Init();
     monitor_data = data;
     
     close(fd);
     flock(f->_fileno, LOCK_UN);
     fclose(f); 
     return ret;  
}

int Monitor::Init(const std::string& db_file, const std::string& monitor_file, long interval) {
    monitor_status_file_ = monitor_file;
    mmap_file_ = db_file;
    // use basename
    lock_file_ = db_file.substr(0, db_file.find_last_of('/') + 1) + ".monitor.db.lock";
    update_interval_ = interval; 
    int ret = InitMonitorData(monitor_data_);
    assert(ret == 0);    
    init_flag_ = true;
    return 0;
}

void Monitor::MonitorThread() {
    uint64_t last_update_time = GetFloorCurrentSeconds(update_interval_);
    std::unordered_map<std::string, MonitorItem> buffer;
    while (true) {
        uint64_t now = GetFloorCurrentSeconds(update_interval_);
        // write buffer to mmap
        if (now >= (update_interval_ + last_update_time)) {
            last_update_time = now;
            for (auto& kv : buffer) {
                monitor_data_->Lock();
                monitor_data_->Update(kv.first.c_str(), kv.second);
                monitor_data_->UnLock();
            }
            buffer.clear();
        } 

        // save metrics to buffer
        int times = 1000;  
        while (!queue_.empty() && --times > 0) {
            UpdatePara* data = NULL; 
            if (!queue_.pop(data)) {
                continue;
            }
            buffer[data->key].AddValue(data->opertation, data->val, last_update_time);    
            delete data;
            data = NULL;
        }  
        if (queue_.empty()) { usleep(100); } 
    }
}
void Monitor::DumpThread() {
    int current_minute = 0;
    std::string monitor_value;
    while (1) {
        current_minute = TimeUtils::GetCurrentMinute();
        if (current_minute != last_update_minute_) {
            last_update_minute_ = current_minute;
            FILE* f = fopen(monitor_status_file_.c_str(), "w+");
            if (f == NULL) {
                usleep(10000);
                continue;
            }
            int flag = flock(f->_fileno, LOCK_EX);
            if (flag != 0) {
                fclose(f);
                usleep(10000);
                continue;
            }
            Dump(&monitor_value);
            fwrite(monitor_value.c_str(), monitor_value.size(), 1, f);
            fclose(f);
            flock(f->_fileno, LOCK_UN);
            // LOG_INFO("dump monitor status:%s", monitor_data_->RawDump().c_str());
            sleep(update_interval_);
        } else {
            usleep(10000);
        }
    }
}

int Monitor::Start() {
    int ret = 0;
#if 0
    if (init_flag_) {
        ret = pthread_create(&_monitor_thread, NULL, Monitor::thread_func, this); 
        pthread_detach(_monitor_thread);
    } else { ret = -1; }
#endif

    std::thread(&Monitor::MonitorThread, this).detach();
    std::thread(&Monitor::DumpThread, this).detach();
    // LOG_INFO("start monitor dump thread successfully");

    return ret;
}

int Monitor::Update(const std::string& key, double value, Operation op) {
    int ret = 0; 
    if (key.empty() || init_flag_ == false ) { 
        return -1;
    } else {
        UpdatePara* data = new UpdatePara(key, value, op);
        ret = queue_.push(data);
    }
    return ret;
}

int Monitor::avg(const std::string& key, double value) {
    Monitor& monitor =  Monitor::GetInstance();
    return monitor.Update(key, value, AVG);
}

int Monitor::inc(const std::string& key, double value) {
    Monitor& monitor =  Monitor::GetInstance();
    return monitor.Update(key, value, INC);
}

int Monitor::max(const std::string& key, double value) {
    Monitor& monitor =  Monitor::GetInstance();
    return monitor.Update(key, value, MAX);
}

int Monitor::min(const std::string& key, double value) {
    Monitor& monitor =  Monitor::GetInstance();
    return monitor.Update(key, value, MIN);
}

void Monitor::Dump(std::string* dump_str) {
    monitor_data_->Dump(dump_str);
}
 
}  // namespace monitor
}  // namespace util 
}  // namespace prediction
