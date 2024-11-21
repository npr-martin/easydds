#include "easyddsServerApp.hpp"

#include <condition_variable>
#include <stdexcept>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>

easyddsServerApp::easyddsServerApp(
        const int &domain_id,
        const EASYDDS::easyddsServerConfig &config)
    : participant_(nullptr)
    , matched_(0)
    , timeout_(config.serverConfig.timeout)
    , start_time_(std::chrono::steady_clock::now())
    , stop_(false)
{
    DomainParticipantQos pqos = getServerDomainParticipantQos(config.open_monitor, config.items, config.serverConfig);
    
    // Create Participant
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos, this);

    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    // if (config.is_also_client)
    // {
    //     std::cout <<
    //         "Server Participant " << pqos.name() <<
    //         " created with GUID " << participant_->guid() <<
    //         " listening in address <" << listening_locator  << "> " <<
    //         " connected to address <" << connection_locator  << "> " <<
    //         std::endl;
    // }
    // else
    // {
    //     std::cout <<
    //         "Server Participant " << pqos.name() <<
    //         " created with GUID " << participant_->guid() <<
    //         " listening in address <" << listening_locator  << "> " <<
    //         std::endl;
    // }
}

easyddsServerApp::~easyddsServerApp()
{
    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void easyddsServerApp::on_participant_discovery(
        DomainParticipant*,
        fastdds::rtps::ParticipantDiscoveryStatus status,
        const ParticipantBuiltinTopicData& info,
        bool& should_be_ignored)
{
    static_cast<void>(should_be_ignored);
    if (status == eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Discovered Participant with GUID " << info.guid << std::endl;
        ++matched_;
    }
    else if (status == eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DROPPED_PARTICIPANT ||
            status == eprosima::fastdds::rtps::ParticipantDiscoveryStatus::REMOVED_PARTICIPANT)
    {
        std::cout << "Dropped Participant with GUID " << info.guid << std::endl;
        --matched_;
    }
}

void easyddsServerApp::run()
{
    while (!is_stopped())
    {
        // Wait for period or stop event
        std::unique_lock<std::mutex> period_lock(mutex_);

        if (timeout_ != 0)
        {
            bool timeout = false;
            cv_.wait_for(period_lock, std::chrono::seconds(timeout_), [&]()
                    {
                        timeout =
                        ((std::chrono::steady_clock::now() - start_time_) >=
                        std::chrono::milliseconds(timeout_ * 1000));
                        return is_stopped() || timeout;
                    });

            if (timeout)
            {
                stop();
            }
        }
        else
        {
            cv_.wait(period_lock, [&]()
                    {
                        return is_stopped();
                    });
        }
    }
}

bool easyddsServerApp::is_stopped()
{
    return stop_.load();
}

void easyddsServerApp::stop()
{
    stop_.store(true);
    cv_.notify_one();
}

bool easyddsServerApp::getIsStopped()
{
     return is_stopped();
}
