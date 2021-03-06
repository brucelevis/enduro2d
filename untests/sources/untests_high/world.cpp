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
                    engine::parameters("world_untests", "enduro2d")
                        .without_graphics(true)));
        }

        ~safe_starter_initializer() noexcept {
            modules::shutdown<starter>();
        }
    };
}

TEST_CASE("world") {
    safe_starter_initializer initializer;
    world& w = the<world>();
    const world& cw = w;

    SECTION("registry") {
        auto e = w.registry().create_entity();
        REQUIRE(cw.registry().alive_entity(e));
        REQUIRE(w.registry().destroy_entity(e));
        REQUIRE_FALSE(cw.registry().alive_entity(e));
    }
}
