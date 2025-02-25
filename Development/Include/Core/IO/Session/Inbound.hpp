#pragma once

#include "Core/Result.hpp"
#include "Core/Span.hpp"

#include <chrono>

namespace shmit
{
namespace io
{
namespace session
{

class Inbound
{
public:
    using Result = BinaryResult;

    virtual size_t InputBytesAvailable() const noexcept = 0;

    virtual Result Request(Span<uint8_t> rx, std::chrono::microseconds timeout) = 0;
};

} // namespace session
} // namespace io
} // namespace shmit