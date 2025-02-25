#pragma once

#include "Core/Result.hpp"

namespace shmit
{
namespace io
{

template<typename T>
class Input
{
public:
    using value_type = std::decay_t<T>;

    virtual BinaryResult Get(value_type& data) noexcept = 0;

protected:
    virtual ~Input() = default;
};

} // namespace io
} // namespace shmit