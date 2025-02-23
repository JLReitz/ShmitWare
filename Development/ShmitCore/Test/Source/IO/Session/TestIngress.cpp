#include "ShmitCore/Mocks/IO/Session/MockInbound.hpp"

#include <ShmitCore/Data/Footprint.hpp>
#include <ShmitCore/IO/Session/Ingress.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <cstring>
#include <limits>

using namespace shmit;
using namespace shmit::io::session;

using ::testing::Invoke;
using ::testing::Return;
using ::testing::_;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Ingress tests               ////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Test the nominal success path for when an object is fetched from an Ingress instance
 *
 */
TEST(Session_Ingress, nominal_success)
{
    using TestValueType = int;

    TestValueType const kExpectedValue {42};

    // Stage mock Inbound session
    MockInbound mock_inbound;
    // Expect plenty of availablities in the session
    EXPECT_CALL(mock_inbound, InputBytesAvailable()).WillOnce(Return(std::numeric_limits<size_t>::max()));
    // Expect request to return success
    EXPECT_CALL(mock_inbound, Request(_, _))
        .WillOnce(Invoke(
            [&](Span<uint8_t> rx, std::chrono::microseconds timeout) -> MockInbound::Result
            {
                static_cast<void>(timeout); // Avoid unused parameter

                // Verify that the byte span is large enough to hold the test value
                bool span_size_matches_expected {data::footprint_size_bytes_v<TestValueType> == rx.size()};
                EXPECT_TRUE(span_size_matches_expected);
                if (!span_size_matches_expected)
                    return MockInbound::Result::Failure();

                // Copy test value in to byte span for Ingress to decode
                std::memcpy(rx.data(), &kExpectedValue, rx.size());
                return MockInbound::Result::Success();
            }));

    // Start the test
    TestValueType          test_value;
    Ingress<TestValueType> test_ingress {mock_inbound};
    ASSERT_TRUE(test_ingress.Get(test_value).IsSuccess());
    ASSERT_EQ(kExpectedValue, test_value);
}

/**
 * @brief Test that a zero-valued duration is passed through Ingress to the connected Inbound session buffer while
 * fetching from an Ingress
 *
 */
TEST(Session_Ingress, no_timeout)
{
    using TestValueType = int;

    // Stage the mock Inbound session
    MockInbound mock_inbound;
    // Expect plenty of availablities in the session
    EXPECT_CALL(mock_inbound, InputBytesAvailable()).WillOnce(Return(std::numeric_limits<size_t>::max()));
    // Expect request to the session with zero duration
    // Don't care about return value for this test
    EXPECT_CALL(mock_inbound, Request(_, std::chrono::microseconds::zero())).WillOnce(Return(MockInbound::Result::Success()));

    // Start the test
    TestValueType          test_value;
    Ingress<TestValueType> test_ingress {mock_inbound};
    test_ingress.Get(test_value);
}

/**
 * @brief Test that a nonzero duration is passed through Ingress to the connected Inbound session buffer while fetching
 * from an Ingress
 *
 */
TEST(Session_Ingress, nonzero_timeout)
{
    using TestValueType = int;

    constexpr std::chrono::microseconds kTestDuration {500};

    // Stage the mock Inbound session
    MockInbound mock_inbound;
    // Expect plenty of availablities in the session
    EXPECT_CALL(mock_inbound, InputBytesAvailable()).WillOnce(Return(std::numeric_limits<size_t>::max()));
    // Expect request to the session with nonzero duration
    EXPECT_CALL(mock_inbound, Request(_, _))
        .WillOnce(Invoke(
            [&](Span<uint8_t> rx, std::chrono::microseconds timeout) -> MockInbound::Result
            {
                // Verify that the timeout supplied to the session is within (0, kTestDuration]
                EXPECT_GT(timeout.count(), 0);
                EXPECT_LE(timeout, kTestDuration);

                // Don't care what the return value is for this test
                return MockInbound::Result::Success();
            }));

    // Start the test
    TestValueType          test_value;
    Ingress<TestValueType> test_ingress {mock_inbound};
    test_ingress.Get(test_value, kTestDuration);
}

/**
 * @brief Test the failout behavior for when an object is fetched from an Egress instance and the Inbound session buffer
 * it is connected to has no bytes available
 *
 */
TEST(Session_Ingress, session_has_no_room)
{
    using TestValueType = int;

    // Stage the mock Inbound session
    MockInbound mock_inbound;
    // Expect session to have no bytes available
    EXPECT_CALL(mock_inbound, InputBytesAvailable()).WillOnce(Return(0U));
    // Expect request to not be made
    EXPECT_CALL(mock_inbound, Request(_, _)).Times(0U);

    // Start the test
    TestValueType          test_value;
    Ingress<TestValueType> test_ingress {mock_inbound};
    ASSERT_TRUE(test_ingress.Get(test_value).IsFailure());
}

/**
 * @brief Test the failout behavior for when an object is fetched from an Ingress instance and the Inbound session
 * buffer it is connected to denies the request for whatever reason
 *
 */
TEST(Session_Ingress, session_request_fails)
{
    using TestValueType = int;

    // Stage the mock Inbound session
    MockInbound mock_inbound;
    // Expect plenty of availablities in the session
    EXPECT_CALL(mock_inbound, InputBytesAvailable()).WillOnce(Return(std::numeric_limits<size_t>::max()));
    // Expect request to be denied
    EXPECT_CALL(mock_inbound, Request(_, _)).WillOnce(Return(MockInbound::Result::Failure()));

    // Start the test
    TestValueType          test_value;
    Ingress<TestValueType> test_ingress {mock_inbound};
    ASSERT_TRUE(test_ingress.Get(test_value).IsFailure());
}
