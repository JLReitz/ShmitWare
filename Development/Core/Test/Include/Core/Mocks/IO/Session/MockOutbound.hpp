#pragma once

#include <Core/IO/Session/Outbound.hpp>
#include <Core/Span.hpp>

#include <gmock/gmock.h>

#include <chrono>

class MockOutbound : public shmit::io::session::Outbound
{
public:
    using Result = shmit::io::session::Outbound::Result;

    MOCK_METHOD(size_t, OutputBytesAvailable, (), (const, noexcept));
    MOCK_METHOD(Result, Post, (shmit::Span<uint8_t const> tx, std::chrono::microseconds timeout), (noexcept));
};