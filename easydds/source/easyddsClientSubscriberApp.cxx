#include "easyddsClientSubscriberApp.hpp"

#include <condition_variable>
#include <stdexcept>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>

#include "easyddsPubSubTypes.hpp"

easyddsClientSubscriberApp::easyddsClientSubscriberApp(
    const std::string &topic_name,
    const int &domain_id,
    const EASYDDS::easyddsClientConfig &config)
    : participant_(nullptr), subscriber_(nullptr), topic_(nullptr)
    , reader_(nullptr), type_(new EmployeePubSubType()), stop_(false), m_sampleCount(0), m_topicName(topic_name)
{
    DomainParticipantQos pqos;
    if(config.useDiscoveryServer)
    {
        pqos = getClientDomainParticipantQos(config.open_monitor, config.items, config.clientConfig);
    }
    else
    {
        pqos = getSubDomainParticipantQos(config.open_monitor, config.items, config.clientConfig, config.qosProfile.samples);
    }

    // Create the Domainparticipant
     auto factory = DomainParticipantFactory::get_instance();
    LibrarySettings library_settings;
    library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
    factory->set_library_settings(library_settings);
    participant_ = factory->create_participant(0, pqos, nullptr,
                    StatusMask::all() >> StatusMask::data_on_readers());

    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    // Register the type
    type_.register_type(participant_);

    // Create the subscriber
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (subscriber_ == nullptr)
    {
        throw std::runtime_error("Subscriber initialization failed");
    }
    // Create the topic
    topic_ = participant_->create_topic(
        topic_name,
        type_.get_type_name(),
        TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create the data reader
    DataReaderQos rqos = getDataQos<DataReaderQos>(config.qosProfile, DATAREADER_QOS_DEFAULT);

    if (config.clientConfig.transport_kind == EASYDDS::TransportKind::DATA_SHARING)
    {
        rqos.data_sharing().automatic();
    }
    else
    {
        rqos.data_sharing().off();
    }

    reader_ = subscriber_->create_datareader(topic_, rqos, this);

    if (reader_ == nullptr)
    {
        throw std::runtime_error("DataWriter initialization failed");
    }
}

easyddsClientSubscriberApp::~easyddsClientSubscriberApp()
{
    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void easyddsClientSubscriberApp::on_subscription_matched(
        DataReader* /*reader*/,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void easyddsClientSubscriberApp::on_data_available(
        DataReader* reader)
{
    Employee sample_;
    SampleInfo info;
    while ((!is_stopped()) && (RETCODE_OK == reader->take_next_sample(&sample_, &info)))
    {
        if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
        {
            if (receivedMsg)
            {
                std::string printInfo = "Send [topic: " + m_topicName + "] Sample: " + std::to_string(m_sampleCount++) + "  \n";
                receivedMsg(printInfo);
            }
            std::cout << "sample received." << std::endl;
        }
        else
        {
            std::cout << "info.instance_state = " << info.instance_state << " , valid_data = " << info.valid_data;
        }
    }
}

void easyddsClientSubscriberApp::run()
{
    // std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    // terminate_cv_.wait(lck, [&]
    //         {
    //             return is_stopped();
    //         });
}

bool easyddsClientSubscriberApp::is_stopped()
{
    return stop_.load();
}

void easyddsClientSubscriberApp::stop()
{
    stop_.store(true);
}

bool easyddsClientSubscriberApp::getIsStopped()
{
    return is_stopped();
}
void easyddsClientSubscriberApp::onMessageReceived(const std::function<void(std::string)> &func)
{
    receivedMsg = func;
}
