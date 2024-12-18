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

    std::string fileName = "./monitor.txt";
    if(argc > 1)
    {
        fileName = argv[1];
    }
    std::ofstream ofs;
    ofs.open(fileName, std::ios::out);

    if(!ofs.is_open())
    {
        std::cout << "Open monitor file error. Please check the filePath: " << fileName << std::endl;
        return EXIT_FAILURE;
    }

    std::shared_ptr<easyddsMonitorSub> sentApp = 
    easyddsApplication::createMoniterSubscriber(0, "SENT_DATA_TOPIC");
    sentApp->registerWriterOp([&](std::string sampleIdentity, std::string msg){
        std::string sentInfo = "sampleIdentity = " + sampleIdentity + ", msg = " + msg + "\n";
        ofs << sentInfo;
    });

    std::shared_ptr<easyddsMonitorSub> recvApp = 
    easyddsApplication::createMoniterSubscriber(0, "RECEIVED_DATA_TOPIC");
    recvApp->registerReaderOp([&](std::string sampleIdentity, std::string readerID){
        std::string recvInfo = "sampleIdentity = " + sampleIdentity + ", readerID = " + readerID + "\n";
        ofs << recvInfo;
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
