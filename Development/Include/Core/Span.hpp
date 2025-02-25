#pragma once

#include "Core/Data/Footprint.hpp"
#include "Core/StdTypes.hpp"

#include <iterator>
#include <limits>
#include <type_traits>

namespace shmit
{

template<typename T>
class Span
{
    static_assert(!std::is_pointer<T>::value, "shmit::Span template parameter 'T' may not be a pointer type");
    static_assert(sizeof(T) > 0, "shmit::Span template parameter 'T' must have a nonzero size");

public:
    using element_type = T;
    using value_type   = std::remove_cv_t<element_type>;

    using pointer       = std::add_pointer_t<element_type>;
    using const_pointer = std::add_pointer_t<std::add_const_t<element_type>>;

    using reference       = std::add_lvalue_reference_t<element_type>;
    using const_reference = std::add_lvalue_reference_t<std::add_const_t<element_type>>;

    using size_type              = size_t;
    using difference_type        = std::ptrdiff_t;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr static size_type npos {std::numeric_limits<size_type>::max()};

    Span() = delete;

    constexpr explicit Span(pointer start, size_type count) noexcept;

    constexpr explicit Span(iterator start, iterator end) noexcept;

    template<size_t N>
    constexpr Span(element_type (&arr)[N]) noexcept;

    constexpr Span(Span const& rhs) noexcept = default;

    ~Span() = default;

    constexpr Span& operator=(Span const& rhs) noexcept = default;

    constexpr reference       operator[](size_type i) noexcept;
    constexpr const_reference operator[](size_type i) const noexcept;

    constexpr iterator       begin() const noexcept;
    constexpr const_iterator cbegin() const noexcept;

    constexpr iterator       end() const noexcept;
    constexpr const_iterator cend() const noexcept;

    constexpr reverse_iterator       rbegin() const noexcept;
    constexpr const_reverse_iterator crbegin() const noexcept;

    constexpr reverse_iterator       rend() const noexcept;
    constexpr const_reverse_iterator crend() const noexcept;

    constexpr reference front() const noexcept;
    constexpr reference back() const noexcept;

    constexpr Span subspan(size_type start, size_type count = npos) const noexcept;

    constexpr reference       at(size_type i) noexcept;
    constexpr const_reference at(size_type i) const noexcept;

    constexpr pointer data() const noexcept;

    constexpr size_type count() const noexcept;

    constexpr size_type size() const noexcept;

    template<typename ValueTypeOut, typename ValueTypeIn>
    friend Span<ValueTypeOut> span_cast(Span<ValueTypeIn> const& span);

private:
    pointer   m_data;
    size_type m_count;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Span constructor definitions                ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Public   ////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
constexpr Span<T>::Span(Span<T>::pointer start, Span<T>::size_type count) noexcept : m_data {start}, m_count {count}
{
}

template<typename T>
constexpr Span<T>::Span(Span<T>::iterator start, Span<T>::iterator end) noexcept :
    m_data {start}, m_count {static_cast<size_type>(end - start)}
{
}

template<typename T>
template<size_t N>
constexpr Span<T>::Span(Span<T>::element_type (&arr)[N]) noexcept : m_data {arr}, m_count {N}
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Span method definitions in alphabetical order               ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Public   ////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
constexpr typename Span<T>::reference Span<T>::at(typename Span<T>::size_type i) noexcept
{
    return *(m_data + i);
}

template<typename T>
constexpr typename Span<T>::const_reference Span<T>::at(typename Span<T>::size_type i) const noexcept
{
    return *(m_data + i);
}

template<typename T>
constexpr typename Span<T>::reference Span<T>::back() const noexcept
{
    return *(m_data + m_count - 1U);
}

template<typename T>
constexpr typename Span<T>::iterator Span<T>::begin() const noexcept
{
    return m_data;
}

template<typename T>
constexpr typename Span<T>::const_iterator Span<T>::cbegin() const noexcept
{
    return const_iterator {begin()};
}

template<typename T>
constexpr typename Span<T>::const_iterator Span<T>::cend() const noexcept
{
    return const_iterator {end()};
}

template<typename T>
constexpr typename Span<T>::size_type Span<T>::count() const noexcept
{
    return m_count;
}

template<typename T>
constexpr typename Span<T>::const_reverse_iterator Span<T>::crbegin() const noexcept
{
    return const_reverse_iterator {cbegin()};
}

template<typename T>
constexpr typename Span<T>::const_reverse_iterator Span<T>::crend() const noexcept
{
    return const_reverse_iterator {cend()};
}

template<typename T>
constexpr typename Span<T>::pointer Span<T>::data() const noexcept
{
    return m_data;
}

template<typename T>
constexpr typename Span<T>::iterator Span<T>::end() const noexcept
{
    return m_data + m_count;
}

template<typename T>
constexpr typename Span<T>::reference Span<T>::front() const noexcept
{
    return *m_data;
}

template<typename T>
constexpr typename Span<T>::reverse_iterator Span<T>::rbegin() const noexcept
{
    return reverse_iterator {begin()};
}

template<typename T>
constexpr typename Span<T>::reverse_iterator Span<T>::rend() const noexcept
{
    return reverse_iterator {end()};
}

template<typename T>
constexpr typename Span<T>::size_type Span<T>::size() const noexcept
{
    return data::footprint_size_bytes_v<T> * m_count;
}

template<typename T>
constexpr Span<T> Span<T>::subspan(typename Span<T>::size_type start, typename Span<T>::size_type count) const noexcept
{
    // Reduce count to the amount following the start point if undefined or larger than the size of the span
    if ((count == npos) || ((start + count) > m_count))
        count = (m_count - start);

    return Span {(m_data + start), count};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Span operator definitions               ////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Public   ////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
constexpr typename Span<T>::reference Span<T>::operator[](typename Span<T>::size_type i) noexcept
{
    return at(i);
}

template<typename T>
constexpr typename Span<T>::const_reference Span<T>::operator[](typename Span<T>::size_type i) const noexcept
{
    return at(i);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Namespace function definitions in alphabetical order                ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename ValueTypeOut, typename ValueTypeIn>
Span<ValueTypeOut> span_cast(Span<ValueTypeIn> const& span)
{
    static_assert(std::is_convertible<ValueTypeIn*, ValueTypeOut*>::value, "`ValueTypeOut` and `ValueTypeIn` must be "
                                                                           "convertible");
    return Span<ValueTypeOut> {reinterpret_cast<ValueTypeOut*>(span.m_data), span.m_count};
}

} // namespace shmit