#pragma once

#include <ShmitCore/IO/Session/Inbound.hpp>
#include <ShmitCore/Span.hpp>

#include <gmock/gmock.h>

#include <chrono>

class MockInbound : public shmit::io::session::Inbound
{
public:
    using Result = shmit::io::session::Inbound::Result;

    MOCK_METHOD(size_t, InputBytesAvailable, (), (const, noexcept));
    MOCK_METHOD(Result, Request, (shmit::Span<uint8_t> rx, std::chrono::microseconds timeout), (noexcept));
};