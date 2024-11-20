#ifndef EASYDDS_CDR_AUX_HPP
#define EASYDDS_CDR_AUX_HPP

#include "easydds.hpp"

constexpr uint32_t Employee_max_cdr_typesize {264UL};
constexpr uint32_t Employee_max_key_cdr_typesize {0UL};


namespace eprosima {
namespace fastcdr {

class Cdr;
class CdrSizeCalculator;

eProsima_user_DllExport void serialize_key(
        eprosima::fastcdr::Cdr& scdr,
        const Employee& data);


} // namespace fastcdr
} // namespace eprosima

#endif // EASYDDS_CDR_AUX_HPP

