#ifndef EASYDDS_SERVER_APP_HPP
#define EASYDDS_SERVER_APP_HPP
#include <condition_variable>

#include <fastdds/dds/builtin/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "easyddsApplication.hpp"

using namespace eprosima;
using namespace eprosima::fastdds;
using namespace eprosima::fastdds::dds;

class easyddsServerApp : public easyddsApplication, public DomainParticipantListener
{
public:
    easyddsServerApp(const int &domain_id = 0,
                     const EASYDDS::easyddsServerConfig &config = EASYDDS::easyddsServerConfig());

    ~easyddsServerApp();

    //! Publisher matched method
    void on_participant_discovery(
        DomainParticipant *participant,
        fastdds::rtps::ParticipantDiscoveryStatus status,
        const fastdds::rtps::ParticipantBuiltinTopicData &info,
        bool &should_be_ignored) override;

    //! Run publisher
    void run() override;

    //! Stop publisher
    void stop() override;

    bool getIsStopped() override;

    std::string getGUID() override;

private:
    //! Return the current state of execution
    bool is_stopped();

    DomainParticipant *participant_;

    int16_t matched_;

    std::mutex mutex_;

    uint16_t timeout_;

    std::chrono::steady_clock::time_point start_time_;

    std::condition_variable cv_;

    std::atomic<bool> stop_;
};

#endif // EASYDDS_SERVER_APP_HPP
