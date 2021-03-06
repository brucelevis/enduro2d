/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018 Matvey Cherevko
 ******************************************************************************/

#pragma once

#include <enduro2d/high/_high.hpp>

namespace e2d { namespace render_system_impl
{
    struct index_u8 {
        using type = u8;
        static index_declaration decl() noexcept {
            return index_declaration::index_type::unsigned_byte;
        }
    };

    struct index_u16 {
        using type = u16;
        static index_declaration decl() noexcept {
            return index_declaration::index_type::unsigned_short;
        }
    };

    struct index_u32 {
        using type = u32;
        static index_declaration decl() noexcept {
            return index_declaration::index_type::unsigned_int;
        }
    };

    struct vertex_v3f_t2f_c32b {
        struct type {
            v3f v;
            v2f t;
            color32 c;
        };
        static vertex_declaration decl() noexcept {
            return vertex_declaration()
                .add_attribute<v3f>("a_vertex")
                .add_attribute<v2f>("a_st")
                .add_attribute<color32>("a_tint").normalized();
        }
    };
}}
