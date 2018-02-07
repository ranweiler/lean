/*
Copyright (c) 2018 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.
Author: Leonardo de Moura
*/
#include <limits>
#include <algorithm>
#include "util/sstream.h"
#include "util/name_generator.h"
#include "util/name_set.h"

namespace lean {
static name * g_tmp_prefix = nullptr;
name_generator::name_generator():name_generator(*g_tmp_prefix) {}

name name_generator::next() {
    if (m_next_idx == std::numeric_limits<unsigned>::max()) {
        // avoid overflow
        m_prefix   = name(m_prefix, m_next_idx);
        m_next_idx = 0;
    }
    name r(m_prefix, m_next_idx);
    m_next_idx++;
    return r;
}

void swap(name_generator & a, name_generator & b) {
    swap(a.m_prefix, b.m_prefix);
    std::swap(a.m_next_idx, b.m_next_idx);
}

static name_set * g_ngen_prefixes = nullptr;

void register_name_generator_prefix(name const & n) {
    lean_assert(!g_ngen_prefixes->contains(n));
    g_ngen_prefixes->insert(n);
}

bool uses_name_generator_prefix(name const & n) {
    if (n.is_anonymous())
        return false;
    else if (g_ngen_prefixes->contains(n))
        return true;
    else
        return uses_name_generator_prefix(n.get_prefix());
}

static void sanitize_name_generator_name(sstream & strm, name const & n) {
    if (n.is_anonymous()) {
        return;
    } else if (n.is_numeral()) {
        sanitize_name_generator_name(strm, n.get_prefix());
        strm << "_" << n.get_numeral();
    } else {
        lean_assert(n.is_string());
        sanitize_name_generator_name(strm, n.get_prefix());
        strm << "_" << n.get_string();
    }
}

name sanitize_name_generator_name(name const & n) {
    if (!uses_name_generator_prefix(n))
        return n;
    sstream strm;
    sanitize_name_generator_name(strm, n);
    return name(strm.str());
}

void initialize_name_generator() {
    g_ngen_prefixes = new name_set();
    g_tmp_prefix    = new name("_uniq");
    register_name_generator_prefix(*g_tmp_prefix);
}

void finalize_name_generator() {
    delete g_tmp_prefix;
    delete g_ngen_prefixes;
}
}
