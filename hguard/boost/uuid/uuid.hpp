// Boost uuid.hpp header file  ----------------------------------------------//

// Copyright 2006 Andy Tompkins.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Revision History
//  06 Feb 2006 - Initial Revision
//  09 Nov 2006 - fixed variant and version bits for v4 guids
//  13 Nov 2006 - added serialization
//  17 Nov 2006 - added name-based guid creation
//  20 Nov 2006 - add fixes for gcc (from Tim Blechmann)
//  07 Mar 2007 - converted to header only
//  10 May 2007 - removed need for Boost.Thread
//              - added better seed - thanks Peter Dimov
//              - removed null()
//              - replaced byte_count() and output_bytes() with size() and begin() and end()
//  11 May 2007 - fixed guid(ByteInputIterator first, ByteInputIterator last)
//              - optimized operator>>
//  14 May 2007 - converted from guid to uuid
//  29 May 2007 - uses new implementation of sha1
//  01 Jun 2007 - removed using namespace directives
//  09 Nov 2007 - moved implementation to uuid.ipp file
//  12 Nov 2007 - moved serialize code to uuid_serialize.hpp file
//  25 Feb 2008 - moved to namespace boost::uuids

#ifndef BOOST_UUID_HPP
#define BOOST_UUID_HPP

#include <boost/config.hpp>
#include <boost/operators.hpp>
#include <boost/array.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>
#include "seed_rng.hpp"
#include <boost/random/detail/ptr_helper.hpp>
#include <iosfwd>
#include <limits>
#include <sstream>
#include <string>
#include <algorithm>
#include <locale>

#ifdef BOOST_NO_STDC_NAMESPACE
namespace std {
    using ::size_t
} //namespace std
#endif

namespace boost {
namespace uuids {

class uuid : boost::totally_ordered<uuid>
{
    typedef array<uint8_t, 16> data_type;

public:
    typedef data_type::value_type value_type;
    typedef data_type::const_iterator const_iterator;
    typedef data_type::difference_type difference_type;
    typedef data_type::size_type size_type;

public:
    uuid() /* throw() */
    {
        for (data_type::iterator i=data_.begin(); i!=data_.end(); ++i) {
            *i = 0;
        }
    }

    explicit uuid(char const*const str)
    {
        if (str == NULL) throw_invalid_argument("invalid uuid string");

        construct(std::string(str));
    }

#ifndef BOOST_NO_STD_WSTRING
    explicit uuid(wchar_t const*const str)
    {
        if (str == NULL) throw_invalid_argument("invalid uuid string");

        construct(std::wstring(str));
    }
#endif

    template <typename ch, typename char_traits, typename alloc>
        explicit uuid(std::basic_string<ch, char_traits, alloc> const& str)
    {
        construct(str);
    }

    template <typename ByteInputIterator>
        uuid(ByteInputIterator first, ByteInputIterator last)
    {
        data_type::iterator i_data = data_.begin();
        size_type i;
        for (i=0; i<16 && first != last; ++i) {
            *i_data++ = numeric_cast<uint8_t>(*first++);
        }
        if (i != 16) {
            throw_invalid_argument("invalid input iterator pair, must span 16 bytes");
        }
    }

    uuid(uuid const& rhs) /* throw() */
        : data_(rhs.data_)
    {}

    ~uuid() /* throw() */
    {}

    uuid& operator=(uuid const& rhs) /* throw() */
    {
        data_ = rhs.data_;
        return *this;
    }

    bool operator==(uuid const& rhs) const /* throw() */
    {
        return (data_ == rhs.data_);
    }

    bool operator<(uuid const& rhs) const /* throw() */
    {
        return (data_ < rhs.data_);
    }

    bool is_null() const /* throw() */
    {
        for (data_type::const_iterator i=data_.begin(); i!=data_.end(); ++i) {
            if (*i != 0) {
                return false;
            }
        }

        return true;
    }

    std::string to_string() const
    {
        return to_basic_string<std::string::value_type, std::string::traits_type, std::string::allocator_type>();
    }

#ifndef BOOST_NO_STD_WSTRING
    std::wstring to_wstring() const
    {
        return to_basic_string<std::wstring::value_type, std::wstring::traits_type, std::wstring::allocator_type>();
    }
#endif

    template <typename ch, typename char_traits, typename alloc>
        std::basic_string<ch, char_traits, alloc> to_basic_string() const
    {
        std::basic_string<ch, char_traits, alloc> s;
        std::basic_stringstream<ch, char_traits, alloc> ss;
        if (!(ss << *this) || !(ss >> s)) {
            throw_runtime_error("failed to convert uuid to string");
        }
        return s;
    }

    size_type size() const { return data_.size(); } /* throw() */
    const_iterator begin() const { return data_.begin(); } /* throw() */
    const_iterator end() const { return data_.end(); } /* throw() */

    void swap(uuid &rhs) /* throw() */
    {
        std::swap(data_, rhs.data_);
    }

public:
    static uuid create(uuid const& namespace_uuid, char const* name, int name_length)
    {
        return create_name_based(namespace_uuid, name, name_length);
    }

private:
    template <typename ch, typename char_traits, typename alloc>
        void construct(std::basic_string<ch, char_traits, alloc> const& str)
    {
        std::basic_stringstream<ch, char_traits, alloc> ss;
        if (!(ss << str) || !(ss >> *this)) {
            throw_invalid_argument("invalid uuid string");
        }
    }

    void throw_invalid_argument(char const*const message) const;
    void throw_runtime_error(char const*const message) const;

private:
    // name based
    static inline uuid create_name_based(uuid const& namespace_uuid, char const* name, int name_length);

    friend bool get_showbraces(std::ios_base&);
    friend void set_showbraces(std::ios_base&, bool);
    static int get_showbraces_index()
    {
        static int index = std::ios_base::xalloc();
        return index;
    }

private:
    data_type data_;
};

inline void swap(uuid &x, uuid &y)
{
    x.swap(y);
}

inline bool get_showbraces(std::ios_base & iosbase) {
    return (iosbase.iword(uuid::get_showbraces_index()) != 0);
}

inline void set_showbraces(std::ios_base & iosbase, bool showbraces) {
    iosbase.iword(uuid::get_showbraces_index()) = showbraces;
}

inline std::ios_base& showbraces(std::ios_base& iosbase)
{
    set_showbraces(iosbase, true);
    return iosbase;
}
inline std::ios_base& noshowbraces(std::ios_base& iosbase)
{
    set_showbraces(iosbase, false);
    return iosbase;
}

template <typename UniformRandomNumberGenerator>
class basic_uuid_generator
{
private:
    typedef boost::uniform_int<unsigned long> distribution_type;
    typedef random::detail::ptr_helper<UniformRandomNumberGenerator> helper_type;
    
    typedef boost::variate_generator<typename helper_type::reference_type, distribution_type> v_gen_type;

public:
    typedef uuid result_type;

    explicit basic_uuid_generator(UniformRandomNumberGenerator rng)
        : _rng(rng)
        , _v_gen(helper_type::ref(_rng),
                 distribution_type(
                     (std::numeric_limits<unsigned long>::min)(),
                     (std::numeric_limits<unsigned long>::max)())
                 )
    {
        // don't seed generators that are passed in
        //detail::seed<helper_type::value_type>(helper_type::ref(_rng));
    }

    basic_uuid_generator()
        : _v_gen(helper_type::ref(_rng),
                 distribution_type(
                     (std::numeric_limits<unsigned long>::min)(),
                     (std::numeric_limits<unsigned long>::max)())
                 )
    {
        // seed generator with good values
        detail::seed<typename helper_type::value_type>(helper_type::ref(_rng));
    }

    uuid operator()()
    {
        BOOST_STATIC_ASSERT(16 % sizeof(unsigned long) == 0);
        unsigned char data[16];

        for (std::size_t i=0; i<16; i+=sizeof(unsigned long))
        {
            *reinterpret_cast<unsigned long*>(&data[i]) = _v_gen();
        }

        // set variant
        // should be 0b10xxxxxx
        data[8] &= 0xBF;
        data[8] |= 0x80;

        // set version
        // should be 0b0100xxxx
        data[6] &= 0x4F; //0b01001111
        data[6] |= 0x40; //0b01000000

        return uuid(data, data+16);
    }

private:
    // note these must be in this order so that _rng is created before _v_gen
    UniformRandomNumberGenerator _rng;
    v_gen_type _v_gen;
};

typedef basic_uuid_generator<mt19937> uuid_generator;

// This is equivalent to boost::hash_range(u.begin(), u.end());
inline std::size_t hash_value(uuid const& u)
{
    std::size_t seed = 0;
    for(uuid::const_iterator i=u.begin(); i != u.end(); ++i)
    {
        seed ^= static_cast<std::size_t>(*i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    return seed;
}

template <typename ch, typename char_traits>
    std::basic_ostream<ch, char_traits>& operator<<(std::basic_ostream<ch, char_traits> &os, uuid const& u)
{
    io::ios_flags_saver flags_saver(os);
    io::ios_width_saver width_saver(os);
    io::basic_ios_fill_saver<ch, char_traits> fill_saver(os);

    const typename std::basic_ostream<ch, char_traits>::sentry ok(os);
    if (ok) {
        bool showbraces = get_showbraces(os);
        if (showbraces) {
            os << os.widen('{');
        }
        os << std::hex;
        os.fill(os.widen('0'));

        std::size_t i=0;
        for (uuid::const_iterator i_data = u.begin(); i_data!=u.end(); ++i_data, ++i) {
            os.width(2);
            os << static_cast<unsigned int>(*i_data);
            if (i == 3 || i == 5 || i == 7 || i == 9) {
                os << os.widen('-');
            }
        }
        if (showbraces) {
            os << os.widen('}');
        }
    }
    return os;
}

template <typename ch, typename char_traits>
    std::basic_istream<ch, char_traits>& operator>>(std::basic_istream<ch, char_traits> &is, uuid &u)
{
    const typename std::basic_istream<ch, char_traits>::sentry ok(is);
    if (ok) {
        unsigned char data[16];

        typedef std::ctype<ch> ctype_t;
        ctype_t const& ctype = std::use_facet<ctype_t>(is.getloc());

        ch xdigits[16];
        {
            char szdigits[17] = "0123456789ABCDEF";
            ctype.widen(szdigits, szdigits+16, xdigits);
        }
        ch*const xdigits_end = xdigits+16;

        ch c;
        c = is.peek();
        bool bHaveBraces = false;
        if (c == is.widen('{')) {
            bHaveBraces = true;
            is >> c; // read brace
        }

        for (std::size_t i=0; i<u.size() && is; ++i) {
            is >> c;
            c = ctype.toupper(c);

            ch* f = std::find(xdigits, xdigits_end, c);
            if (f == xdigits_end) {
                is.setstate(std::ios_base::failbit);
                break;
            }

            unsigned char byte = static_cast<unsigned char>(std::distance(&xdigits[0], f));

            is >> c;
            c = ctype.toupper(c);
            f = std::find(xdigits, xdigits_end, c);
            if (f == xdigits_end) {
                is.setstate(std::ios_base::failbit);
                break;
            }

            byte <<= 4;
            byte |= static_cast<unsigned char>(std::distance(&xdigits[0], f));

            data[i] = byte;

            if (is) {
                if (i == 3 || i == 5 || i == 7 || i == 9) {
                    is >> c;
                    if (c != is.widen('-')) is.setstate(std::ios_base::failbit);
                }
            }
        }

        if (bHaveBraces && is) {
            is >> c;
            if (c != is.widen('}')) is.setstate(std::ios_base::failbit);
        }
        if (is) {
            u = uuid(data, data+16);
        }
    }
    return is;
}

}} //namespace boost::uuids

#include "uuid.ipp"

#endif // BOOST_UUID_HPP
