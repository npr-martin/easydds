#include "easyddsClientPublisherApp.hpp"

#include <condition_variable>
#include <stdexcept>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>

#include "easyddsPubSubTypes.hpp"

easyddsClientPublisherApp::easyddsClientPublisherApp(
    const std::string &topic_name,
    const int &domain_id,
    const EASYDDS::easyddsClientConfig &config)
    : participant_(nullptr), publisher_(nullptr), topic_(nullptr)
    , writer_(nullptr), type_(new EmployeePubSubType()), matched_(0), stop_(false)
{
    // Configure Participant QoS
    DomainParticipantQos pqos;
    if(config.useDiscoveryServer)
    {
        pqos = getClientDomainParticipantQos(config.open_monitor, config.items, config.clientConfig);
    }
    else
    {
        pqos = getPubDomainParticipantQos(config.open_monitor, config.items, config.clientConfig, config.qosProfile.samples);
    }
    

    // Create Domainparticipant
    auto factory = DomainParticipantFactory::get_instance();
    LibrarySettings library_settings;
    library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
    factory->set_library_settings(library_settings);
    participant_ = factory->create_participant(0, pqos, nullptr);

    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    // Regsiter type
    type_.register_type(participant_);

    // Create the publisher
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    if (publisher_ == nullptr)
    {
        throw std::runtime_error("Publisher initialization failed");
    }

    // Create the topic
    topic_ = participant_->create_topic(topic_name, type_.get_type_name(), TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create de data writer
    DataWriterQos wqos = getDataQos<DataWriterQos>(config.qosProfile, DATAWRITER_QOS_DEFAULT);

    if (config.clientConfig.transport_kind == EASYDDS::TransportKind::DATA_SHARING)
    {
        wqos.data_sharing().automatic();
    }
    else
    {
        wqos.data_sharing().off();
    }

    writer_ = publisher_->create_datawriter(topic_, wqos, this);

    if (writer_ == nullptr)
    {
        throw std::runtime_error("DataWriter initialization failed");
    }
}

easyddsClientPublisherApp::~easyddsClientPublisherApp()
{
    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void easyddsClientPublisherApp::on_publication_matched(
    DataWriter * /*writer*/,
    const PublicationMatchedStatus &info)
{
    if (info.current_count_change == 1)
    {
        matched_ = static_cast<int16_t>(info.current_count);
        std::cout << "Publisher matched." << std::endl;
        // cv_.notify_one();
    }
    else if (info.current_count_change == -1)
    {
        matched_ = static_cast<int16_t>(info.current_count);
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void easyddsClientPublisherApp::run()
{
    // while (!is_stopped() && ((samples_ == 0) || (hello_.index() < samples_)))
    // {
    //     if (publish())
    //     {
    //         std::cout << "Message: '" << hello_.message() << "' with index: '" << hello_.index()
    //                   << "' SENT" << std::endl;
    //     }
    //     // Wait for period or stop event
    //     std::unique_lock<std::mutex> period_lock(mutex_);
    //     cv_.wait_for(period_lock, std::chrono::milliseconds(period_ms_), [&]()
    //             {
    //                 return is_stopped();
    //             });
    // }
}

bool easyddsClientPublisherApp::getIsStopped()
{
    return is_stopped();
}

bool easyddsClientPublisherApp::send(const std::string &msg)
{
    bool ret = false;

    if (!is_stopped())
    {
        Employee sample_;
        // auto now_begin = std::chrono::system_clock::now();
        // std::string sendmsg = getCurTimeStr(now_begin) + msg;
        // sample_.text(sendmsg);

        auto now = std::chrono::high_resolution_clock::now();
        auto duration = now.time_since_epoch();
        uint64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        std::string sendmsg = std::to_string(microseconds) + msg;
     
        sample_.text(sendmsg);
        ret = (RETCODE_OK == writer_->write(&sample_));   
        auto now_2 = std::chrono::high_resolution_clock::now();
        std::cout << "the send time is " << getCurTimeStr(now) <<"   "<<getCurTimeStr(now_2)<<std::endl;
        //auto now_finish = std::chrono::system_clock::now();

        // printCurrentTime(now_begin);
        // printCurrentTime(now_finish);


    }
    return ret;
}

bool easyddsClientPublisherApp::is_stopped()
{
    return stop_.load();
}

void easyddsClientPublisherApp::stop()
{
    stop_.store(true);
}