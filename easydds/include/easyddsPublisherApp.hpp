#ifndef EASYDDS_PUBLISHER_APP_HPP
#define EASYDDS_PUBLISHER_APP_HPP

#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "easyddsApplication.hpp"

class easyddsPublisherApp : public easyddsApplication,
                            public eprosima::fastdds::dds::DataWriterListener
{
public:
        easyddsPublisherApp(
            const std::string &topic_name,
            const int &domain_id = 0,
            const qos_profile_s &qos_profile = qos_profile_default,
            const bool &open_monitor = false,
            const MONITOR_TOPIC::monitorItems &items = MONITOR_TOPIC::monitorItems_default);

        ~easyddsPublisherApp();

        //! Publisher matched method
        void on_publication_matched(
            eprosima::fastdds::dds::DataWriter *writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus &info) override;

        //! Run publisher
        void run() override;

        //! Trigger the end of execution
        void stop() override;

        bool send(const std::string &msg) override;

        bool getIsStopped() override;

private:
        //! Return the current state of execution
        bool is_stopped();

        std::shared_ptr<eprosima::fastdds::dds::DomainParticipantFactory> factory_;
        eprosima::fastdds::dds::DomainParticipant *participant_;
        eprosima::fastdds::dds::Publisher *publisher_;
        eprosima::fastdds::dds::Topic *topic_;
        eprosima::fastdds::dds::DataWriter *writer_;
        eprosima::fastdds::dds::TypeSupport type_;
        std::condition_variable cv_;
        int32_t matched_;
        std::mutex mutex_;
        std::atomic<bool> stop_;
        std::string m_topicName;
};

#endif // EASYDDS_PUBLISHER_APP_HPP