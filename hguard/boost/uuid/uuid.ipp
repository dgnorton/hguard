// Boost uuid.ipp header file  ----------------------------------------------//

// Copyright 2007 Andy Tompkins.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Revision History
//  08 Nov 2007 - Initial Revision
//  25 Feb 2008 - Moved to namespace boost::uuids

#include "uuid.hpp"
#include "sha1.hpp"
#include <boost/generator_iterator.hpp>
#include <boost/throw_exception.hpp>
#include <stdexcept>

namespace boost {
namespace uuids {

inline uuid uuid::create_name_based(uuid const& namespace_uuid, char const* name, int name_length)
{
    detail::sha1 sha;
    sha.process_bytes(namespace_uuid.data_.data(), namespace_uuid.data_.static_size);
    sha.process_bytes(name, name_length);
    unsigned int digest[5];

    sha.get_digest(digest);

    unsigned char data[16];
    for (int i=0; i<4; ++i) {
        data[i*4+0] = ((digest[i] >> 24) & 0xFF);
        data[i*4+1] = ((digest[i] >> 16) & 0xFF);
        data[i*4+2] = ((digest[i] >> 8) & 0xFF);
        data[i*4+3] = ((digest[i] >> 0) & 0xFF);
    }

    // set variant
    // should be 0b10xxxxxx
    data[8] &= 0xBF;
    data[8] |= 0x80;

    // set version
    // should be 0b0101xxxx
    data[6] &= 0x5F; //0b01011111
    data[6] |= 0x50; //0b01010000

    return uuid(data, data+16);
}

inline void uuid::throw_invalid_argument(char const*const message) const
{
    boost::throw_exception(std::invalid_argument(message));
}

inline void uuid::throw_runtime_error(char const*const message) const
{
    boost::throw_exception(std::runtime_error(message));
}

}} //namespace boost::uuids
