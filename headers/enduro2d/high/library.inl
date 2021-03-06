/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018 Matvey Cherevko
 ******************************************************************************/

#ifndef E2D_INCLUDE_GUARD_C9B4A08B2E2A4A659AB390BDC6EBEFE8
#define E2D_INCLUDE_GUARD_C9B4A08B2E2A4A659AB390BDC6EBEFE8
#pragma once

#include "_high.hpp"
#include "library.hpp"

namespace e2d
{
    //
    // library
    //

    template < typename Asset >
    typename Asset::load_result library::load_asset(str_view address) {
        auto p = load_asset_async<Asset>(address);

        if ( modules::is_initialized<deferrer>() ) {
            the<deferrer>().active_safe_wait_promise(p);
        }

        return p.get_or_default(nullptr);
    }

    template < typename Asset >
    typename Asset::load_async_result library::load_asset_async(str_view address) {
        if ( !modules::is_initialized<asset_cache<Asset>>() ) {
            return Asset::load_async(*this, address);
        }

        auto& cache = the<asset_cache<Asset>>();
        if ( auto cached_asset = cache.find(address) ) {
            return stdex::make_resolved_promise(std::move(cached_asset));
        }

        return Asset::load_async(*this, address)
            .then([
                &cache,
                address_hash = make_hash(address)
            ](auto&& new_asset){
                cache.store(address_hash, new_asset);
                return new_asset;
            });
    }

    //
    // asset_cache
    //

    template < typename T >
    asset_cache<T>::asset_cache(library& l)
    : library_(l) {}

    template < typename T >
    asset_cache<T>::~asset_cache() noexcept = default;

    template < typename T >
    typename asset_cache<T>::asset_result asset_cache<T>::find(str_hash address) const {
        std::lock_guard<std::mutex> guard(mutex_);
        const auto iter = assets_.find(address);
        return iter != assets_.end()
            ? iter->second
            : nullptr;
    }

    template < typename T >
    void asset_cache<T>::store(str_hash address, const asset_result& asset) {
        std::lock_guard<std::mutex> guard(mutex_);
        assets_[address] = asset;
    }

    template < typename T >
    void asset_cache<T>::clear() noexcept {
        std::lock_guard<std::mutex> guard(mutex_);
        assets_.clear();
    }

    template < typename T >
    std::size_t asset_cache<T>::asset_count() const noexcept {
        std::lock_guard<std::mutex> guard(mutex_);
        return assets_.size();
    }

    template < typename T >
    std::size_t asset_cache<T>::unload_self_unused_assets() noexcept {
        std::lock_guard<std::mutex> guard(mutex_);
        std::size_t result = 0u;
        for ( auto iter = assets_.begin(); iter != assets_.end(); ) {
            if ( !iter->second || 1 == iter->second->use_count() ) {
                iter = assets_.erase(iter);
                ++result;
            } else {
                ++iter;
            }
        }
        return result;
    }
}

#endif
