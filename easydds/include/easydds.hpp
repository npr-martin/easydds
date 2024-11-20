#ifndef EASYDDS_HPP
#define EASYDDS_HPP

#include <cstdint>
#include <string>
#include <utility>
#include <fastcdr/cdr/fixed_size_string.hpp>

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec(dllexport)
#else
#define eProsima_user_DllExport
#endif // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif // _WIN32

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(EASYDDS_SOURCE)
#define EASYDDS_DllAPI __declspec(dllexport)
#else
#define EASYDDS_DllAPI __declspec(dllimport)
#endif // EASYDDS_SOURCE
#else
#define EASYDDS_DllAPI
#endif // EPROSIMA_USER_DLL_EXPORT
#else
#define EASYDDS_DllAPI
#endif // _WIN32

/*!
 * @brief This class represents the structure Employee defined by the user in the IDL file.
 * @ingroup easydds
 */
class Employee
{
public:
    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport Employee()
    {
    }

    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~Employee()
    {
    }

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object Employee that will be copied.
     */
    eProsima_user_DllExport Employee(
        const Employee &x)
    {
        m_text = x.m_text;
    }

    eProsima_user_DllExport Employee(
        const std::string &x)
    {
        m_text = x;
    }
    /*!
     * @brief Move constructor.
     * @param x Reference to the object Employee that will be copied.
     */
    eProsima_user_DllExport Employee(
        Employee &&x) noexcept
    {
        m_text = std::move(x.m_text);
    }

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object Employee that will be copied.
     */
    eProsima_user_DllExport Employee &operator=(
        const Employee &x)
    {

        m_text = x.m_text;

        return *this;
    }

    /*!
     * @brief Move assignment.
     * @param x Reference to the object Employee that will be copied.
     */
    eProsima_user_DllExport Employee &operator=(
        Employee &&x) noexcept
    {

        m_text = std::move(x.m_text);
        return *this;
    }

    /*!
     * @brief Comparison operator.
     * @param x Employee object to compare.
     */
    eProsima_user_DllExport bool operator==(
        const Employee &x) const
    {
        return (m_text == x.m_text);
    }

    /*!
     * @brief Comparison operator.
     * @param x Employee object to compare.
     */
    eProsima_user_DllExport bool operator!=(
        const Employee &x) const
    {
        return !(*this == x);
    }

    /*!
     * @brief This function copies the value in member text
     * @param _text New value to be copied in member text
     */
    eProsima_user_DllExport void text(
        const std::string &_text)
    {
        m_text = _text;
    }

    /*!
     * @brief This function moves the value in member text
     * @param _text New value to be moved in member text
     */
    eProsima_user_DllExport void text(
        std::string &&_text)
    {
        m_text = std::move(_text);
    }

    /*!
     * @brief This function returns a constant reference to member text
     * @return Constant reference to member text
     */
    eProsima_user_DllExport const std::string &text() const
    {
        return m_text;
    }

    /*!
     * @brief This function returns a reference to member text
     * @return Reference to member text
     */
    eProsima_user_DllExport std::string &text()
    {
        return m_text;
    }

private:
    std::string m_text;
};

#endif // EASYDDS_HPP_
