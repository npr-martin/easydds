#ifndef EASYDDS_CLIENT_SUBSCRIBER_APP_HPP
#define EASYDDS_CLIENT_SUBSCRIBER_APP_HPP

#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "easydds.hpp"
#include "easyddsApplication.hpp"

using namespace eprosima;
using namespace eprosima::fastdds;
using namespace eprosima::fastdds::dds;

class easyddsClientSubscriberApp : public easyddsApplication, public DataReaderListener
{
public:
        easyddsClientSubscriberApp(
            const std::string &topic_name,
            const int &domain_id = 0,
            const qos_profile_s &qos_profile = qos_profile_default,
            const bool &open_monitor = false,
            const MONITOR_TOPIC::monitorItems &items = MONITOR_TOPIC::monitorItems_default,
            const SERVER::client_config &config = SERVER::client_config());

        ~easyddsClientSubscriberApp();

        //! Subscription callback
        void on_data_available(
            DataReader *reader) override;

        //! Subscriber matched method
        void on_subscription_matched(
            DataReader *reader,
            const SubscriptionMatchedStatus &info) override;

        //! Run subscriber
        void run() override;

        //! Trigger the end of execution
        void stop() override;

        bool getIsStopped() override;
        
        void onMessageReceived(const std::function<void(std::string)> &func) override;

private:
        std::function<void(std::string)> receivedMsg;
        //! Return the current state of execution
        bool is_stopped();

        DomainParticipant *participant_;

        Subscriber *subscriber_;

        Topic *topic_;

        DataReader *reader_;

        TypeSupport type_;

        std::atomic<bool> stop_;

        int m_sampleCount;
};

#endif // EASYDDS_CLIENT_SUBSCRIBER_APP_HPP
