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

using namespace EASYDDS;
using namespace eprosima;
using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

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

    static std::shared_ptr<easyddsClientPublisherApp> createClientPublisher(
        const std::string &topic_name,
        const int &domain_id = 0,
        const EASYDDS::easyddsClientConfig &config = EASYDDS::easyddsClientConfig());

    static std::shared_ptr<easyddsClientSubscriberApp> createClientSubscriber(  
        const std::string &topic_name,
        const int &domain_id = 0,
        const EASYDDS::easyddsClientConfig &config = EASYDDS::easyddsClientConfig());

    static std::shared_ptr<easyddsServerApp> createServer(  
        const int &domain_id = 0,
        const EASYDDS::easyddsServerConfig &config = EASYDDS::easyddsServerConfig());

protected:
    DomainParticipantQos getClientDomainParticipantQos(const bool &monitorEnabled,
                                                       const EASYDDS::monitorItems &items,
                                                       const EASYDDS::client_config &config);

    DomainParticipantQos getServerDomainParticipantQos(const bool &monitorEnabled,
                                                       const EASYDDS::monitorItems &items,
                                                       const EASYDDS::server_config &config);

    DomainParticipantQos getPubDomainParticipantQos(const bool &monitorEnabled,
                                                    const EASYDDS::monitorItems &items,
                                                    const EASYDDS::client_config &config,
                                                    const uint32_t &samples);

    DomainParticipantQos getSubDomainParticipantQos(const bool &monitorEnabled,
                                                    const EASYDDS::monitorItems &items,
                                                    const EASYDDS::client_config &config,
                                                    const uint32_t &samples);

    void setMonitorContent(DomainParticipantQos pqos, const bool &monitorEnabled, const EASYDDS::monitorItems &items);
   
    template <typename T>
    T getDataQos(const qos_profile_s &qos_profile, T defaultValue);
};

template <typename T>
T easyddsApplication::getDataQos(const EASYDDS::qos_profile_s &qos_profile, T defaultValue)
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
   
    uint32_t max_samples = qos_profile.samples;
    if (max_samples == 0)
    {
        max_samples = DATAWRITER_QOS_DEFAULT.resource_limits().max_samples_per_instance;
    }

     qos.resource_limits().max_samples_per_instance = max_samples;
    qos.resource_limits().max_samples = qos.resource_limits().max_instances * max_samples;

    return qos;
}

#endif // EASYDDS_APPLICATION_HPP