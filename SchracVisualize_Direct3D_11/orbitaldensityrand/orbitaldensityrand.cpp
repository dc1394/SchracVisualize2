/*! \file orbitaldensityrand.cpp
    \brief OrbitalDensityRandクラスの実装
    Copyright © 2019 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#include "myrandom/myrandsfmt.h"
#include "orbitaldensityrand.h"
#include "utility/utility.h"
#include <boost/assert.hpp>                                     // for boost::assert
#include <boost/math/special_functions/spherical_harmonic.hpp>  // for boost::math::spherical_harmonic
#include <boost/range/algorithm.hpp>                            // for boost::fill
#include <tbb/parallel_for.h>                                   // for tbb::parallel_for

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
            mr_(0.0, 1.0),
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
            tbb::parallel_for(
                tbb::blocked_range<int>(0, static_cast<std::int32_t>(vertexsize_.load())),
                [this, m, reim](auto const & range) {
                    for (auto && i = range.begin(); i != range.end(); ++i) {
                        FillSimpleVertex(m, reim, vertex_[i]);
                    }
                });
            break;

        case Normal_Nelson_type::NELSON:
            for (auto i = 0UL; i < vertexsize_.load(); i++) {
                FillSimpleVertex(m, vertex_[i]);
            }
            break;

        default:
            BOOST_ASSERT(!"nornelが異常!");
            break;
        }
        
		complete_.store(true);
	}

    void OrbitalDensityRand::FillSimpleVertex(std::int32_t m, SimpleVertex& ver)
    {
        using namespace boost::math;
        using namespace constants;

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

        auto x = std::sin(theta) * std::cos(phi) * pgd_->dphidr(r) / (*pgd_)(r);
        x += std::cos(theta) * std::cos(phi) / r * dylmdtheta / ylm;
        x -= std::sin(phi) / (r * std::sin(theta)) * dylmdphi / ylm;

        auto const t2 = std::sin(theta) * std::sin(phi);
        if (q_[1] * t2 < 0.0)
        {
            phi = pi<double>() + phi;
        }

        auto y = std::sin(theta) * std::sin(phi) * pgd_->dphidr(r) / (*pgd_)(r);
        y += std::cos(theta) * std::sin(phi) / r * dylmdtheta / ylm;
        y += std::cos(phi) / (r * std::sin(theta)) * dylmdphi / ylm;

        auto z = std::cos(theta) * pgd_->dphidr(r) / (*pgd_)(r);
        z -= std::sin(theta) / r * dylmdtheta / ylm;

        q_[0] += x * DT + mr_.myrand() * std::sqrt(DT);
        q_[1] += y * DT + mr_.myrand() * std::sqrt(DT);
        q_[2] += z * DT + mr_.myrand() * std::sqrt(DT);

        ver.Pos.x = static_cast<float>(q_[0]);
        ver.Pos.y = static_cast<float>(q_[1]);
        ver.Pos.z = static_cast<float>(q_[2]);

        ver.Color.x = 0.8f;
        ver.Color.y = 0.0f;
        ver.Color.z = 0.8f;
        ver.Color.w = 1.0f;
    }

	void OrbitalDensityRand::FillSimpleVertex(std::int32_t m, Re_Im_type reim, SimpleVertex & ver) const
	{
		if (thread_end_) {
			return;
		}

		auto pp = 0.0, p = 0.0;
		auto sign = 0;
		double x, y, z;

	    myrandom::MyRandSfmt mr(-rmax_, rmax_);
        myrandom::MyRandSfmt mr2(pgd_->Funcmin, pgd_->Funcmax);

		do {
			if (thread_end_) {
				return;
			}

			x = mr.myrand();
			y = mr.myrand();
			z = mr.myrand();

			auto const r = std::sqrt(x * x + y * y + z * z);
			if (r < pgd_->R_meshmin()) {
				continue;
			}

			switch (pgd_->Rho_wf_type_) {
			case getdata::GetData::Rho_Wf_type::RHO:
			{
				auto const phi = std::acos(x / std::sqrt(x * x + y * y));
                double v;
                if (m >= 0) {
                    v = boost::math::spherical_harmonic_r(pgd_->L, m, std::acos(z / r), phi);
                }
                else {
                    v = boost::math::spherical_harmonic_i(pgd_->L, m, std::acos(z / r), phi);
                }
                
                pp = ((*pgd_)(r) * v * v);
				p = mr2.myrand();
			}
			break;

			case getdata::GetData::Rho_Wf_type::WF:
			{
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

				pp = (*pgd_)(r) * ylm;
				p = mr2.myrand();
			}
			break;

			default:
                BOOST_ASSERT(!"何かがおかしい!");
				break;
			}

			switch (pgd_->Rho_wf_type_) {
			case getdata::GetData::Rho_Wf_type::RHO:
				sign = 1;
				break;

			case getdata::GetData::Rho_Wf_type::WF:
				sign = (pp > 0.0) - (pp < 0.0);
				break;

			default:
                BOOST_ASSERT(!"何かがおかしい!");
				break;
			}

			if (!m && pgd_->Rho_wf_type_ == getdata::GetData::Rho_Wf_type::WF && reim == Re_Im_type::IMAGINARY) {
				break;
			}
			
		} while (std::fabs(pp) < std::fabs(p));

		ver.Pos.x = static_cast<float>(x);
		ver.Pos.y = static_cast<float>(y);
		ver.Pos.z = static_cast<float>(z);

		ver.Color.x = sign > 0 ? 0.8f : 0.0f;
		ver.Color.y = sign < 0 ? 0.8f : 0.0f;
		ver.Color.z = 0.8f;
		ver.Color.w = 1.0f;
	}

	double GetRmax(std::shared_ptr<getdata::GetData> const & pgd)
	{
		auto const n = static_cast<double>(pgd->N);
		return (2.3622 * n + 3.3340) * n + 1.3228;
	}
}
