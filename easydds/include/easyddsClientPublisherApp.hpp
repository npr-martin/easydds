#ifndef EASYDDS_CLIENT_PUBLIHSER_APP_HPP
#define EASYDDS_CLIENT_PUBLIHSER_APP_HPP

#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "easyddsApplication.hpp"
#include "easydds.hpp"

using namespace eprosima;
using namespace eprosima::fastdds;
using namespace eprosima::fastdds::dds;

class easyddsClientPublisherApp : public easyddsApplication, public DataWriterListener
{
public:
        easyddsClientPublisherApp(const std::string &topic_name,
                                  const int &domain_id = 0,
                                  const EASYDDS::easyddsClientConfig &config = EASYDDS::easyddsClientConfig());

        ~easyddsClientPublisherApp();

        //! Publisher matched method
        void on_publication_matched(
            DataWriter *writer,
            const PublicationMatchedStatus &info) override;

        //! Run publisher
        void run() override;

        //! Stop publisher
        void stop() override;

        bool getIsStopped() override;

        bool send(const std::string &msg) override;

private:
        //! Return the current state of execution
        bool is_stopped();

        DomainParticipant *participant_;

        Publisher *publisher_;

        Topic *topic_;

        DataWriter *writer_;

        TypeSupport type_;

        int16_t matched_;

        std::atomic<bool> stop_;
};

#endif // EASYDDS_CLIENT_PUBLIHSER_APP_HPP
