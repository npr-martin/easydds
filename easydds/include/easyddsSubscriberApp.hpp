#ifndef EASYDDS_SUBSCRIBER_APP_HPP
#define EASYDDS_SUBSCRIBER_APP_HPP

#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "easydds.hpp"
#include "easyddsApplication.hpp"

class easyddsSubscriberApp : public easyddsApplication,
                             public eprosima::fastdds::dds::DataReaderListener
{
public:
        easyddsSubscriberApp(
            const std::string &topic_name,
            const int &domain_id = 0,
            const qos_profile_s &qos_profile = qos_profile_default,
            const bool &open_monitor = false,
            const MONITOR_TOPIC::monitorItems &items = MONITOR_TOPIC::monitorItems_default);

        virtual ~easyddsSubscriberApp();

        //! Subscription callback
        void on_data_available(
            eprosima::fastdds::dds::DataReader *reader) override;

        //! Subscriber matched method
        void on_subscription_matched(
            eprosima::fastdds::dds::DataReader *reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus &info) override;

        //! Run subscriber
        void run() override;

        //! Trigger the end of execution
        void stop() override;

        void onMessageReceived(const std::function<void(std::string)> &func) override;

        bool getIsStopped() override;

private:
        //! Return the current state of execution
        bool is_stopped();

        std::function<void(std::string)> receivedMsg;

        std::shared_ptr<eprosima::fastdds::dds::DomainParticipantFactory> factory_;
        eprosima::fastdds::dds::DomainParticipant *participant_;
        eprosima::fastdds::dds::Subscriber *subscriber_;
        eprosima::fastdds::dds::Topic *topic_;
        eprosima::fastdds::dds::DataReader *reader_;
        eprosima::fastdds::dds::TypeSupport type_;
        std::atomic<bool> stop_;
        mutable std::mutex terminate_cv_mtx_;
        std::condition_variable terminate_cv_;
        std::string m_topicName;
        int m_sampleCount;
};

#endif // EASYDDS_SUBSCRIBER_APP_HPP