# -*- coding: utf-8 -*-
# CopyRight 2019 360. All rights reserved.
# @file upload_metrics.py
# @date 2019-11-01 19:03
# @brief

import requests
import time
import json
import socket # used for getting hostname
import os # used for executing shell commands

raw_metric_file_path = './prediction.cmm'
monitor_reader = './bin/monitor_reader'
# read the metric file, output a map
# containing <metric_name, metric_value> pairs
def read_metrics():
    ret_code = os.system("cd bin && ./monitor_reader ../data/monitor.dat > ../monitor_status")
    if (ret_code != 0) :
        return {}
    input = open('monitor_status', 'r')
    # first line is header, ignore it
    input.readline()
    metric_dict = {} 
    for line in input.readlines():
        raw_tuple = line.split('\t')
        if len(raw_tuple) < 5:
            continue
        print str(raw_tuple)
        raw_tuple = [s.replace('\n', '') for s in raw_tuple]
        date = raw_tuple[0]
        time_stamp = raw_tuple[1]
        key = raw_tuple[2]
        value = raw_tuple[3]
        # raw_tuple[4] is type, we do not use it
        if key in metric_dict:
            (old_time_stamp, old_value) = metric_dict[key]
            if time_stamp > old_time_stamp:
                metric_dict[key] = (time_stamp, value)
        else:
            metric_dict[key] = (time_stamp, value)
    return metric_dict 

# metric_data is k-v structured
def make_payload(metric_dict):
    result = []
    for key in metric_dict:
        (time_stamp, value) = metric_dict[key]
        #print str(key)+"\t"+str(value)
        one_payload = {}
        one_payload['endpoint'] = str(socket.gethostname())
        one_payload['metric'] = key
        one_payload['timestamp'] = int(time_stamp)
        one_payload['step'] = 60
        one_payload['value'] = value
        one_payload['counterType'] = 'GAUGE'
        one_payload['tags'] = 'idc=lg,loc=beijing'
        result.append(one_payload)
    return result

payload = make_payload(read_metrics())        
print str(payload)
r = requests.post("http://127.0.0.1:1988/v1/push", data=json.dumps(payload))

#print r.text

