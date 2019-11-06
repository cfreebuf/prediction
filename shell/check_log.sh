# -*- coding: utf-8 -*-
# CopyRight 2019 360. All rights reserved.
# @file check_log.sh
# -*- coding: utf-8 -*-
# @date 2019-11-01 19:00
# @brief

# keep logs files of the last 3 days
find /home/appops/app/logs -type f -mtime +3 -exec rm -f {} \;
