#include "Core/Mocks/IO/Session/MockOutbound.hpp"

#include <Core/Data/Footprint.hpp>
#include <Core/IO/Session/Egress.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <limits>

using namespace shmit;
using namespace shmit::io::session;

using ::testing::Invoke;
using ::testing::Return;
using ::testing::_;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Egress tests                ////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Test the nominal success path for when an object is put to an Egress instance
 *
 */
TEST(Session_Egress, nominal_success)
{
    using TestValueType = int;

    constexpr TestValueType kTestValue {42};

    // Stage mock Outbound session
    MockOutbound mock_outbound;
    // Expect plenty of availablities in the session
    EXPECT_CALL(mock_outbound, OutputBytesAvailable()).WillOnce(Return(std::numeric_limits<size_t>::max()));
    // Expect posting of the transmission to return success
    EXPECT_CALL(mock_outbound, Post(_, _))
        .WillOnce(Invoke(
            [&](Span<uint8_t const> tx, std::chrono::microseconds timeout) -> MockOutbound::Result
            {
                static_cast<void>(timeout); // Avoid unused parameter

                // Verify that the span contains the test value
                EXPECT_EQ(shmit::data::footprint_size_bytes_v<TestValueType>, tx.size());
                auto value {*(reinterpret_cast<TestValueType const*>(tx.data()))};
                EXPECT_EQ(kTestValue, value);

                return MockOutbound::Result::Success();
            }));

    // Start the test
    Egress<TestValueType> test_egress {mock_outbound};
    ASSERT_TRUE(test_egress.Put(kTestValue).IsSuccess());
}

/**
 * @brief Test that a zero-valued duration is passed through Egress to the connected Outbound session buffer while
 * putting to an Egress
 *
 */
TEST(Session_Egress, no_timeout)
{
    using TestValueType = int;

    constexpr TestValueType kTestValue {42};

    // Stage mock Outbound session
    MockOutbound mock_outbound;
    // Expect plenty of availablities in the session
    EXPECT_CALL(mock_outbound, OutputBytesAvailable()).WillOnce(Return(std::numeric_limits<size_t>::max()));
    // Expect posting of the transmission to the session with zero duration
    // Don't care about return value for this test
    EXPECT_CALL(mock_outbound, Post(_, std::chrono::microseconds::zero())).WillOnce(Return(MockOutbound::Result::Success()));

    // Start the test
    Egress<TestValueType> test_egress {mock_outbound};
    test_egress.Put(kTestValue);
}

/**
 * @brief Test that a nonzero duration is passed through Egress to the connected Outbound session buffer while putting
 * to an Egress
 *
 */
TEST(Session_Egress, nonzero_timeout)
{
    using TestValueType = int;

    constexpr TestValueType             kTestValue {42};
    constexpr std::chrono::microseconds kTestDuration {500};

    // Stage mock Outbound session
    MockOutbound mock_outbound;
    // Expect plenty of availablities in the session
    EXPECT_CALL(mock_outbound, OutputBytesAvailable()).WillOnce(Return(std::numeric_limits<size_t>::max()));
    // Expect posting of the transmission with nonzero duration
    EXPECT_CALL(mock_outbound, Post(_, _))
        .WillOnce(Invoke(
            [&](Span<uint8_t const> tx, std::chrono::microseconds timeout) -> MockOutbound::Result
            {
                // Verify that the timeout supplied to the session is within (0, kTestDuration]
                EXPECT_GT(timeout.count(), 0);
                EXPECT_LE(timeout, kTestDuration);

                // Don't care what the return value is for this test
                return MockOutbound::Result::Success();
            }));

    // Start the test
    Egress<TestValueType> test_egress {mock_outbound};
    test_egress.Put(kTestValue, kTestDuration);
}

/**
 * @brief Test the failout behavior for when an object is put to an Egress instance and the OUtbound session buffer it
 * is connected to has no bytes available
 *
 */
TEST(Session_Egress, session_has_no_room)
{
    // Stage mock Outbound session
    MockOutbound mock_outbound;
    // Expect the session to have no available space
    EXPECT_CALL(mock_outbound, OutputBytesAvailable()).WillOnce(Return(0U));
    // Expect session to not be posted to
    EXPECT_CALL(mock_outbound, Post(_, _)).Times(0U);

    // Start the test
    int         test_value {42};
    Egress<int> test_egress {mock_outbound};
    ASSERT_TRUE(test_egress.Put(test_value).IsFailure());
}

/**
 * @brief Test the failout behavior for when an object is put to an Egress instance and the Outbound session buffer it
 * is connected to denies the posting for whatever reason
 *
 */
TEST(Session_Egress, session_posting_fails)
{
    // Stage mock Outbound session
    MockOutbound mock_outbound;
    // Expect plenty of availablities in the session
    EXPECT_CALL(mock_outbound, OutputBytesAvailable()).WillOnce(Return(std::numeric_limits<size_t>::max()));
    // Expect session posting to fail
    EXPECT_CALL(mock_outbound, Post(_, _)).WillOnce(Return(MockOutbound::Result::Failure()));

    // Start the test
    int         test_value {42};
    Egress<int> test_egress {mock_outbound};
    ASSERT_TRUE(test_egress.Put(test_value).IsFailure());
}
