#include "easyddsSubscriberApp.hpp"

#include <condition_variable>
#include <stdexcept>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

#include "easyddsPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;

easyddsSubscriberApp::easyddsSubscriberApp(
      const std::string &topic_name
    , const int &domain_id
    , const qos_profile_s &qos_profile
    , const bool &open_monitor
    , const MONITOR_TOPIC::monitorItems &items)
    : factory_(nullptr), participant_(nullptr), subscriber_(nullptr), topic_(nullptr), 
    reader_(nullptr), type_(new EmployeePubSubType()), stop_(false), 
    m_topicName(topic_name), m_sampleCount(0)
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

    // Create the subscriber
    SubscriberQos sub_qos = SUBSCRIBER_QOS_DEFAULT;
    participant_->get_default_subscriber_qos(sub_qos);
    subscriber_ = participant_->create_subscriber(sub_qos, nullptr, StatusMask::none());
    if (subscriber_ == nullptr)
    {
        throw std::runtime_error("Employee Subscriber initialization failed");
    }

    // Create the topic
    TopicQos topic_qos = TOPIC_QOS_DEFAULT;
    participant_->get_default_topic_qos(topic_qos);
    topic_ = participant_->create_topic(topic_name, type_.get_type_name(), topic_qos);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Employee Topic initialization failed");
    }

    // Create the reader
    DataReaderQos reader_qos = getDataQos<DataReaderQos>(qos_profile, DATAREADER_QOS_DEFAULT);
    reader_ = subscriber_->create_datareader(topic_, reader_qos, this, StatusMask::all());
    if (reader_ == nullptr)
    {
        throw std::runtime_error("Employee DataReader initialization failed");
    }
}

easyddsSubscriberApp::~easyddsSubscriberApp()
{
    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        factory_->delete_participant(participant_);
    }
}

void easyddsSubscriberApp::on_subscription_matched(
    DataReader * /*reader*/,
    const SubscriptionMatchedStatus &info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Employee Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Employee Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void easyddsSubscriberApp::on_data_available(
    DataReader *reader)
{
    Employee sample_;
    SampleInfo info;
    while ((!is_stopped()) && (RETCODE_OK == reader->take_next_sample(&sample_, &info)))
    {
        if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
        {
            std::string printInfo = "Send [topic: " + m_topicName + "] Sample: " + std::to_string(m_sampleCount++) + "  \n";
            receivedMsg(printInfo);
        }
    }
}

void easyddsSubscriberApp::run()
{
    // std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    // terminate_cv_.wait(lck, [this]
    //         {
    //             return is_stopped();
    //         });
}

bool easyddsSubscriberApp::is_stopped()
{
    return stop_.load();
}

void easyddsSubscriberApp::stop()
{
    stop_.store(true);
    terminate_cv_.notify_all();
}

void easyddsSubscriberApp::onMessageReceived(const std::function<void(std::string)> &func)
{
    receivedMsg = func;
}

bool easyddsSubscriberApp::getIsStopped()
{
    return is_stopped();
}
