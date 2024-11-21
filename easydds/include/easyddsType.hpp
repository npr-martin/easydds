#ifndef EASYDDS_TYPE_H
#define EASYDDS_TYPE_H
#include <fastdds/dds/core/policy/QosPolicies.hpp>

using namespace eprosima::fastdds::dds;
struct rmw_time_s
{
    /// Seconds component
    uint64_t sec;

    /// Nanoseconds component
    uint64_t nsec;
};

struct tims_s
{
    int32_t sec;
    uint32_t nsec;
};

struct qos_profile_s
{
    enum HistoryQosPolicyKind history;

    size_t depth;

    enum ReliabilityQosPolicyKind reliability;

    enum DurabilityQosPolicyKind durability;

    struct rmw_time_s deadline;

    struct rmw_time_s lifespan;

    enum LivelinessQosPolicyKind liveliness;

    struct tims_s liveliness_lease_duration;
};
static const qos_profile_s qos_profile_default =
    {
        KEEP_LAST_HISTORY_QOS,
        20,
        RELIABLE_RELIABILITY_QOS,
        VOLATILE_DURABILITY_QOS,
        {0, 0},
        {0, 0},
        MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
        {TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS}};

namespace MONITOR_TOPIC
{
    enum monitorItems : uint64_t
    {
        HISTORY_LATENCY_TOPIC = 1 << 0,
        NETWORK_LATENCY_TOPIC = 1 << 1,
        PUBLICATION_THROUGHPUT_TOPIC = 1 << 2,
        SUBSCRIPTION_THROUGHPUT_TOPIC = 1 << 3,
        RTPS_SENT_TOPIC = 1 << 4,
        RTPS_LOST_TOPIC = 1 << 5,
        HEARTBEAT_COUNT_TOPIC = 1 << 6,
        ACKNACK_COUNT_TOPIC = 1 << 7,
        NACKFRAG_COUNT_TOPIC = 1 << 8,
        GAP_COUNT_TOPIC = 1 << 9,
        DATA_COUNT_TOPIC = 1 << 10,
        RESENT_DATAS_TOPIC = 1 << 11,
        SAMPLE_DATAS_TOPIC = 1 << 12,
        PDP_PACKETS_TOPIC = 1 << 13,
        EDP_PACKETS_TOPIC = 1 << 14,
        PHYSICAL_DATA_TOPIC = 1 << 15
    };
    static const monitorItems monitorItems_default = static_cast<monitorItems>(65535);
    static std::string transTopic(monitorItems item)
    {
        switch (item)
        {
        case HISTORY_LATENCY_TOPIC:
            return "HISTORY_LATENCY_TOPIC";
            break;
        case NETWORK_LATENCY_TOPIC:
            return "NETWORK_LATENCY_TOPIC";
            break;
        case PUBLICATION_THROUGHPUT_TOPIC:
            return "PUBLICATION_THROUGHPUT_TOPIC";
            break;
        case SUBSCRIPTION_THROUGHPUT_TOPIC:
            return "SUBSCRIPTION_THROUGHPUT_TOPIC";
            break;
        case RTPS_SENT_TOPIC:
            return "RTPS_SENT_TOPIC";
            break;
        case RTPS_LOST_TOPIC:
            return "RTPS_LOST_TOPIC";
            break;
        case HEARTBEAT_COUNT_TOPIC:
            return "HEARTBEAT_COUNT_TOPIC";
            break;
        case ACKNACK_COUNT_TOPIC:
            return "ACKNACK_COUNT_TOPIC";
            break;
        case NACKFRAG_COUNT_TOPIC:
            return "NACKFRAG_COUNT_TOPIC";
            break;
        case GAP_COUNT_TOPIC:
            return "GAP_COUNT_TOPIC";
            break;
        case RESENT_DATAS_TOPIC:
            return "RESENT_DATAS_TOPIC";
            break;
        case SAMPLE_DATAS_TOPIC:
            return "SAMPLE_DATAS_TOPIC";
            break;
        case PDP_PACKETS_TOPIC:
            return "PDP_PACKETS_TOPIC";
            break;
        case EDP_PACKETS_TOPIC:
            return "EDP_PACKETS_TOPIC";
            break;
        case PHYSICAL_DATA_TOPIC:
            return "PHYSICAL_DATA_TOPIC";
            break;
        default:
            break;
        }
        return std::string();
    }

}
namespace SERVER
{
    enum class TransportKind : uint8_t
    {
        UDPv4,
        UDPv6,
        TCPv4,
        TCPv6,
        SHM,
    };
    struct client_config
    {
        uint16_t connection_port{16166};
        std::string connection_address{"127.0.0.1"};
        TransportKind transport_kind{TransportKind::UDPv4};
    };

    //! Server configuration structure
    //! A server can, in turn, act as a client
    struct server_config : public client_config
    {
        bool is_also_client{false};
        uint16_t listening_port{16166};
        uint16_t timeout{0};
        std::string listening_address{"127.0.0.1"};
    };
    inline bool is_ip(
        const std::string ip_str)
    {
        return eprosima::fastdds::rtps::IPLocator::isIPv4(ip_str) || eprosima::fastdds::rtps::IPLocator::isIPv6(ip_str);
    }

    inline std::string get_ip_from_dns(
        const std::string &domain_name,
        TransportKind kind)
    {
        std::pair<std::set<std::string>, std::set<std::string>> dns_response =
            eprosima::fastdds::rtps::IPLocator::resolveNameDNS(domain_name);

        if (kind == TransportKind::UDPv4 || kind == TransportKind::TCPv4)
        {
            if (dns_response.first.empty())
            {
                std::cout << "Not DNS found for IPv4 for " << domain_name << std::endl;
                return "";
            }
            else
            {
                std::string solution(*dns_response.first.begin());
                std::cout << "DNS found for " << domain_name << " => " << solution << std::endl;
                return solution;
            }
        }
        else if (kind == TransportKind::UDPv6 || kind == TransportKind::TCPv6)
        {
            if (dns_response.second.empty())
            {
                std::cout << "Not DNS found for IPv6 for " << domain_name << std::endl;
                return "";
            }
            else
            {
                std::string solution(*dns_response.second.begin());
                std::cout << "DNS found for " << domain_name << " => " << solution << std::endl;
                return solution;
            }
        }

        return domain_name;
    }

}
#endif // EASYDDS_TYPE_H