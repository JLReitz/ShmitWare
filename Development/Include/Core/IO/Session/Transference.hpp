#pragma once

#include "Core/Result.hpp"
#include "Core/Span.hpp"

namespace shmit
{
namespace io
{
namespace session
{

class Transference
{
public:
    enum class ResultCode : uint8_t
    {
        kFailed   = 0,
        kComplete = 1,
        kPending  = 2
    };

    using Result = EnumeratedResult<ResultCode, ResultCode::kComplete, ResultCode::kFailed>;

    Transference() = delete;

    Transference(Span<uint8_t> data) noexcept;
    Transference(Span<uint8_t const> data) noexcept;

    Transference(Transference const& copy) = default;
    Transference(Transference&& move)      = default;

    ~Transference() = default;

    Transference& operator=(Transference const& copy) = default;
    Transference& operator=(Transference&& move)      = default;

    Span<uint8_t const> GetData() const noexcept;

    Result GetResult() const noexcept;

    void SetResult(Result result) noexcept;

private:
    Result              m_result;
    Span<uint8_t const> m_data;
};

} // namespace session
} // namespace io
} // namespace shmit
