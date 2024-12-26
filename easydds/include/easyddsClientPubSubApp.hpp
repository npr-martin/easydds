#ifndef EASYDDS_CLIENT_PUBSUB_APP_HPP
#define EASYDDS_CLIENT_PUBSUB_APP_HPP

#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "easyddsApplication.hpp"
#include "easydds.hpp"

using namespace eprosima;
using namespace eprosima::fastdds;
using namespace eprosima::fastdds::dds;

class easyddsClientPubSubApp : public easyddsApplication, public DataReaderListener, public DataWriterListener
{
public:
        easyddsClientPubSubApp(const std::string &pub_topic_name, const std::string &sub_topic_name,
                               const int &domain_id = 0,
                               const EASYDDS::easyddsClientConfig &config = EASYDDS::easyddsClientConfig());

        ~easyddsClientPubSubApp();

        //! Publisher matched method
        void on_publication_matched(
            DataWriter *writer,
            const PublicationMatchedStatus &info) override;

        //! Subscriber matched method
        void on_subscription_matched(
            DataReader *reader,
            const SubscriptionMatchedStatus &info) override;

        //! Subscription callback
        void on_data_available(
            DataReader *reader) override;

        //! Run publisher
        void run() override;

        //! Stop publisher
        void stop() override;

        bool getIsStopped() override;

        bool send(const std::string &msg) override;

        std::string getGUID() override;

        void onMessageReceived(const std::function<void(std::string)> &func) override;

private:
        //! Return the current state of execution
        bool is_stopped();

        std::function<void(std::string)> receivedMsg;

        DomainParticipant *pub_participant_;
        DomainParticipant *sub_participant_;

        Publisher * publisher_;
        Subscriber* subscriber_;

        Topic *pub_topic_;
        Topic *sub_topic_;

        DataWriter* writer_;
        DataReader* reader_;

        TypeSupport type_;

        int16_t pub_matched_;
        int16_t sub_matched_;

        std::atomic<bool> stop_;

        int m_sampleCount;
        std::string m_topicName;
};

#endif // EASYDDS_CLIENT_PUBSUB_APP_HPP
