#pragma once

#include "Core/IO/Session/Outbound.hpp"

#include <iostream>

namespace shmit
{
namespace io
{
namespace serial
{

class Channel
{
public:
    Channel(std::streambuf& stream_buffer);

    std::iostream& GetStream();

private:
    std::streambuf& m_stream_buffer;
    std::iostream   m_iostream;
};

} // namespace serial
} // namespace io
} // namespace shmit