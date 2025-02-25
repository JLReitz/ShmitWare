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

/**!
 * @brief Output data buffer interface
 *
 */
class Outbound
{
public:
    using Result = BinaryResult;

    virtual size_t OutputBytesAvailable() const noexcept = 0;

    virtual Result Post(Span<uint8_t const> tx, std::chrono::microseconds timeout) noexcept = 0;
};

} // namespace session
} // namespace io
} // namespace shmit