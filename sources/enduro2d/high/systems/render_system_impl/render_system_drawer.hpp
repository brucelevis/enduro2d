/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018 Matvey Cherevko
 ******************************************************************************/

#pragma once

#include <enduro2d/high/_high.hpp>

#include <enduro2d/high/node.hpp>
#include <enduro2d/high/components/camera.hpp>

#include "render_system_base.hpp"
#include "render_system_batcher.hpp"

namespace e2d { namespace render_system_impl
{
    class bad_drawer_operation final : public exception {
    public:
        const char* what() const noexcept final {
            return "bad drawer operation";
        }
    };

    class drawer : private noncopyable {
    public:
        using batcher_type = batcher<
            index_u16,
            vertex_v3f_t2f_c32b>;

        class context : noncopyable {
        public:
            context(
                const camera& cam,
                const const_node_iptr& cam_n,
                engine& engine,
                render& render,
                batcher_type& batcher);

            void draw(
                const const_node_iptr& node);

            void draw(
                const const_node_iptr& node,
                const renderer& node_r,
                const model_renderer& mdl_r);

            void draw(
                const const_node_iptr& node,
                const renderer& node_r,
                const sprite_renderer& spr_r);

            void flush();
        private:
            render& render_;
            batcher_type& batcher_;
            render::property_block property_cache_;
            render::property_block internal_properties_;
        };
    public:
        drawer(engine& e, debug& d, render& r);

        template < typename F >
        void with(const camera& cam, const const_node_iptr& cam_n, F&& f);
    private:
        engine& engine_;
        render& render_;
        batcher_type batcher_;
    };
}}

namespace e2d { namespace render_system_impl
{
    template < typename F >
    void drawer::with(const camera& cam, const const_node_iptr& cam_n, F&& f) {
        context ctx{cam, cam_n, engine_, render_, batcher_};
        std::forward<F>(f)(ctx);
        ctx.flush();
    }
}}
