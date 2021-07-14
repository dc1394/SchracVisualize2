/*! \file orbitaldensityrand.cpp
    \brief OrbitalDensityRandクラスの実装

    Copyright © 2019 @dc1394 All Rights Reserved.
    (but this is originally adapted by サンマヤ for TDXHydrogenScene.cpp from http://sammaya.garyoutensei.com/math_phys/phys_sym/monte_hydrogen.html )
    This software is released under the BSD 2-Clause License.
*/

#include "orbitaldensityrand.h"
#include "utility/utility.h"
#include <boost/assert.hpp>                                     // for boost::assert
#include <boost/math/special_functions/spherical_harmonic.hpp>  // for boost::math::spherical_harmonic
#include <boost/range/algorithm.hpp>                            // for boost::fill

namespace orbitaldensityrand {
	OrbitalDensityRand::OrbitalDensityRand(std::shared_ptr<getdata::GetData> const & pgd)
        :   Complete([this] { return complete_.load(); }, nullptr),
            Pth([this] { return std::cref(pth_); }, nullptr),
		    Redraw(nullptr, [this](auto redraw) { return redraw_ = redraw; }),
            Rmax([this] { return rmax_; }, nullptr),
		    Thread_end(nullptr, [this](auto thread_end) { 
			    thread_end_.store(thread_end);
			    return thread_end; }),
            Vertex([this] { return std::cref(vertex_); }, nullptr),
		    Vertexsize([this]{ return vertexsize_.load(); }, [this](std::vector<SimpleVertex>::size_type size) { 
				vertexsize_.store(size);
				return size; }),
            pgd_(pgd),
            q_({ 1.0, 0.0, 0.0 }),
		    rmax_(GetRmax(pgd)),
		    vertex_(VERTEXSIZE_INIT_VALUE)
    {
    }

    void OrbitalDensityRand::operator()(std::int32_t m, Normal_Nelson_type nornel, Re_Im_type reim)
    {
        if (redraw_) {
            if (vertex_.size() != vertexsize_) {
                vertex_.resize(vertexsize_);
            }

            pth_.reset(new std::thread([this, m, nornel, reim] { ClearFillSimpleVertex(m, nornel, reim); }), [this](std::thread * pth)
            {
                if (pth->joinable()) {
                    thread_end_.store(true);
                    pth->join();
                }

                utility::Safe_Delete<std::thread> sd;
                sd(pth);
            });
            redraw_ = false;
        }
    }

	void OrbitalDensityRand::ClearFillSimpleVertex(std::int32_t m, Normal_Nelson_type nornel, Re_Im_type reim)
	{
		complete_.store(false);

		SimpleVertex sv{};
		sv.Color = { 0.0f, 0.0f, 0.0f, 0.0f };
		sv.Pos = { 0.0f, 0.0f, 0.0f };
		boost::fill(vertex_, sv);

        switch (nornel)
        {
        case Normal_Nelson_type::NORMAL:
            {
                auto const threads = static_cast<std::int32_t>(std::thread::hardware_concurrency());
                auto const num = static_cast<std::int32_t>(vertexsize_.load() / threads);

                auto thvec = std::vector<std::thread>(threads);
                for (auto i = 0; i < threads - 1; i++) {
                    thvec[i] = std::thread([i, m, num, reim, this]() { FillSimpleVertex(m, reim, num * i, num * (i + 1)); });
                }
                thvec[threads - 1] = std::thread([m, num, reim, threads, this]() { FillSimpleVertex(m, reim, num * (threads - 1), static_cast<std::int32_t>(vertexsize_.load())); });

                for (auto && th : thvec) {
                    th.join();
                }
            }
            break;

        case Normal_Nelson_type::NELSON:
            FillSimpleVertex(m);
            break;

        default:
            BOOST_ASSERT(!"nornelが異常!");
            break;
        }
        
		complete_.store(true);
	}

    void OrbitalDensityRand::FillSimpleVertex(std::int32_t m)
    {
        using namespace boost::math;
        using namespace constants;

        for (auto i = 0UL; i < vertexsize_.load(); i++) {
            if (thread_end_) {
                return;
            }

            auto const r = std::hypot(q_[0], q_[1], q_[2]);

            auto const theta1 = std::acos(q_[2] / r);
            auto const theta2 = std::atan(std::hypot(q_[0], q_[1]) / q_[2]);
            auto const theta = (theta1 < 0.0 && theta2 < 0.0) ? 2.0 * pi<double>() - theta1 : theta1;

            auto phi = std::atan(q_[1] / q_[0]);

            auto const t1 = std::sin(theta) * std::cos(phi);
            if (q_[0] * t1 < 0.0)
            {
                phi = pi<double>() - phi;
            }

            auto const ylm = spherical_harmonic_r(pgd_->L, m, theta, phi);
            auto const dylmdtheta = Numerical_diff(
                theta,
                myfunctional::make_functional([this, m, phi](double th) { return spherical_harmonic_r(pgd_->L, m, th, phi); }));
            auto const dylmdphi = Numerical_diff(
                phi,
                myfunctional::make_functional([this, m, theta](double ph) { return spherical_harmonic_r(pgd_->L, m, theta, ph); }));

            auto f_x = std::sin(theta) * std::cos(phi) * pgd_->dphidr(r) / (*pgd_)(r);
            f_x += std::cos(theta) * std::cos(phi) / r * dylmdtheta / ylm;
            f_x -= std::sin(phi) / (r * std::sin(theta)) * dylmdphi / ylm;

            auto const t2 = std::sin(theta) * std::sin(phi);
            if (q_[1] * t2 < 0.0)
            {
                phi = pi<double>() + phi;
            }

            auto f_y = std::sin(theta) * std::sin(phi) * pgd_->dphidr(r) / (*pgd_)(r);
            f_y += std::cos(theta) * std::sin(phi) / r * dylmdtheta / ylm;
            f_y += std::cos(phi) / (r * std::sin(theta)) * dylmdphi / ylm;

            auto f_z = std::cos(theta) * pgd_->dphidr(r) / (*pgd_)(r);
            f_z -= std::sin(theta) / r * dylmdtheta / ylm;

            q_[0] += f_x * DT + mr_.normal_distribution_rand() * std::sqrt(DT);
            q_[1] += f_y * DT + mr_.normal_distribution_rand() * std::sqrt(DT);
            q_[2] += f_z * DT + mr_.normal_distribution_rand() * std::sqrt(DT);

            vertex_[i].Pos.x = static_cast<float>(q_[0]);
            vertex_[i].Pos.y = static_cast<float>(q_[1]);
            vertex_[i].Pos.z = static_cast<float>(q_[2]);

            vertex_[i].Color.x = 0.8f;
            vertex_[i].Color.y = 0.0f;
            vertex_[i].Color.z = 0.8f;
            vertex_[i].Color.w = 1.0f;
        }
    }

	void OrbitalDensityRand::FillSimpleVertex(std::int32_t m, Re_Im_type reim, std::int32_t starti, std::int32_t endi)
	{
		if (thread_end_) {
			return;
		}

        if (!m && pgd_->Rho_wf_type_ == getdata::GetData::Rho_Wf_type::WF && reim == Re_Im_type::IMAGINARY) {
            return;
        }

		auto sign = 1;
		auto x = 0.0;
        auto y = 0.0;
        auto z = 0.0;

        myrandom::MyRandSfmt mr;
                
        auto nextflag = false;
        auto cnt = 0;

        do {
            if (thread_end_) {
                return;
            }

            switch (pgd_->Rho_wf_type_) {
            case getdata::GetData::Rho_Wf_type::RHO:
            {
                auto rho = [this, m, nextflag](auto x, auto y, auto z) mutable
                {
                    auto const r = std::sqrt(x * x + y * y + z * z);
                    if (r < pgd_->R_meshmin()) {
                        nextflag = true;
                        return 0.0;
                    }
                    auto const phi = std::acos(x / std::sqrt(x * x + y * y));
                    double v;
                    if (m >= 0) {
                        v = boost::math::spherical_harmonic_r(pgd_->L, m, std::acos(z / r), phi);
                    }
                    else {
                        v = boost::math::spherical_harmonic_i(pgd_->L, m, std::acos(z / r), phi);
                    }

                    return ((*pgd_)(r) * v * v);
                };
                
                auto const maxr = pgd_->R2rhomaxr();

                auto const x_star = mr.normal_distribution_rand(x, maxr);
                auto const y_star = mr.normal_distribution_rand(y, maxr);
                auto const z_star = mr.normal_distribution_rand(z, maxr);

                auto const rho_t = rho(x, y, z);    // xの電子密度
                if (nextflag) {
                    x = x_star;
                    y = y_star;
                    z = z_star;
                    nextflag = false;
                    continue;
                }

                auto const rho_star = rho(x_star, y_star, z_star);  // x* の電子密度

                // 採択率を計算  α = p(x*) / p(x_t)
                auto const alpha = rho_star / rho_t;

                //採択率により決定
                auto const ar = mr.myrand();    // 0 <= ar <= 1 の一様乱数 ar を生成
                if (ar <= alpha) {              // 採択
                    x = x_star;
                    y = y_star;
                    z = z_star;
                }
                else {                          // 採択しない
                    continue;
                }                
            }
            break;

            case getdata::GetData::Rho_Wf_type::WF:
            {
                auto phi = [this, m, nextflag, reim](auto x, auto y, auto z) mutable
                {
                    auto const r = std::sqrt(x * x + y * y + z * z);
                    if (r < pgd_->R_meshmin()) {
                        nextflag = true;
                        return 0.0;
                    }
                    auto const phi = std::acos(x / std::sqrt(x * x + y * y));
                    double ylm = 0.0;
                    switch (reim) {
                    case Re_Im_type::REAL:
                        ylm = boost::math::spherical_harmonic_r(pgd_->L, m, std::acos(z / r), phi);
                        break;

                    case Re_Im_type::IMAGINARY:
                        ylm = boost::math::spherical_harmonic_i(pgd_->L, m, std::acos(z / r), phi);
                        break;

                    default:
                        BOOST_ASSERT(!"何かがおかしい!");
                        break;
                    }

                    return (*pgd_)(r) * ylm;
                };

                auto const maxr = pgd_->R2rhomaxr();

                auto const x_star = mr.normal_distribution_rand(x, maxr);
                auto const y_star = mr.normal_distribution_rand(y, maxr);
                auto const z_star = mr.normal_distribution_rand(z, maxr);

                auto const phi_t = phi(x, y, z);    // xの波動関数
                if (nextflag) {
                    x = x_star;
                    y = y_star;
                    z = z_star;
                    nextflag = false;
                    continue;
                }

                auto const phi_star = phi(x_star, y_star, z_star);  // x* の波動関数

                // 採択率を計算  α = p(x*) / p(x_t)
                auto const alpha = (phi_star * phi_star) / (phi_t * phi_t);

                //採択率により決定
                auto const ar = mr.myrand();    // 0 <= ar <= 1 の一様乱数 ar を生成
                if (ar <= alpha) {              // 採択
                    x = x_star;
                    y = y_star;
                    z = z_star;
                    if (phi_star >= 0.0) {
                        sign = 1;
                    }
                    else {
                        sign = -1;
                    }
                }
                else {                          // 採択しない
                    continue;
                }
            }
            break;

            default:
                BOOST_ASSERT(!"何かがおかしい!");
                break;
            }

            vertex_[starti + cnt].Pos.x = static_cast<float>(x);
            vertex_[starti + cnt].Pos.y = static_cast<float>(y);
            vertex_[starti + cnt].Pos.z = static_cast<float>(z);

            vertex_[starti + cnt].Color.x = sign > 0 ? 0.8f : 0.0f;
            vertex_[starti + cnt].Color.y = sign < 0 ? 0.8f : 0.0f;
            vertex_[starti + cnt].Color.z = 0.8f;
            vertex_[starti + cnt].Color.w = 1.0f;
            cnt++;
		} while (cnt < (endi - starti));
	}

	double GetRmax(std::shared_ptr<getdata::GetData> const & pgd)
	{
		auto const n = static_cast<double>(pgd->N);
		return (2.3622 * n + 3.3340) * n + 1.3228;
	}
}
