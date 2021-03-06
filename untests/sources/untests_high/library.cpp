/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018 Matvey Cherevko
 ******************************************************************************/

#include "_high.hpp"
using namespace e2d;

namespace
{
    class safe_starter_initializer final : private noncopyable {
    public:
        safe_starter_initializer() {
            modules::initialize<starter>(0, nullptr,
                starter::parameters(
                    engine::parameters("library_untests", "enduro2d")
                        .without_graphics(true)));
        }

        ~safe_starter_initializer() noexcept {
            modules::shutdown<starter>();
        }
    };
}

TEST_CASE("library"){
    safe_starter_initializer initializer;
    library& l = the<library>();
    {
        auto text_res = l.load_asset<text_asset>("text_asset.txt");
        REQUIRE(text_res);
        REQUIRE(text_res->content() == "hello");

        auto text_res_from_cache = l.load_asset<text_asset>("text_asset.txt");
        REQUIRE(text_res_from_cache);
        REQUIRE(text_res_from_cache.get() == text_res.get());

        REQUIRE(0u == the<asset_cache<text_asset>>().unload_self_unused_assets());
        REQUIRE(the<asset_cache<text_asset>>().asset_count() == 1);

        text_res.reset();
        text_res_from_cache.reset();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        REQUIRE(1u == the<asset_cache<text_asset>>().unload_self_unused_assets());
        REQUIRE(the<asset_cache<text_asset>>().asset_count() == 0);
    }
    {
        auto text_res = l.load_asset<text_asset>("text_asset.txt");
        REQUIRE(text_res);
        REQUIRE(text_res->content() == "hello");

        auto binary_res = l.load_asset<binary_asset>("binary_asset.bin");
        REQUIRE(binary_res);
        REQUIRE(binary_res->content() == buffer("world", 5));

        REQUIRE(0u == l.unload_unused_assets());

        text_res.reset();
        binary_res.reset();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        REQUIRE(2u == l.unload_unused_assets());
    }
    {
        auto empty_res = l.load_asset<binary_asset>("empty_asset");
        REQUIRE_FALSE(empty_res);
    }
    {
        auto image_res = l.load_asset<image_asset>("image.png");
        REQUIRE(image_res);
        REQUIRE(!image_res->content().empty());

        REQUIRE(the<asset_cache<image_asset>>().find("image.png"));
        REQUIRE(the<asset_cache<binary_asset>>().find("image.png"));

        the<deferrer>().worker().wait_all();
        the<asset_cache<binary_asset>>().unload_self_unused_assets();
        REQUIRE(the<asset_cache<image_asset>>().find("image.png"));
        REQUIRE_FALSE(the<asset_cache<binary_asset>>().find("image.png"));

        image_res.reset();
        the<deferrer>().worker().wait_all();
        the<asset_cache<image_asset>>().unload_self_unused_assets();
        REQUIRE_FALSE(the<asset_cache<image_asset>>().find("image.png"));
        REQUIRE_FALSE(the<asset_cache<binary_asset>>().find("image.png"));
    }
    {
        if ( modules::is_initialized<render>() ) {
            auto shader_res = l.load_asset<shader_asset>("shader.json");
            REQUIRE(shader_res);
            REQUIRE(shader_res->content());

            auto texture_res = l.load_asset<texture_asset>("image.png");
            REQUIRE(texture_res);
            REQUIRE(texture_res->content());

            auto sprite_res = l.load_asset<sprite_asset>("sprite.json");
            REQUIRE(sprite_res);
            REQUIRE(sprite_res->content().size() == v2f(10.f, 20.f));
            REQUIRE(sprite_res->content().pivot() == v2f(0.5f, 0.7f));
            REQUIRE(sprite_res->content().texrect() == b2f(0.5f, 1.f));
            REQUIRE(sprite_res->content().texture());
            REQUIRE(sprite_res->content().material());

            auto model_res = l.load_asset<model_asset>("model.json");
            REQUIRE(model_res);
            REQUIRE(model_res->content().mesh());
            REQUIRE(model_res->content().material_count() == 1);
            REQUIRE(model_res->content().material(0));
            REQUIRE_FALSE(model_res->content().mesh()->content().vertices().empty());
            REQUIRE(model_res->content().mesh()->content().indices_submesh_count() == 1);
            REQUIRE_FALSE(model_res->content().mesh()->content().indices(0).empty());
        }
    }
    {
        if ( modules::is_initialized<render>() ) {
            auto material_res = l.load_asset<material_asset>("material.json");
            REQUIRE(material_res);
            {
                auto texture_res = l.load_asset<texture_asset>("image.png");
                REQUIRE(texture_res);
                REQUIRE(texture_res->content());

                const auto* sampler = material_res->content().properties().sampler("s");
                REQUIRE(sampler);
                REQUIRE(sampler->s_wrap() == render::sampler_wrap::clamp);
                REQUIRE(sampler->t_wrap() == render::sampler_wrap::repeat);
                REQUIRE(sampler->r_wrap() == render::sampler_wrap::mirror);
                REQUIRE(sampler->min_filter() == render::sampler_min_filter::linear_mipmap_linear);
                REQUIRE(sampler->mag_filter() == render::sampler_mag_filter::linear);
                REQUIRE(texture_res->content() == sampler->texture());
            }
            {
                const auto* property = material_res->content().properties().property("i");
                REQUIRE(property);
                REQUIRE(property->index() == 0);
                REQUIRE(stdex::get<i32>(*property) == 42);
            }
            REQUIRE(material_res->content().pass_count() == 1);
            const auto& pass = material_res->content().pass(0);
            {
                const auto* property = pass.properties().property("f");
                REQUIRE(property);
                REQUIRE(property->index() == 1);
                REQUIRE(math::approximately(stdex::get<f32>(*property), 4.2f));
            }
            {
                const auto* property = pass.properties().property("v1");
                REQUIRE(property);
                REQUIRE(property->index() == 2);
                REQUIRE(stdex::get<v2i>(*property) == v2i(1,2));
            }
            {
                const auto* property = pass.properties().property("v2");
                REQUIRE(property);
                REQUIRE(property->index() == 6);
                REQUIRE(stdex::get<v3f>(*property) == v3f(3.f));
            }
            {
                const auto* property = pass.properties().property("v3");
                REQUIRE(property);
                REQUIRE(property->index() == 4);
                REQUIRE(stdex::get<v4i>(*property) == v4i(1,2,3,4));
            }
            {
                const auto* property = pass.properties().property("m1");
                REQUIRE(property);
                REQUIRE(property->index() == 8);
                REQUIRE(stdex::get<m2f>(*property) == m2f(1,2,3,4));

                const auto* property2 = pass.properties().property("m4");
                REQUIRE(property2);
                REQUIRE(*property2 == *property);
            }
            {
                const auto* property = pass.properties().property("m2");
                REQUIRE(property);
                REQUIRE(property->index() == 9);
                REQUIRE(stdex::get<m3f>(*property) == m3f(1,2,3,4,5,6,7,8,9));

                const auto* property2 = pass.properties().property("m5");
                REQUIRE(property2);
                REQUIRE(*property2 == *property);
            }
            {
                const auto* property = pass.properties().property("m3");
                REQUIRE(property);
                REQUIRE(property->index() == 10);
                REQUIRE(stdex::get<m4f>(*property) == m4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16));

                const auto* property2 = pass.properties().property("m6");
                REQUIRE(property2);
                REQUIRE(*property2 == *property);
            }
            {
                REQUIRE(pass.states().depth().range_near() == 1.f);
                REQUIRE(pass.states().depth().range_far() == 2.f);
                REQUIRE(pass.states().depth().write() == false);
                REQUIRE(pass.states().depth().func() == render::compare_func::greater);

                REQUIRE(pass.states().stencil().write() == 2u);
                REQUIRE(pass.states().stencil().func() == render::compare_func::never);
                REQUIRE(pass.states().stencil().ref() == 4u);
                REQUIRE(pass.states().stencil().mask() == 5u);
                REQUIRE(pass.states().stencil().pass() == render::stencil_op::incr);
                REQUIRE(pass.states().stencil().sfail() == render::stencil_op::decr);
                REQUIRE(pass.states().stencil().zfail() == render::stencil_op::invert);

                REQUIRE(pass.states().culling().mode() == render::culling_mode::cw);
                REQUIRE(pass.states().culling().face() == render::culling_face::front);

                REQUIRE(pass.states().capabilities().culling());
                REQUIRE(pass.states().capabilities().blending());
                REQUIRE(pass.states().capabilities().depth_test());
                REQUIRE(pass.states().capabilities().stencil_test());

                REQUIRE(pass.states().blending().constant_color() == color::white());
                REQUIRE(pass.states().blending().color_mask() == render::blending_color_mask::gba);

                REQUIRE(pass.states().blending().src_rgb_factor() == render::blending_factor::dst_alpha);
                REQUIRE(pass.states().blending().src_alpha_factor() == render::blending_factor::dst_alpha);

                REQUIRE(pass.states().blending().dst_rgb_factor() == render::blending_factor::dst_color);
                REQUIRE(pass.states().blending().dst_alpha_factor() == render::blending_factor::src_color);

                REQUIRE(pass.states().blending().rgb_equation() == render::blending_equation::subtract);
                REQUIRE(pass.states().blending().alpha_equation() == render::blending_equation::reverse_subtract);
            }
        }
    }
}
