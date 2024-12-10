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
        int ,
        char** )
{
    std::shared_ptr<easyddsMonitorSub> app = 
    easyddsApplication::createMoniterSubscriber(0, "HISTORY_LATENCY_TOPIC");
    
    std::string unUsed = transTopic(EASYDDS::monitorItems_default);

    // std::thread thread(&easyddsApplication::run, app);

    std::cout << "Monitor HISTORY_LATENCY_TOPIC is running. Please press Ctrl+C to stop the monitor at any time." << std::endl;

    stop_app_handler = [&](int signum)
                {
                    std::cout << signum << " received. Stop execution." << std::endl;
                    app->stop();
                };

        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
    #ifndef _WIN32
        signal(SIGQUIT, signal_handler);
        signal(SIGHUP, signal_handler);
    #endif // _WIN32

    app->run();
    // thread.join();
    
    return EXIT_SUCCESS;
}
