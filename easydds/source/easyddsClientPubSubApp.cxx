#include "easyddsClientPubSubApp.hpp"

#include <condition_variable>
#include <stdexcept>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
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

easyddsClientPubSubApp::easyddsClientPubSubApp(
    const std::string &pub_topic_name,
    const std::string &sub_topic_name,
    const int &domain_id,
    const EASYDDS::easyddsClientConfig &config)
    : pub_participant_(nullptr), sub_participant_(nullptr),
      publisher_(nullptr), subscriber_(nullptr),
      pub_topic_(nullptr), sub_topic_(nullptr),
      writer_(nullptr), reader_(nullptr), type_(new EmployeePubSubType()), 
      pub_matched_(0), sub_matched_(0), stop_(false),
      m_sampleCount(0), m_topicName(sub_topic_name)
{
    // Create Domainparticipant
    auto factory = DomainParticipantFactory::get_instance();
    LibrarySettings library_settings;
    library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
    factory->set_library_settings(library_settings);

    // publisher
    {
        // Configure Participant QoS
        DomainParticipantQos pub_pqos;
        if (config.useDiscoveryServer)
        {
            pub_pqos = getClientDomainParticipantQos(config.open_monitor, config.items, config.clientConfig);
        }
        else
        {
            pub_pqos = getPubDomainParticipantQos(config.open_monitor, config.items,
                                                  config.clientConfig, config.qosProfile.samples);
        }

        pub_participant_ = factory->create_participant(0, pub_pqos, nullptr);

        if (pub_participant_ == nullptr)
        {
            throw std::runtime_error("Participant initialization failed");
        }

        // Regsiter type
        type_.register_type(pub_participant_);

        // Create the publisher
        publisher_ = pub_participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

        if (publisher_ == nullptr)
        {
            throw std::runtime_error("Publisher initialization failed");
        }

        // Create the topic
        pub_topic_ = pub_participant_->create_topic(pub_topic_name, type_.get_type_name(), TOPIC_QOS_DEFAULT);

        if (pub_topic_ == nullptr)
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

        writer_ = publisher_->create_datawriter(pub_topic_, wqos, this);

        if (writer_ == nullptr)
        {
            throw std::runtime_error("DataWriter initialization failed");
        }
    }

    // subscriber
    {
        DomainParticipantQos sub_pqos;
        if (config.useDiscoveryServer)
        {
            sub_pqos = getClientDomainParticipantQos(config.open_monitor, config.items, config.clientConfig);
        }
        else
        {
            sub_pqos = getSubDomainParticipantQos(config.open_monitor, config.items,
                                              config.clientConfig, config.qosProfile.samples);
        }

        sub_participant_ = factory->create_participant(0, sub_pqos, nullptr,
                                                   StatusMask::all() >> StatusMask::data_on_readers());

        if (sub_participant_ == nullptr)
        {
            throw std::runtime_error("Participant initialization failed");
        }

        // Register the type
        type_.register_type(sub_participant_);

        // Create the subscriber
        subscriber_ = sub_participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

        if (subscriber_ == nullptr)
        {
            throw std::runtime_error("Subscriber initialization failed");
        }
        // Create the topic
        sub_topic_ = sub_participant_->create_topic(
            sub_topic_name,
            type_.get_type_name(),
            TOPIC_QOS_DEFAULT);

        if (sub_topic_ == nullptr)
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

        reader_ = subscriber_->create_datareader(sub_topic_, rqos, this);

        if (reader_ == nullptr)
        {
            throw std::runtime_error("DataWriter initialization failed");
        }
    }
}

easyddsClientPubSubApp::~easyddsClientPubSubApp()
{
    if (nullptr != pub_participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        pub_participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(pub_participant_);
    }

    if (nullptr != sub_participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        sub_participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(sub_participant_);
    }
}

void easyddsClientPubSubApp::on_publication_matched(
    DataWriter * /*writer*/,
    const PublicationMatchedStatus &info)
{
    if (info.current_count_change == 1)
    {
        pub_matched_ = static_cast<int16_t>(info.current_count);
        std::cout << "Publisher matched." << std::endl;
        // cv_.notify_one();
    }
    else if (info.current_count_change == -1)
    {
        pub_matched_ = static_cast<int16_t>(info.current_count);
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void easyddsClientPubSubApp::on_subscription_matched(
        DataReader* /*reader*/,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        sub_matched_ = static_cast<int16_t>(info.current_count);
        std::cout << "Sub matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        sub_matched_ = static_cast<int16_t>(info.current_count);
        std::cout << "Sub unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void easyddsClientPubSubApp::run()
{

}

bool easyddsClientPubSubApp::getIsStopped()
{
    return is_stopped();
}

bool easyddsClientPubSubApp::send(const std::string &msg)
{
    bool ret = false;

    if (!is_stopped()) // && pub_matched_> 0)
    {
        Employee sample_(msg);

        ret = (RETCODE_OK == writer_->write(&sample_));
    }
    return ret;
}

void easyddsClientPubSubApp::on_data_available(
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
                receivedMsg(sample_.text());
            }
            std::cout << "sample received." << std::endl;
        }
        else
        {
            std::cout << "info.instance_state = " << info.instance_state << " , valid_data = " << info.valid_data;
        }
    }
}

void easyddsClientPubSubApp::onMessageReceived(const std::function<void(std::string)> &func)
{
    receivedMsg = func;
}

std::string easyddsClientPubSubApp::getGUID()
{
    if(writer_)
    {
        std::stringstream ss;
        ss << writer_->guid();
        return ss.str();
    }
    return std::string();
}

bool easyddsClientPubSubApp::is_stopped()
{
    return stop_.load();
}

void easyddsClientPubSubApp::stop()
{
    stop_.store(true);
}