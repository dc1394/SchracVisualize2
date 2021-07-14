/*! \file myrandsfmt.h
    \brief SFMTを使った自作乱数クラスの実装

    Copyright © 2017-2021 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#include "myrandsfmt.h"

namespace myrandom {
    double MyRandSfmt::normal_distribution_rand()
    {
        using namespace boost::math::constants;

        if (normal_dist_rand_.has_value()) {
            auto const res = *normal_dist_rand_;
            normal_dist_rand_ = std::nullopt;

            return res;
        }
        else {
            auto const r1 = myrand();
            auto const r2 = myrand();

            auto const res = std::sqrt(-2.0 * std::log(r1)) * std::cos(2.0 * pi<double>() * r2);
            normal_dist_rand_ = std::make_optional(std::sqrt(-2.0 * std::log(r1)) * std::sin(2.0 * pi<double>() * r2));

            return res;
        }
    }

    double MyRandSfmt::normal_distribution_rand(double mu, double sigma2)
    {
        using namespace boost::math::constants;

        auto const sigma = std::sqrt(sigma2);
        if (normal_dist_rand_.has_value()) {
            auto const res = *normal_dist_rand_;
            normal_dist_rand_ = std::nullopt;

            return res * sigma + mu;
        }
        else {
            auto const r1 = myrand();
            auto const r2 = myrand();

            auto const res = std::sqrt(-2.0 * std::log(r1)) * std::cos(2.0 * pi<double>() * r2) * sigma + mu;
            normal_dist_rand_ = std::make_optional(std::sqrt(-2.0 * std::log(r1)) * std::sin(2.0 * pi<double>() * r2));
            
            return res;
        }
    }
}
