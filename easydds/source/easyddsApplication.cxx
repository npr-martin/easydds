#include "easyddsApplication.hpp"

#include "easyddsPublisherApp.hpp"
#include "easyddsSubscriberApp.hpp"
#include "easyddsClientPublisherApp.hpp"
#include "easyddsClientSubscriberApp.hpp"
#include "easyddsServerApp.hpp"

std::shared_ptr<easyddsPublisherApp> easyddsApplication::
    createPublisher(const std::string &topic_name,
                    const int &domain_id,
                    const qos_profile_s &qos_profile,
                    const bool &open_monitor,
                    const MONITOR_TOPIC::monitorItems &items)
{
    return std::make_shared<easyddsPublisherApp>(topic_name, domain_id, qos_profile, open_monitor, items);
}

std::shared_ptr<easyddsSubscriberApp> easyddsApplication::
    createSubscriber(const std::string &topic_name,
                     const int &domain_id,
                     const qos_profile_s &qos_profile,
                     const bool &open_monitor,
                     const MONITOR_TOPIC::monitorItems &items)
{
    return std::make_shared<easyddsSubscriberApp>(topic_name, domain_id, qos_profile, open_monitor, items);
}

std::shared_ptr<easyddsClientPublisherApp> easyddsApplication::
    createClientPublisher(const std::string &topic_name,
                          const int &domain_id,
                          const qos_profile_s &qos_profile,
                          const bool &open_monitor,
                          const MONITOR_TOPIC::monitorItems &items,
                          const SERVER::client_config &config)
{

    return std::make_shared<easyddsClientPublisherApp>(topic_name, domain_id, qos_profile, open_monitor, items, config);
}

std::shared_ptr<easyddsClientSubscriberApp> easyddsApplication::
    createClientSubscriber(const std::string &topic_name,
                           const int &domain_id,
                           const qos_profile_s &qos_profile,
                           const bool &open_monitor,
                           const MONITOR_TOPIC::monitorItems &items,
                           const SERVER::client_config &config)
{
    return std::make_shared<easyddsClientSubscriberApp>(topic_name, domain_id, qos_profile, open_monitor, items, config);
}

std::shared_ptr<easyddsServerApp> easyddsApplication::
    createServer(const int &domain_id,
                 const qos_profile_s &qos_profile,
                 const bool &open_monitor,
                 const MONITOR_TOPIC::monitorItems &items,
                 const SERVER::server_config &config)
{
    return std::make_shared<easyddsServerApp>(domain_id, qos_profile, open_monitor, items, config);
}

DomainParticipantQos easyddsApplication::getDomainParticipantQos(const bool &monitorEnabled, const MONITOR_TOPIC::monitorItems &items)
{
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    std::string monitorTopic;

    if (!monitorEnabled || items)
    {
        pqos.properties().properties().emplace_back("fastdds.statistics", "");
    }
    else
    {
        for (int i = 1; i < 16; ++i)
        {
            if (items & static_cast<uint64_t>(1 << i))
            {
                monitorTopic += MONITOR_TOPIC::transTopic(static_cast<MONITOR_TOPIC::monitorItems>(1 << i));
                monitorTopic += ";";
            }
        }
        monitorTopic.pop_back();
        pqos.properties().properties().emplace_back("fastdds.statistics", monitorTopic);
    }
    return pqos;
}

DomainParticipantQos easyddsApplication::getClientDomainParticipantQos(const bool &monitorEnabled, const MONITOR_TOPIC::monitorItems &items, const SERVER::client_config &config)
{
    DomainParticipantQos pqos;

    //add monitor 
     std::string monitorTopic;

    if (!monitorEnabled || items)
    {
        pqos.properties().properties().emplace_back("fastdds.statistics", "");
    }
    else
    {
        for (int i = 1; i < 16; ++i)
        {
            if (items & static_cast<uint64_t>(1 << i))
            {
                monitorTopic += MONITOR_TOPIC::transTopic(static_cast<MONITOR_TOPIC::monitorItems>(1 << i));
                monitorTopic += ";";
            }
        }
        monitorTopic.pop_back();
        pqos.properties().properties().emplace_back("fastdds.statistics", monitorTopic);
    }

    pqos.name("DS-Client_pub");
    pqos.transport().use_builtin_transports = false;

    uint16_t server_port = config.connection_port;

    std::string ip_server_address(config.connection_address);

    // Check if DNS is required
    if (!SERVER::is_ip(config.connection_address))
    {
        ip_server_address = SERVER::get_ip_from_dns(config.connection_address, config.transport_kind);
    }

    if (ip_server_address.empty())
    {
        throw std::runtime_error("Invalid connection address");
    }

    // Create DS locator
    eprosima::fastdds::rtps::Locator server_locator;
    eprosima::fastdds::rtps::IPLocator::setPhysicalPort(server_locator, server_port);

    std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> descriptor;

    switch (config.transport_kind)
    {
    case SERVER::TransportKind::SHM:
        descriptor = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
        server_locator.kind = LOCATOR_KIND_SHM;
        break;

    case SERVER::TransportKind::UDPv4:
    {
        auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
        // descriptor_tmp->interfaceWhiteList.push_back(ip_server_address);
        descriptor = descriptor_tmp;

        server_locator.kind = LOCATOR_KIND_UDPv4;
        eprosima::fastdds::rtps::IPLocator::setIPv4(server_locator, ip_server_address);
        break;
    }

    case SERVER::TransportKind::UDPv6:
    {
        auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv6TransportDescriptor>();
        // descriptor_tmp->interfaceWhiteList.push_back(ip_server_address);
        descriptor = descriptor_tmp;

        server_locator.kind = LOCATOR_KIND_UDPv6;
        eprosima::fastdds::rtps::IPLocator::setIPv6(server_locator, ip_server_address);
        break;
    }

    case SERVER::TransportKind::TCPv4:
    {
        auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
        descriptor_tmp->add_listener_port(0);
        descriptor = descriptor_tmp;

        server_locator.kind = LOCATOR_KIND_TCPv4;
        eprosima::fastdds::rtps::IPLocator::setLogicalPort(server_locator, server_port);
        eprosima::fastdds::rtps::IPLocator::setIPv4(server_locator, ip_server_address);
        break;
    }

    case SERVER::TransportKind::TCPv6:
    {
        auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
        descriptor_tmp->add_listener_port(0);
        descriptor = descriptor_tmp;

        server_locator.kind = LOCATOR_KIND_TCPv6;
        eprosima::fastdds::rtps::IPLocator::setLogicalPort(server_locator, server_port);
        eprosima::fastdds::rtps::IPLocator::setIPv6(server_locator, ip_server_address);
        break;
    }

    default:
        break;
    }

    // Set participant as DS CLIENT
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol =
        eprosima::fastdds::rtps::DiscoveryProtocol::CLIENT;

    // Add remote SERVER to CLIENT's list of SERVERs
    pqos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(server_locator);

    // Add descriptor
    pqos.transport().user_transports.push_back(descriptor);
   
     std::cout <<
        "Publisher Participant " << pqos.name() <<
        " connecting to server <" << server_locator  << "> " <<
        std::endl;
        return pqos;
}

DomainParticipantQos easyddsApplication::getServerDomainParticipantQos(const bool &monitorEnabled, const MONITOR_TOPIC::monitorItems &items, const SERVER::server_config &config)
{
    DomainParticipantQos pqos;
    pqos.name("DS-Server");
    pqos.transport().use_builtin_transports = false;

    std::string ip_listening_address(config.listening_address);
    std::string ip_connection_address(config.connection_address);
    // Check if DNS is required
    if (!SERVER::is_ip(config.listening_address))
    {
        ip_listening_address = SERVER::get_ip_from_dns(config.listening_address, config.transport_kind);
    }

    if (ip_listening_address.empty())
    {
        throw std::runtime_error("Invalid listening address");
    }

    // Do the same for connection
    if (config.is_also_client && !SERVER::is_ip(config.connection_address))
    {
        ip_connection_address = SERVER::get_ip_from_dns(config.connection_address, config.transport_kind);
    }

    if (config.is_also_client && ip_connection_address.empty())
    {
        throw std::runtime_error("Invalid connection address");
    }

    // Configure Listening address
    // Create DS SERVER locator
    eprosima::fastdds::rtps::Locator listening_locator;
    eprosima::fastdds::rtps::Locator connection_locator;
    eprosima::fastdds::rtps::IPLocator::setPhysicalPort(listening_locator, config.listening_port);
    eprosima::fastdds::rtps::IPLocator::setPhysicalPort(connection_locator, config.connection_port);

    std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> descriptor;

    switch (config.transport_kind)
    {
        case SERVER::TransportKind::SHM:
            descriptor = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
            listening_locator.kind = LOCATOR_KIND_SHM;
            connection_locator.kind = LOCATOR_KIND_SHM;
            break;

        case SERVER::TransportKind::UDPv4:
        {
            auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
            descriptor = descriptor_tmp;

            listening_locator.kind = LOCATOR_KIND_UDPv4;
            eprosima::fastdds::rtps::IPLocator::setIPv4(listening_locator, ip_listening_address);
            connection_locator.kind = LOCATOR_KIND_UDPv4;
            eprosima::fastdds::rtps::IPLocator::setIPv4(connection_locator, ip_connection_address);
            break;
        }

        case SERVER::TransportKind::UDPv6:
        {
            auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv6TransportDescriptor>();
            descriptor = descriptor_tmp;

            listening_locator.kind = LOCATOR_KIND_UDPv6;
            eprosima::fastdds::rtps::IPLocator::setIPv6(listening_locator, ip_listening_address);
            connection_locator.kind = LOCATOR_KIND_UDPv6;
            eprosima::fastdds::rtps::IPLocator::setIPv6(connection_locator, ip_connection_address);
            break;
        }

        case SERVER::TransportKind::TCPv4:
        {
            auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
            descriptor_tmp->add_listener_port(config.listening_port);
            descriptor = descriptor_tmp;

            listening_locator.kind = LOCATOR_KIND_TCPv4;
            eprosima::fastdds::rtps::IPLocator::setLogicalPort(listening_locator, config.listening_port);
            eprosima::fastdds::rtps::IPLocator::setIPv4(listening_locator, ip_listening_address);
            connection_locator.kind = LOCATOR_KIND_TCPv4;
            eprosima::fastdds::rtps::IPLocator::setIPv4(connection_locator, ip_connection_address);
            eprosima::fastdds::rtps::IPLocator::setLogicalPort(connection_locator, config.connection_port);
            break;
        }

        case SERVER::TransportKind::TCPv6:
        {
            auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
            descriptor_tmp->add_listener_port(config.listening_port);
            descriptor = descriptor_tmp;

            listening_locator.kind = LOCATOR_KIND_TCPv6;
            eprosima::fastdds::rtps::IPLocator::setLogicalPort(listening_locator, config.listening_port);
            eprosima::fastdds::rtps::IPLocator::setIPv6(listening_locator, ip_listening_address);
            connection_locator.kind = LOCATOR_KIND_TCPv6;
            eprosima::fastdds::rtps::IPLocator::setIPv6(connection_locator, ip_connection_address);
            eprosima::fastdds::rtps::IPLocator::setLogicalPort(connection_locator, config.connection_port);
            break;
        }

        default:
            break;
    }

    // Add descriptor
    pqos.transport().user_transports.push_back(descriptor);

    // Set participant as SERVER
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol =
            eprosima::fastdds::rtps::DiscoveryProtocol::SERVER;

    // Set SERVER's listening locator for PDP
    pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(listening_locator);

    // Configure Connection address
    if (config.is_also_client)
    {
        // Add remote SERVER to CLIENT's list of SERVERs
        pqos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(connection_locator);
    }
    
    return pqos;
}
