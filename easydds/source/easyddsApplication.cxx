#include "easyddsApplication.hpp"

#include "easyddsClientPublisherApp.hpp"
#include "easyddsClientSubscriberApp.hpp"
#include "easyddsServerApp.hpp"

std::shared_ptr<easyddsClientPublisherApp> easyddsApplication::
    createClientPublisher(const std::string &topic_name,
                          const int &domain_id,
                          const EASYDDS::easyddsClientConfig &config)
{
    return std::make_shared<easyddsClientPublisherApp>(topic_name, domain_id, config);
}

std::shared_ptr<easyddsClientSubscriberApp> easyddsApplication::
    createClientSubscriber(const std::string &topic_name,
                           const int &domain_id,
                           const EASYDDS::easyddsClientConfig &config)
{
    return std::make_shared<easyddsClientSubscriberApp>(topic_name, domain_id, config);
}

std::shared_ptr<easyddsServerApp> easyddsApplication::
    createServer(const int &domain_id,
                 const EASYDDS::easyddsServerConfig &config)
{
    return std::make_shared<easyddsServerApp>(domain_id, config);
}

DomainParticipantQos easyddsApplication::getClientDomainParticipantQos(const bool &monitorEnabled,
                                                                       const EASYDDS::monitorItems &items,
                                                                       const EASYDDS::client_config &config)
{
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;

    setMonitorContent(pqos, monitorEnabled, items);
    
    pqos.name("DS-Client_pub");
    pqos.transport().use_builtin_transports = false;

    uint16_t server_port = config.connection_port;

    std::string ip_server_address(config.connection_address);

    // Check if DNS is required
    if (!EASYDDS::is_ip(config.connection_address))
    {
        ip_server_address = EASYDDS::get_ip_from_dns(config.connection_address, config.transport_kind);
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
    case EASYDDS::TransportKind::SHM:
        descriptor = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
        server_locator.kind = LOCATOR_KIND_SHM;
        break;

    case EASYDDS::TransportKind::UDPv4:
    {
        auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
        // descriptor_tmp->interfaceWhiteList.push_back(ip_server_address);
        descriptor = descriptor_tmp;

        server_locator.kind = LOCATOR_KIND_UDPv4;
        eprosima::fastdds::rtps::IPLocator::setIPv4(server_locator, ip_server_address);
        break;
    }

    case EASYDDS::TransportKind::UDPv6:
    {
        auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv6TransportDescriptor>();
        // descriptor_tmp->interfaceWhiteList.push_back(ip_server_address);
        descriptor = descriptor_tmp;

        server_locator.kind = LOCATOR_KIND_UDPv6;
        eprosima::fastdds::rtps::IPLocator::setIPv6(server_locator, ip_server_address);
        break;
    }

    case EASYDDS::TransportKind::TCPv4:
    {
        auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
        descriptor_tmp->add_listener_port(0);
        descriptor = descriptor_tmp;

        server_locator.kind = LOCATOR_KIND_TCPv4;
        eprosima::fastdds::rtps::IPLocator::setLogicalPort(server_locator, server_port);
        eprosima::fastdds::rtps::IPLocator::setIPv4(server_locator, ip_server_address);
        break;
    }

    case EASYDDS::TransportKind::TCPv6:
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

    std::cout << "Publisher Participant " << pqos.name() << " connecting to server <" << server_locator << "> " << std::endl;
    return pqos;
}

DomainParticipantQos easyddsApplication::getServerDomainParticipantQos(const bool &monitorEnabled,
                                                                       const EASYDDS::monitorItems &items,
                                                                       const EASYDDS::server_config &config)
{
    DomainParticipantQos pqos;

    setMonitorContent(pqos, monitorEnabled, items);

    pqos.name("DS-Server");
    pqos.transport().use_builtin_transports = false;

    std::string ip_listening_address(config.listening_address);
    std::string ip_connection_address(config.connection_address);
    // Check if DNS is required
    if (!EASYDDS::is_ip(config.listening_address))
    {
        ip_listening_address = EASYDDS::get_ip_from_dns(config.listening_address, config.transport_kind);
    }

    if (ip_listening_address.empty())
    {
        throw std::runtime_error("Invalid listening address");
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
    case EASYDDS::TransportKind::SHM:
        descriptor = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
        listening_locator.kind = LOCATOR_KIND_SHM;
        connection_locator.kind = LOCATOR_KIND_SHM;
        break;

    case EASYDDS::TransportKind::UDPv4:
    {
        auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
        descriptor = descriptor_tmp;

        listening_locator.kind = LOCATOR_KIND_UDPv4;
        eprosima::fastdds::rtps::IPLocator::setIPv4(listening_locator, ip_listening_address);
        connection_locator.kind = LOCATOR_KIND_UDPv4;
        eprosima::fastdds::rtps::IPLocator::setIPv4(connection_locator, ip_connection_address);
        break;
    }

    case EASYDDS::TransportKind::UDPv6:
    {
        auto descriptor_tmp = std::make_shared<eprosima::fastdds::rtps::UDPv6TransportDescriptor>();
        descriptor = descriptor_tmp;

        listening_locator.kind = LOCATOR_KIND_UDPv6;
        eprosima::fastdds::rtps::IPLocator::setIPv6(listening_locator, ip_listening_address);
        connection_locator.kind = LOCATOR_KIND_UDPv6;
        eprosima::fastdds::rtps::IPLocator::setIPv6(connection_locator, ip_connection_address);
        break;
    }

    case EASYDDS::TransportKind::TCPv4:
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

    case EASYDDS::TransportKind::TCPv6:
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

    return pqos;
}

DomainParticipantQos easyddsApplication::getPubDomainParticipantQos(const bool &monitorEnabled,
                                                                    const EASYDDS::monitorItems &items,
                                                                    const EASYDDS::client_config &config,
                                                                    const uint32_t &samples)
{
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;

    setMonitorContent(pqos, monitorEnabled, items);

    pqos.name("DeliveryMechanisms_pub_participant");
    uint32_t max_samples = samples;
    if (max_samples == 0)
    {
        max_samples = DATAWRITER_QOS_DEFAULT.resource_limits().max_samples_per_instance;
    }

    // Transport default definitions
    pqos.transport().use_builtin_transports = false;

    switch (config.transport_kind)
    {
        case EASYDDS::TransportKind::SHM:
        case EASYDDS::TransportKind::DATA_SHARING:
        {
            std::shared_ptr<SharedMemTransportDescriptor> shm_transport_ =
                    std::make_shared<SharedMemTransportDescriptor>();
            shm_transport_->segment_size(shm_transport_->max_message_size() * max_samples);
            pqos.transport().user_transports.push_back(shm_transport_);
            break;
        }
        case EASYDDS::TransportKind::LARGE_DATA:
        {
            // Large Data is a builtin transport
            pqos.transport().use_builtin_transports = true;
            pqos.setup_transports(BuiltinTransports::LARGE_DATA);
            break;
        }
        case EASYDDS::TransportKind::TCPv4:
        {
            std::shared_ptr<TCPv4TransportDescriptor> tcp_v4_transport_ = std::make_shared<TCPv4TransportDescriptor>();
            pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastdds::dds::c_TimeInfinite;
            pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = eprosima::fastdds::dds::Duration_t(5, 0);
            tcp_v4_transport_->sendBufferSize = 0;
            tcp_v4_transport_->receiveBufferSize = 0;
            std::string tcp_ip_address = "127.0.0.1";
            if (!config.connection_address.empty())
            {
                tcp_ip_address = config.connection_address;
            }
            // Set unicast locators
            Locator_t tcp_v4_locator_;
            tcp_v4_locator_.kind = LOCATOR_KIND_TCPv4;
            IPLocator::setIPv4(tcp_v4_locator_, tcp_ip_address);
            IPLocator::setPhysicalPort(tcp_v4_locator_, 5100);
            pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(tcp_v4_locator_);
            pqos.wire_protocol().default_unicast_locator_list.push_back(tcp_v4_locator_);
            tcp_v4_transport_->set_WAN_address(tcp_ip_address);
            tcp_v4_transport_->add_listener_port(5100);
            pqos.transport().user_transports.push_back(tcp_v4_transport_);
            break;
        }
        case EASYDDS::TransportKind::TCPv6:
        {
            std::shared_ptr<TCPv6TransportDescriptor> tcp_v6_transport_ = std::make_shared<TCPv6TransportDescriptor>();
            pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastdds::dds::c_TimeInfinite;
            pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = eprosima::fastdds::dds::Duration_t(5, 0);
            tcp_v6_transport_->sendBufferSize = 0;
            tcp_v6_transport_->receiveBufferSize = 0;
            std::string tcp_ip_address = "::1";
            if (!config.connection_address.empty())
            {
                tcp_ip_address = config.connection_address;
            }
            // Set unicast locators
            Locator_t tcp_v6_locator_;
            tcp_v6_locator_.kind = LOCATOR_KIND_TCPv6;
            IPLocator::setIPv6(tcp_v6_locator_, tcp_ip_address);
            IPLocator::setPhysicalPort(tcp_v6_locator_, 5100);
            pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(tcp_v6_locator_);
            pqos.wire_protocol().default_unicast_locator_list.push_back(tcp_v6_locator_);
            tcp_v6_transport_->add_listener_port(5100);
            pqos.transport().user_transports.push_back(tcp_v6_transport_);
            break;
        }
        case EASYDDS::TransportKind::UDPv4:
        {
            pqos.transport().user_transports.push_back(std::make_shared<UDPv4TransportDescriptor>());
            break;
        }
        case EASYDDS::TransportKind::UDPv6:
        {
            pqos.transport().user_transports.push_back(std::make_shared<UDPv6TransportDescriptor>());
            break;
        }
        default:
        {
            pqos.transport().use_builtin_transports = true;
            break;
        }
    }
    return pqos;
}

DomainParticipantQos easyddsApplication::getSubDomainParticipantQos(const bool &monitorEnabled,
                                                                    const EASYDDS::monitorItems &items,
                                                                    const EASYDDS::client_config &config,
                                                                    const uint32_t &samples)
{
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;

    setMonitorContent(pqos, monitorEnabled, items);

    pqos.name("DeliveryMechanisms_sub_participant");
    pqos.transport().use_builtin_transports = false;
    uint32_t max_samples = samples;
    if (max_samples == 0)
    {
        max_samples = DATAREADER_QOS_DEFAULT.resource_limits().max_samples_per_instance;
    }

    // Transport default definitions
    pqos.transport().use_builtin_transports = false;
    switch (config.transport_kind)
    {
        case EASYDDS::TransportKind::SHM:
        case EASYDDS::TransportKind::DATA_SHARING:
        {
            std::shared_ptr<SharedMemTransportDescriptor> shm_transport_ =
                    std::make_shared<SharedMemTransportDescriptor>();
            shm_transport_->segment_size(shm_transport_->max_message_size() * max_samples);
            pqos.transport().user_transports.push_back(shm_transport_);
            break;
        }
        case EASYDDS::TransportKind::LARGE_DATA:
        {
            // Large Data is a builtin transport
            pqos.transport().use_builtin_transports = true;
            pqos.setup_transports(BuiltinTransports::LARGE_DATA);
            break;
        }
        case EASYDDS::TransportKind::TCPv4:
        {
            Locator tcp_v4_initial_peers_locator_;
            tcp_v4_initial_peers_locator_.kind = LOCATOR_KIND_TCPv4;
            tcp_v4_initial_peers_locator_.port = 5100;
            std::string tcp_ip_address = "127.0.0.1";
            if (!config.connection_address.empty())
            {
                tcp_ip_address = config.connection_address;
            }
            IPLocator::setIPv4(tcp_v4_initial_peers_locator_, tcp_ip_address);
            pqos.wire_protocol().builtin.initialPeersList.push_back(tcp_v4_initial_peers_locator_);
            pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastdds::dds::c_TimeInfinite;
            pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = eprosima::fastdds::dds::Duration_t(5, 0);
            pqos.transport().user_transports.push_back(std::make_shared<TCPv4TransportDescriptor>());
            break;
        }
        case EASYDDS::TransportKind::TCPv6:
        {
            Locator tcp_v6_initial_peers_locator_;
            tcp_v6_initial_peers_locator_.kind = LOCATOR_KIND_TCPv6;
            tcp_v6_initial_peers_locator_.port = 5100;
            std::string tcp_ip_address = "::1";
            if (!config.connection_address.empty())
            {
                tcp_ip_address = config.connection_address;
            }
            IPLocator::setIPv6(tcp_v6_initial_peers_locator_, tcp_ip_address);
            pqos.wire_protocol().builtin.initialPeersList.push_back(tcp_v6_initial_peers_locator_);
            pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastdds::dds::c_TimeInfinite;
            pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = eprosima::fastdds::dds::Duration_t(5, 0);
            pqos.transport().user_transports.push_back(std::make_shared<TCPv6TransportDescriptor>());
            break;
        }
        case EASYDDS::TransportKind::UDPv4:
        {
            pqos.transport().user_transports.push_back(std::make_shared<UDPv4TransportDescriptor>());
            break;
        }
        case EASYDDS::TransportKind::UDPv6:
        {
            pqos.transport().user_transports.push_back(std::make_shared<UDPv6TransportDescriptor>());
            break;
        }
        default:
        {
            pqos.transport().use_builtin_transports = true;
            break;
        }
    }
    return pqos;
}

void easyddsApplication::setMonitorContent(DomainParticipantQos pqos, const bool &monitorEnabled, const EASYDDS::monitorItems &items)
{

    // add monitor
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
                monitorTopic += EASYDDS::transTopic(static_cast<EASYDDS::monitorItems>(1 << i));
                monitorTopic += ";";
            }
        }
        monitorTopic.pop_back();
        pqos.properties().properties().emplace_back("fastdds.statistics", monitorTopic);
    }
}
