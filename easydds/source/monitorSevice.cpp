#include <csignal>
#include <stdexcept>
#include <thread>
#include <fstream>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>

#include "easyddsApplication.hpp"
#include "easyddsMonitorSub.hpp"

// 定义一个缓冲区和互斥锁
std::vector<char> buffer;
std::mutex mtx;
std::condition_variable cv;
bool finished = false;

// 写文件的函数
void write_to_file(const std::string& filename) 
{
    std::ofstream file(filename, std::ios::out | std::ios::app);
    while (true) 
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []{ return !buffer.empty() || finished; });

        if (finished && buffer.empty()) 
        {
            break;
        }

        // 写入缓冲区内容到文件
        file.write(buffer.data(), buffer.size());
        buffer.clear(); // 清空缓冲区
        file.flush();
        lock.unlock(); // 解锁互斥锁
    }
    file.close();
}

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
    std::string fileName = "./monitor.txt";
    if(argc > 1)
    {
        fileName = argv[1];
    }
    // std::ofstream ofs;
    // ofs.open(fileName, std::ios::out);

    // if(!ofs.is_open())
    // {
    //     std::cout << "Open monitor file error. Please check the filePath: " << fileName << std::endl;
    //     return EXIT_FAILURE;
    // }

    // 启动消费者线程，每隔1秒写入文件
    std::thread writer(write_to_file, fileName);

    std::shared_ptr<easyddsMonitorSub> sentApp = 
    easyddsApplication::createMoniterSubscriber(0, "SENT_DATA_TOPIC");
    sentApp->registerWriterOp([&](const std::string& sampleIdentity, const std::string& msg){
        std::lock_guard<std::mutex> lock(mtx);
        std::string sentInfo = "sampleIdentity = " + sampleIdentity + ", msg = " + msg + "\n";
        buffer.insert(buffer.end(), sentInfo.begin(), sentInfo.end());
        cv.notify_one();
        // ofs << sentInfo;
    });

    std::shared_ptr<easyddsMonitorSub> recvApp = 
    easyddsApplication::createMoniterSubscriber(0, "RECEIVED_DATA_TOPIC");
    recvApp->registerReaderOp([&](const std::string& sampleIdentity, const std::string& readerID){
        std::lock_guard<std::mutex> lock(mtx);
        std::string recvInfo = "sampleIdentity = " + sampleIdentity + ", readerID = " + readerID + "\n";
        buffer.insert(buffer.end(), recvInfo.begin(), recvInfo.end());
        cv.notify_one();
        // ofs << recvInfo;
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
        finished = true;
        cv.notify_all();
        // ofs.close();
    };

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
#ifndef _WIN32
    signal(SIGQUIT, signal_handler);
    signal(SIGHUP, signal_handler);
#endif // _WIN32
    
    writer.join();
    thread.join();
    sentApp->run();

    return EXIT_SUCCESS;
}
