#include "Core/IO/Session/Transference.hpp"

namespace shmit
{
namespace io
{
namespace session
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Transference constructor definitions             ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  Public      ========================================================================================================

Transference::Transference(Span<uint8_t> data) noexcept : m_result {Result::Code::kPending}, m_data {data}
{
}

Transference::Transference(Span<uint8_t const> data) noexcept : m_result {Result::Code::kPending}, m_data {data}
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Transference method definitions in alphabetical order                ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  Public      ========================================================================================================

Span<uint8_t const> Transference::GetData() const noexcept
{
    return m_data;
}

Transference::Result Transference::GetResult() const noexcept
{
    return m_result;
}

void Transference::SetResult(Result result) noexcept
{
    m_result = result;
}

} // namespace session
} // namespace io
} // namespace shmit
