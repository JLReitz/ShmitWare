#pragma once

#include "Core/Result.hpp"

namespace shmit
{
namespace io
{

template<typename T>
class Output
{
public:
    using value_type = std::decay_t<T>;

    virtual BinaryResult Put(value_type const& data) noexcept = 0;

protected:
    virtual ~Output() = default;
};

} // namespace io
} // namespace shmit