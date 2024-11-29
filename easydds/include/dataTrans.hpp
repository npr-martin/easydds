// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/**
 * @file database_queue.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__DATABASE_QUEUE_HPP
#define FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__DATABASE_QUEUE_HPP

#include <condition_variable>
#include <cstring>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include <fastdds/statistics/types/types.hpp>

namespace EASYDDS
{
    using StatisticsWriterReaderData = eprosima::fastdds::statistics::WriterReaderData;
    using StatisticsLocator2LocatorData = eprosima::fastdds::statistics::Locator2LocatorData;
    using StatisticsEntityData = eprosima::fastdds::statistics::EntityData;
    using StatisticsEntity2LocatorTraffic = eprosima::fastdds::statistics::Entity2LocatorTraffic;
    using StatisticsEntityCount = eprosima::fastdds::statistics::EntityCount;
    using StatisticsDiscoveryTime = eprosima::fastdds::statistics::DiscoveryTime;
    using StatisticsSampleIdentityCount = eprosima::fastdds::statistics::SampleIdentityCount;
    using StatisticsPhysicalData = eprosima::fastdds::statistics::PhysicalData;
    using StatisticsReceivedData = eprosima::fastdds::statistics::ReceivedData;
    using StatisticsEntityId = eprosima::fastdds::statistics::detail::EntityId_s;
    using StatisticsGuidPrefix = eprosima::fastdds::statistics::detail::GuidPrefix_s;
    using StatisticsGuid = eprosima::fastdds::statistics::detail::GUID_s;
    using StatisticsSequenceNumber = eprosima::fastdds::statistics::detail::SequenceNumber_s;
    using StatisticsSampleIdentity = eprosima::fastdds::statistics::detail::SampleIdentity_s;
    using StatisticsLocator = eprosima::fastdds::statistics::detail::Locator_s;

    std::string deserialize_guid(
            StatisticsGuid data)
    {
        eprosima::fastdds::rtps::GUID_t guid;
        memcpy(guid.guidPrefix.value, data.guidPrefix().value().data(), eprosima::fastdds::rtps::GuidPrefix_t::size);
        memcpy(guid.entityId.value, data.entityId().value().data(), eprosima::fastdds::rtps::EntityId_t::size);
        std::stringstream ss;
        ss << guid;
        return ss.str();
    }

    std::string deserialize_guid(
            StatisticsLocator data)
    {
        if (data.port() != 0)
        {
            throw std::runtime_error("Wrong format: src_locator.port must be 0");
        }
        eprosima::fastdds::rtps::GUID_t guid;
        memcpy(guid.guidPrefix.value, data.address().data(), eprosima::fastdds::rtps::GuidPrefix_t::size);
        memcpy(guid.entityId.value,
                data.address().data() + eprosima::fastdds::rtps::GuidPrefix_t::size,
                eprosima::fastdds::rtps::EntityId_t::size);
        std::stringstream ss;
        ss << guid;
        return ss.str();
    }

    std::string deserialize_locator(
            StatisticsLocator data)
    {
        int32_t kind = data.kind();
        uint32_t port = data.port();
        std::array<uint8_t, 16> address = data.address();

        eprosima::fastdds::rtps::Locator_t locator(kind, port);
        memcpy(locator.address, address.data(), address.size());
        std::stringstream ss;
        ss << locator;
        return ss.str();
    }

    uint64_t deserialize_sequence_number(
            StatisticsSequenceNumber data)
    {
        int32_t high = data.high();
        uint32_t low = data.low();

        return eprosima::fastdds::rtps::SequenceNumber_t(high, low).to64long();
    }

    std::pair<std::string, uint64_t> deserialize_sample_identity(
            StatisticsSampleIdentity data)
    {
        std::string writer_guid = deserialize_guid(data.writer_guid());
        uint64_t sequence_number = deserialize_sequence_number(data.sequence_number());

        return std::make_pair(writer_guid, sequence_number);
    }

}

#endif //FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__DATABASE_QUEUE_HPP
