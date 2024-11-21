#ifndef EASYDDS_APPLICATION_HPP
#define EASYDDS_APPLICATION_HPP

#include <atomic>
#include <memory>
#include <string>
#include <fastdds/dds/domain/DomainParticipant.hpp>
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
#include "easyddsType.hpp"

class easyddsPublisherApp;
class easyddsSubscriberApp;
class easyddsClientPublisherApp;
class easyddsClientSubscriberApp;
class easyddsServerApp;
class easyddsApplication
{
public:
    //! Virtual destructor
    virtual ~easyddsApplication() = default;

    //! Run application
    virtual void run() = 0;

    //! Trigger the end of execution
    virtual void stop() = 0;

    virtual bool send(const std::string &msg) { return true; }

    virtual void onMessageReceived(const std::function<void(std::string)> &func) {}
    
    virtual bool getIsStopped() = 0;

    static std::shared_ptr<easyddsPublisherApp> createPublisher(
        const std::string &topic_name,
        const int &domain_id = 0,
        const qos_profile_s &qos_profile = qos_profile_default,
        const bool &open_monitor = false,
        const MONITOR_TOPIC::monitorItems &items = MONITOR_TOPIC::monitorItems_default);

    static std::shared_ptr<easyddsSubscriberApp> createSubscriber(
        const std::string &topic_name,
        const int &domain_id = 0,
        const qos_profile_s &qos_profile = qos_profile_default,
        const bool &open_monitor = false,
        const MONITOR_TOPIC::monitorItems &items = MONITOR_TOPIC::monitorItems_default);
    
    static std::shared_ptr<easyddsClientPublisherApp> createClientPublisher(
        const std::string &topic_name,
        const int &domain_id = 0,
        const qos_profile_s &qos_profile = qos_profile_default,
        const bool &open_monitor = false,
        const MONITOR_TOPIC::monitorItems &items = MONITOR_TOPIC::monitorItems_default,
        const SERVER::client_config &config = SERVER::client_config());

    static std::shared_ptr<easyddsClientSubscriberApp> createClientSubscriber(  
        const std::string &topic_name,
        const int &domain_id = 0,
        const qos_profile_s &qos_profile = qos_profile_default,
        const bool &open_monitor = false,
        const MONITOR_TOPIC::monitorItems &items = MONITOR_TOPIC::monitorItems_default,
        const SERVER::client_config &config = SERVER::client_config());

    static std::shared_ptr<easyddsServerApp> createServer(  
        const int &domain_id = 0,
        const qos_profile_s &qos_profile = qos_profile_default,
        const bool &open_monitor = false,
        const MONITOR_TOPIC::monitorItems &items = MONITOR_TOPIC::monitorItems_default,
        const SERVER::server_config &config  = SERVER::server_config());



protected:
    DomainParticipantQos getDomainParticipantQos(const bool &monitorEnabled, const MONITOR_TOPIC::monitorItems &items);
    
    DomainParticipantQos getClientDomainParticipantQos(const bool &monitorEnabled, const MONITOR_TOPIC::monitorItems &items, const SERVER::client_config &config);
   
    DomainParticipantQos getServerDomainParticipantQos(const bool &monitorEnabled, const MONITOR_TOPIC::monitorItems &items, const SERVER::server_config &config);
   
    template <typename T>
    T getDataQos(const qos_profile_s &qos_profile, T defaultValue);
};

template <typename T>
T easyddsApplication::getDataQos(const qos_profile_s &qos_profile, T defaultValue)
{
    T qos = defaultValue;
    qos.history().kind = qos_profile.history;
    qos.history().depth = qos_profile.depth;
    qos.reliability().kind = qos_profile.reliability;
    qos.durability().kind = qos_profile.durability;
    qos.deadline().period.seconds = qos_profile.deadline.sec;
    qos.deadline().period.nanosec = qos_profile.deadline.nsec;
    qos.lifespan().duration.seconds = qos_profile.lifespan.sec;
    qos.lifespan().duration.nanosec = qos_profile.lifespan.nsec;
    qos.liveliness().kind = qos_profile.liveliness;
    qos.liveliness().lease_duration.seconds = qos_profile.liveliness_lease_duration.sec;
    qos.liveliness().lease_duration.nanosec = qos_profile.liveliness_lease_duration.nsec;
    qos.liveliness().announcement_period = {TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS};
    return qos;
}

#endif // EASYDDS_APPLICATION_HPP