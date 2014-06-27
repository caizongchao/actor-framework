/******************************************************************************\
 *           ___        __                                                    *
 *          /\_ \    __/\ \                                                   *
 *          \//\ \  /\_\ \ \____    ___   _____   _____      __               *
 *            \ \ \ \/\ \ \ '__`\  /'___\/\ '__`\/\ '__`\  /'__`\             *
 *             \_\ \_\ \ \ \ \L\ \/\ \__/\ \ \L\ \ \ \L\ \/\ \L\.\_           *
 *             /\____\\ \_\ \_,__/\ \____\\ \ ,__/\ \ ,__/\ \__/.\_\          *
 *             \/____/ \/_/\/___/  \/____/ \ \ \/  \ \ \/  \/__/\/_/          *
 *                                          \ \_\   \ \_\                     *
 *                                           \/_/    \/_/                     *
 *                                                                            *
 * Copyright (C) 2011 - 2014                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the Boost Software License, Version 1.0. See             *
 * accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt  *
\******************************************************************************/

#include "cppa/message.hpp"
#include "cppa/message_handler.hpp"

#include "cppa/detail/decorated_tuple.hpp"

namespace cppa {

message::message(detail::message_data* ptr) : m_vals(ptr) {}

message::message(message&& other) : m_vals(std::move(other.m_vals)) {}

message::message(const data_ptr& vals) : m_vals(vals) {}

message& message::operator=(message&& other) {
    m_vals.swap(other.m_vals);
    return *this;
}

void message::reset() { m_vals.reset(); }

void* message::mutable_at(size_t p) {
    CPPA_REQUIRE(m_vals);
    return m_vals->mutable_at(p);
}

const void* message::at(size_t p) const {
    CPPA_REQUIRE(m_vals);
    return m_vals->at(p);
}

const uniform_type_info* message::type_at(size_t p) const {
    CPPA_REQUIRE(m_vals);
    return m_vals->type_at(p);
}

bool message::equals(const message& other) const {
    CPPA_REQUIRE(m_vals);
    return m_vals->equals(*other.vals());
}

message message::drop(size_t n) const {
    CPPA_REQUIRE(m_vals);
    if (n == 0) return *this;
    if (n >= size()) return message{};
    return message{detail::decorated_tuple::create(m_vals, n)};
}

message message::drop_right(size_t n) const {
    CPPA_REQUIRE(m_vals);
    if (n == 0) return *this;
    if (n >= size()) return message{};
    std::vector<size_t> mapping(size() - n);
    size_t i = 0;
    std::generate(mapping.begin(), mapping.end(), [&] { return i++; });
    return message{detail::decorated_tuple::create(m_vals, std::move(mapping))};
}

optional<message> message::apply(message_handler handler) {
    return handler(*this);
}

bool message::apply_iterative(message_handler handler) {
    if (empty()) return true;
    for (size_t chunk_size = 1; chunk_size <= size(); ++chunk_size) {
        auto chunk = take(chunk_size);
        auto remainder = drop(chunk_size);
        if (handler(chunk)) {
            return remainder.empty() ? true
                                     : remainder.apply_iterative(std::move(handler));
        }
    }
    return false;
}

} // namespace cppa