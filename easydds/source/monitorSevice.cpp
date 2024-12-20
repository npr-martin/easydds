#include <csignal>
#include <stdexcept>
#include <thread>
#include <fstream>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>

#include "easyddsApplication.hpp"
#include "easyddsMonitorSub.hpp"

// 用于存储解析后的INI文件内容
struct IniSection 
{
    std::map<std::string, std::string> keyValues;
};

std::map<std::string, IniSection> parseIniFile(const std::string& filename) 
{
    std::map<std::string, IniSection> iniData;
    std::ifstream file(filename);
    std::string line;
    std::string currentSection;

    // 检查文件是否成功打开
    if (!file.is_open()) 
    {
        std::cerr << "Unable to open file " << filename << std::endl;
        return iniData;
    }

    while (std::getline(file, line)) 
    {
        // 忽略空行和注释
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        // 检测节的开始
        if (line[0] == '[') 
        {
            size_t end = line.find(']');
            if (end != std::string::npos) 
            {
                currentSection = line.substr(1, end - 1);
                iniData[currentSection];
            }
        } 
        else 
        {
            // 解析键值对
            size_t equalPos = line.find('=');
            if (equalPos != std::string::npos) 
            {
                std::string key = line.substr(0, equalPos);
                std::string value = line.substr(equalPos + 1);
                // 去除键和值周围的空格
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                iniData[currentSection].keyValues[key] = value;
            }
        }
    }

    file.close();
    return iniData;
}

std::string getIniValue(const std::map<std::string, IniSection> &iniData,
                        const std::string &section, const std::string &key,
                        const std::string &defaultValue)
{
    std::string result = defaultValue;

    auto sectionIter = std::find_if(iniData.begin(), iniData.end(),
                                    [=](const std::pair<std::string, IniSection>& sectionKey)
                                    {
                                        return sectionKey.first == section;
                                    });
    if (sectionIter != iniData.end())
    {
        auto valueMap = sectionIter->second.keyValues;
        auto valueIter = std::find_if(valueMap.begin(), valueMap.end(),
                                      [=](const std::pair<std::string, std::string>& valueKey)
                                      {
                                          return valueKey.first == key;
                                      });
        if (valueIter != valueMap.end())
        {
            result = valueIter->second;
        }
    }

    return result;
}

std::map<std::string, TransportKind> strKindMap{
    {"default", TransportKind::DEFAULT},
    {"udpv4", TransportKind::UDPv4},
    {"udpv6", TransportKind::UDPv6},
    {"tcpv4", TransportKind::TCPv4},
    {"tcpv6", TransportKind::TCPv6},
    {"shm", TransportKind::SHM},
    {"datasharing", TransportKind::DATA_SHARING},
    {"largedata", TransportKind::LARGE_DATA}};

easyddsClientConfig getConfigFromIni(const std::string& fileName)
{
    easyddsClientConfig ecc = easyddsClientConfig();
    auto iniData = parseIniFile(fileName);

    // TransportKind
    std::string transportKind = getIniValue(iniData, "config", "transportKind", "default");
    TransportKind tk = TransportKind::DEFAULT;
    auto iter = strKindMap.find(transportKind);
    if(iter != strKindMap.end())
    {
        tk = iter->second;
    }
    ecc.clientConfig.transport_kind = tk;

    // UseDiscoveryServer
    std::string useDiscoveryServer = getIniValue(iniData, "config", "useDiscoveryServer", "false");
    ecc.useDiscoveryServer = (useDiscoveryServer == "true");

    if(ecc.useDiscoveryServer)
    {
        // IPAddress
        std::string ipAddress = getIniValue(iniData, "discoveryConfig", "ipaddress", "127.0.0.1");
        ecc.clientConfig.connection_address = ipAddress;

        // Port
        std::string port = getIniValue(iniData, "discoveryConfig", "port", "16166");
        ecc.clientConfig.connection_port = std::stoi(port);
    }

    return ecc;
}

void checkAndCreateIni(const std::string& fileName)
{
    // 尝试以输入模式打开文件，以检查文件是否存在
    std::ifstream file_check(fileName);
    if (!file_check) 
    {
        // 文件不存在，以输出模式打开文件并写入文本
        std::ofstream file_write(fileName);
        if (file_write)
        {
            file_write << "[config]\n# default/udpv4/udpv6/tcpv4/tcpv6/shm/datasharing/largedata\n"
                          "transportKind = default\n# true/false\nuseDiscoveryServer = false\n\n"
                          "[discoveryConfig]\nipaddress = 127.0.0.1\nport = 16166";
        }
    }
}

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
    std::string fileName = (argc > 1) ? argv[1] : "./monitor.txt";

    // 启动消费者线程，每隔1秒写入文件
    std::thread writer(write_to_file, fileName);

    std::string iniName = "./monitor.ini";
    checkAndCreateIni(iniName);

    easyddsClientConfig ecc = getConfigFromIni(iniName);

    std::shared_ptr<easyddsMonitorSub> sentApp = 
    easyddsApplication::createMoniterSubscriber("SENT_DATA_TOPIC", 0, ecc);
    sentApp->registerWriterOp([&](const std::string& sampleIdentity, const std::string& msg){
        std::lock_guard<std::mutex> lock(mtx);
        std::string sentInfo = "sampleIdentity = " + sampleIdentity + ", msg = " + msg + "\n";
        buffer.insert(buffer.end(), sentInfo.begin(), sentInfo.end());
        cv.notify_one();
        // ofs << sentInfo;
    });

    std::shared_ptr<easyddsMonitorSub> recvApp = 
    easyddsApplication::createMoniterSubscriber("RECEIVED_DATA_TOPIC", 0, ecc);
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
