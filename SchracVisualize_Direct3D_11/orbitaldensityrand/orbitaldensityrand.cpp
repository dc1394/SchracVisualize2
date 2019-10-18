/*! \file orbitaldensityrand.cpp
    \brief OrbitalDensityRandクラスの実装
    Copyright © 2019 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#include "orbitaldensityrand.h"
#include "utility/utility.h"
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
		    rmax_(GetRmax(pgd)),
            mr_(-rmax_, rmax_),
            mr2_(pgd_->Funcmin, pgd_->Funcmax),
		    vertex_(VERTEXSIZE_INIT_VALUE)
    {
    }

    void OrbitalDensityRand::operator()(std::int32_t m, OrbitalDensityRand::Re_Im_type reim)
    {
        if (redraw_) {
            if (vertex_.size() != vertexsize_) {
                vertex_.resize(vertexsize_);
            }

            pth_.reset(new std::thread([this, m, reim] { ClearFillSimpleVertex(m, reim); }), [this](std::thread * pth)
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

	void OrbitalDensityRand::ClearFillSimpleVertex(std::int32_t m, OrbitalDensityRand::Re_Im_type reim)
	{
		complete_.store(false);

		SimpleVertex sv{};
		sv.Color = { 0.0f, 0.0f, 0.0f, 0.0f };
		sv.Pos = { 0.0f, 0.0f, 0.0f };
		boost::fill(vertex_, sv);

        for (auto i = 0UL; i < vertexsize_.load(); i++) {
            FillSimpleVertex(m, reim, vertex_[i]);
        }

		complete_.store(true);
	}


	void OrbitalDensityRand::FillSimpleVertex(std::int32_t m, OrbitalDensityRand::Re_Im_type reim, SimpleVertex & ver)
	{
		if (thread_end_) {
			return;
		}

		auto pp = 0.0, p = 0.0;
		auto sign = 0;
		double x, y, z;

		do {
			if (thread_end_) {
				return;
			}

			x = mr_.myrand();
			y = mr_.myrand();
			z = mr_.myrand();

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
				p = mr2_.myrand();
			}
			break;

			case getdata::GetData::Rho_Wf_type::WF:
			{
				auto const phi = std::acos(x / std::sqrt(x * x + y * y));
				double ylm = 0.0;
				switch (reim) {
				case OrbitalDensityRand::Re_Im_type::REAL:
					ylm = boost::math::spherical_harmonic_r(pgd_->L, m, std::acos(z / r), phi);
					break;

				case OrbitalDensityRand::Re_Im_type::IMAGINARY:
					ylm = boost::math::spherical_harmonic_i(pgd_->L, m, std::acos(z / r), phi);
					break;

				default:
                    BOOST_ASSERT(!"何かがおかしい!");
					break;
				}

				pp = (*pgd_)(r) * ylm;
				p = mr2_.myrand();
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

			if (!m && pgd_->Rho_wf_type_ == getdata::GetData::Rho_Wf_type::WF && reim == OrbitalDensityRand::Re_Im_type::IMAGINARY) {
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
