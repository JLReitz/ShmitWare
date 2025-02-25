#pragma once

#include "Core/StdTypes.hpp"

#include <type_traits>

namespace shmit
{

template<typename EnumT, EnumT SuccessV, EnumT FailureV>
class EnumeratedResult
{
public:
    static_assert(std::is_enum<EnumT>::value, "`EnumT` must be a scoped enumerator type");

    using Code       = typename std::remove_cv<EnumT>::type;
    using Underlying = typename std::underlying_type<EnumT>::type;

    constexpr static Code kFailureCode {FailureV};
    constexpr static Code kSuccessCode {SuccessV};

    // EnumeratedResult is not trivially constructable
    EnumeratedResult() = delete;

    constexpr EnumeratedResult(Code code);

    constexpr EnumeratedResult(EnumeratedResult const& copy) = default;
    constexpr EnumeratedResult(EnumeratedResult&& move)      = default;

    ~EnumeratedResult() = default;

    constexpr EnumeratedResult& operator=(EnumeratedResult const& copy) = default;
    constexpr EnumeratedResult& operator=(EnumeratedResult&& move)      = default;

    constexpr static EnumeratedResult Failure() noexcept;

    constexpr static EnumeratedResult Success() noexcept;

    constexpr bool IsFailure() const noexcept;

    constexpr bool IsSuccess() const noexcept;

    constexpr operator Code() const noexcept;

    constexpr operator Underlying() const noexcept;

    constexpr bool operator==(EnumeratedResult const& rhs);

    constexpr bool operator==(Code const& rhs);

    constexpr bool operator!=(EnumeratedResult const& rhs);

    constexpr bool operator!=(Code const& rhs);

private:
    Code m_code {};
};

enum class BinaryResultCode : uint8_t
{
    kFailed    = 0,
    kSucceeded = 1
};

using BinaryResult = EnumeratedResult<BinaryResultCode, BinaryResultCode::kSucceeded, BinaryResultCode::kFailed>;

template<typename EnumT, EnumT SuccessV, EnumT FailureV>
constexpr EnumeratedResult<EnumT, SuccessV, FailureV>::EnumeratedResult(Code code) : m_code {code}
{
}

template<typename EnumT, EnumT SuccessV, EnumT FailureV>
constexpr EnumeratedResult<EnumT, SuccessV, FailureV> EnumeratedResult<EnumT, SuccessV, FailureV>::Failure() noexcept // Static method
{
    return EnumeratedResult {kFailureCode};
}

template<typename EnumT, EnumT SuccessV, EnumT FailureV>
constexpr EnumeratedResult<EnumT, SuccessV, FailureV> EnumeratedResult<EnumT, SuccessV, FailureV>::Success() noexcept // Static method
{
    return EnumeratedResult {kSuccessCode};
}

template<typename EnumT, EnumT SuccessV, EnumT FailureV>
constexpr bool EnumeratedResult<EnumT, SuccessV, FailureV>::IsFailure() const noexcept
{
    return (m_code == kFailureCode);
}

template<typename EnumT, EnumT SuccessV, EnumT FailureV>
constexpr bool EnumeratedResult<EnumT, SuccessV, FailureV>::IsSuccess() const noexcept
{
    return (m_code == kSuccessCode);
}

template<typename EnumT, EnumT SuccessV, EnumT FailureV>
constexpr EnumeratedResult<EnumT, SuccessV, FailureV>::operator Code() const noexcept
{
    return m_code;
}

template<typename EnumT, EnumT SuccessV, EnumT FailureV>
constexpr EnumeratedResult<EnumT, SuccessV, FailureV>::operator Underlying() const noexcept
{
    return static_cast<Underlying>(m_code);
}

template<typename EnumT, EnumT SuccessV, EnumT FailureV>
constexpr bool EnumeratedResult<EnumT, SuccessV, FailureV>::operator==(EnumeratedResult const& rhs)
{
    return (m_code == rhs.m_code);
}

template<typename EnumT, EnumT SuccessV, EnumT FailureV>
constexpr bool EnumeratedResult<EnumT, SuccessV, FailureV>::operator==(Code const& rhs)
{
    return (m_code == rhs);
}

template<typename EnumT, EnumT SuccessV, EnumT FailureV>
constexpr bool EnumeratedResult<EnumT, SuccessV, FailureV>::operator!=(EnumeratedResult const& rhs)
{
    return !(*this == rhs);
}

template<typename EnumT, EnumT SuccessV, EnumT FailureV>
constexpr bool EnumeratedResult<EnumT, SuccessV, FailureV>::operator!=(Code const& rhs)
{
    return !(*this == rhs);
}

} // namespace shmit