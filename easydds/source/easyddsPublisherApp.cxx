#include "easyddsPublisherApp.hpp"

#include <condition_variable>
#include <csignal>
#include <stdexcept>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>

#include "easyddsPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;

easyddsPublisherApp::easyddsPublisherApp(
    const std::string &topic_name
    , const int &domain_id
    , const qos_profile_s &qos_profile
    , const bool &open_monitor
    , const MONITOR_TOPIC::monitorItems &items)
    : factory_(nullptr), participant_(nullptr), publisher_(nullptr),
      topic_(nullptr), writer_(nullptr), type_(new EmployeePubSubType()),
      matched_(0), stop_(false), m_topicName(topic_name)
{
    // Create the participant
    DomainParticipantQos pqos = getDomainParticipantQos(open_monitor, items);
    factory_ = DomainParticipantFactory::get_shared_instance();
    participant_ = factory_->create_participant(domain_id, pqos, nullptr, StatusMask::none());
    if (participant_ == nullptr)
    {
        throw std::runtime_error("Employee Participant initialization failed");
    }

    // Register the type
    type_.register_type(participant_);

    // Create the publisher
    PublisherQos pub_qos = PUBLISHER_QOS_DEFAULT;
    participant_->get_default_publisher_qos(pub_qos);
    publisher_ = participant_->create_publisher(pub_qos, nullptr, StatusMask::none());
    if (publisher_ == nullptr)
    {
        throw std::runtime_error("Employee Publisher initialization failed");
    }

    // Create the topic
    TopicQos topic_qos = TOPIC_QOS_DEFAULT;
    participant_->get_default_topic_qos(topic_qos);
    topic_ = participant_->create_topic(topic_name, type_.get_type_name(), topic_qos);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Employee Topic initialization failed");
    }

    // Create the data writer
    DataWriterQos writer_qos = getDataQos<DataWriterQos>(qos_profile, DATAWRITER_QOS_DEFAULT);
    writer_ = publisher_->create_datawriter(topic_, writer_qos, this, StatusMask::all());

    if (writer_ == nullptr)
    {
        throw std::runtime_error("Employee DataWriter initialization failed");
    }
}

easyddsPublisherApp::~easyddsPublisherApp()
{
    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        factory_->delete_participant(participant_);
    }
}

void easyddsPublisherApp::on_publication_matched(
    DataWriter * /*writer*/,
    const PublicationMatchedStatus &info)
{
    if (info.current_count_change == 1)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            matched_ = info.current_count;
        }
        std::cout << "Employee Publisher matched." << std::endl;
        cv_.notify_one();
    }
    else if (info.current_count_change == -1)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            matched_ = info.current_count;
        }
        std::cout << "Employee Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void easyddsPublisherApp::run()
{
    // while (!is_stopped())
    // {
    //     if (publish())
    //     {
    //         //std::cout << "Sample '" << std::to_string(++samples_sent_) << "' SENT: topic_name: " <<m_topicName<< std::endl;
    //         std::cout <<"Sent [topic: " << m_topicName << "] Sample " << std::to_string(++samples_sent_) <<std::endl;
    //     }
    //     // Wait for period or stop event
    //     std::unique_lock<std::mutex> period_lock(mutex_);
    //     cv_.wait_for(period_lock, std::chrono::milliseconds(period_ms_), [this]()
    //             {
    //                 return is_stopped();
    //             });
    // }
}

bool easyddsPublisherApp::send(const std::string &msg)
{
    bool ret = false;
    // Wait for the data endpoints discovery
    // std::unique_lock<std::mutex> matched_lock(mutex_);
    // cv_.wait(matched_lock, [&]()
    //          {
    //             // at least one has been discovered
    //             return ((matched_ > 0) || is_stopped()); });

    if (!is_stopped())
    {
        Employee sample_(msg);
        // const clock_t begin_time = clock();
        ret = (RETCODE_OK == writer_->write(&sample_));
        // float mseconds = float(clock() - begin_time);
    }
    return ret;
}

bool easyddsPublisherApp::getIsStopped()
{
    return is_stopped();
}

bool easyddsPublisherApp::is_stopped()
{
    return stop_.load();
}

void easyddsPublisherApp::stop()
{
    stop_.store(true);
    // cv_.notify_one();
}
