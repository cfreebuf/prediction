# -*- coding: utf-8 -*-
# CopyRight 2019 360. All rights reserved.
# @file run.sh
# @date 2019-11-01 19:03
# @brief

module=prediction

function start() {
    nohup ./bin/$module --flagfile=./conf/gflags.conf > nohup.out 2>&1 &
}

function stop() {
    for pid in `ps ux | grep -F "./bin/$module --flagfile=./conf/gflags.conf" | grep -v grep | awk '{print $2}'`
    do
        echo $pid
        kill -9 $pid
    done
}

if [ $# -lt 1 ]; then
    echo "usage:sh shell/run.sh [start][stop][restart]"
    exit
fi

if [ $1 == 'start' ]; then
    echo -e "\033[32mSTART MODULE [$module]\033[0m"
    start
    echo -e "\033[32mSTART MODULE SUCCEFULLY [$module]\033[0m"
fi

if [ $1 == 'stop' ]; then
    echo -e "\033[32mSTOP MODULE [$module]\033[0m"
    stop
    echo -e "\033[32mSTOP MODULE SUCCEFULLY [$module]\033[0m"
fi

if [ $1 == 'restart' ]; then
    echo -e "\033[32mRESTART MODULE [$module]\033[0m"
    stop
    sleep 1
    start
    echo -e "\033[32mRESTART MODULE SUCCEFULLY [$module]\033[0m"
fi
