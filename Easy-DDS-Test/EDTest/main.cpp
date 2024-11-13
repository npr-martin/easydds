#include "easyddstest.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EasyDDSTest w;
    w.show();
    return a.exec();
}

//#include <csignal>
//#include <cstring>
//#include <functional>
//#include <iostream>
//#include <stdexcept>
//#include <thread>

//#include <fastdds/dds/log/Log.hpp>

//#include "easyddsApplication.hpp"

//using eprosima::fastdds::dds::Log;

//std::function<void(int)> stop_handler;
//void signal_handler(
//        int signum)
//{
//    stop_handler(signum);
//}

//std::string parse_signal(
//        const int& signum)
//{
//    switch (signum)
//    {
//        case SIGINT:
//            return "SIGINT";
//        case SIGTERM:
//            return "SIGTERM";
//#ifndef _WIN32
//        case SIGQUIT:
//            return "SIGQUIT";
//        case SIGHUP:
//            return "SIGHUP";
//#endif // _WIN32
//        default:
//            return "UNKNOWN SIGNAL";
//    }
//}

//int main(
//        int argc,
//        char** argv)
//{
//    auto ret = EXIT_SUCCESS;
//    int domain_id = 0;
//    std::shared_ptr<easyddsApplication> app;

//    if(argc !=3 || (strcmp(argv[2], "publisher") != 0 && strcmp(argv[2], "subscriber") != 0))
//    {
//        std::cout << "Error: Incorrect arguments." << std::endl;
//        std::cout << "Usage: " << std::endl << std::endl;
//        std::cout << argv[0] << " \"topic_name\" publisher|subscriber" << std::endl << std::endl;
//        ret = EXIT_FAILURE;
//    }
//    else
//    {
//        try
//        {
//            app = easyddsApplication::make_app(domain_id, argv[1], argv[2]);
//        }
//        catch (const std::runtime_error& e)
//        {
//            EPROSIMA_LOG_ERROR(app_name, e.what());
//            ret = EXIT_FAILURE;
//        }



//    // if (argc != 2 || (strcmp(argv[1], "publisher") != 0 && strcmp(argv[1], "subscriber") != 0))
//    // {
//    //     std::cout << "Error: Incorrect arguments." << std::endl;
//    //     std::cout << "Usage: " << std::endl << std::endl;
//    //     std::cout << argv[0] << " publisher|subscriber" << std::endl << std::endl;
//    //     ret = EXIT_FAILURE;
//    // }
//    // else
//    // {
//    //     try
//    //     {
//    //         app = easyddsApplication::make_app(domain_id, argv[1]);
//    //     }
//    //     catch (const std::runtime_error& e)
//    //     {
//    //         EPROSIMA_LOG_ERROR(app_name, e.what());
//    //         ret = EXIT_FAILURE;
//    //     }

//        std::thread thread(&easyddsApplication::run, app);

//        std::cout << argv[1] << "'s " << argv[2] << " running. Please press Ctrl+C to stop the "
//                  << argv[2] << " at any time." << std::endl;

//        stop_handler = [&](int signum)
//                {
//                    std::cout << "\n" << parse_signal(signum) << " received, stopping " << argv[1]
//                              << " execution." << std::endl;
//                    app->stop();
//                };

//        signal(SIGINT, signal_handler);
//        signal(SIGTERM, signal_handler);
//#ifndef _WIN32
//        signal(SIGQUIT, signal_handler);
//        signal(SIGHUP, signal_handler);
//#endif // _WIN32

//        thread.join();
//    }

//    Log::Reset();
//    return ret;
//}
