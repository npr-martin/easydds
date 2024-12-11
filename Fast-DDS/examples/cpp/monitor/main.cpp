// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file main.cpp
 *
 */

#include <csignal>
#include <stdexcept>
#include <thread>
#include <fstream>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>

#include "easyddsApplication.hpp"
#include "easyddsMonitorSub.hpp"

std::function<void(int)> stop_app_handler;
void signal_handler(
        int signum)
{
    stop_app_handler(signum);
}

int main(
        int argc,
        char** argv)
{
    // to make build finish
    std::string unUsed = transTopic(EASYDDS::monitorItems_default);

    std::string fileName = "/home/mhy/monitor.txt";
    if(argc > 1)
    {
        fileName = argv[1];
    }
    std::ofstream ofs;
    ofs.open(fileName, std::ios::out);

    if(!ofs.is_open())
    {
        std::cout << "open monitor file error. Please check the filePath: " << fileName << std::endl;
        return EXIT_FAILURE;
    }

    std::shared_ptr<easyddsMonitorSub> sentApp = 
    easyddsApplication::createMoniterSubscriber(0, "SENT_DATA_TOPIC");
    sentApp->registerWriterOp([&](std::string sampleIdentity, std::string msg){
        // std::cout << "sampleIdentity = " << sampleIdentity << ", msg = " << msg << "." << std::endl;
        ofs << "sampleIdentity = " << sampleIdentity << ", msg = " << msg << std::endl;
    });

    std::shared_ptr<easyddsMonitorSub> recvApp = 
    easyddsApplication::createMoniterSubscriber(0, "RECEIVED_DATA_TOPIC");
    recvApp->registerReaderOp([&](std::string sampleIdentity, std::string readerID){
        // std::cout << "sampleIdentity = " << sampleIdentity << ", readerID = " << readerID << "." << std::endl;
        ofs << "sampleIdentity = " << sampleIdentity << ", readerID = " << readerID << std::endl;
    });

    std::thread thread(&easyddsApplication::run, recvApp);

    std::cout << "Monitor SENT_DATA_TOPIC&RECEIVED_DATA_TOPIC is running. "
                 "Please press Ctrl+C to stop the monitor at any time."
              << std::endl;
    std::cout << "Monitor Information will be saved in " << fileName << std::endl;

    stop_app_handler = [&](int signum)
                {
                    std::cout << signum << " received. Stop execution." << std::endl;
                    sentApp->stop();
                    recvApp->stop();
                    ofs.close();
                };

        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
    #ifndef _WIN32
        signal(SIGQUIT, signal_handler);
        signal(SIGHUP, signal_handler);
    #endif // _WIN32

    thread.join();
    sentApp->run();
    
    return EXIT_SUCCESS;
}
