/*! \file getdata.h
    \brief rのメッシュと、そのメッシュにおける電子密度を与えるクラスの実装

    Copyright ｩ 2015-2019 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#include "getdata.h"
#include "readdatafile.h"
#include <iterator>                     // for std::distance
#include <stdexcept>                    // for std::runtime_error
#include <boost/algorithm/string.hpp>   // for boost::algorithm
#include <boost/assert.hpp>             // for BOOST_ASSERT
#include <boost/range/algorithm.hpp>    // for boost::max_element

namespace getdata {
    // #region コンストラクタ

    GetData::GetData(std::string const & filename) :
        Atomname([this] { return std::cref(atomname_); }, nullptr),
        Phimax([this] { return phimax_; }, nullptr),
        L([this] { return l_; }, nullptr),
        N([this] { return n_; }, nullptr),
        Orbital([this] { return orbital_; }, nullptr),
        R2rhomaxr([this] { return r2rhomaxr_; }, nullptr),
        Rho_wf_type([this] { return rho_wf_type_; }, nullptr),
        R_meshmin([this] { return r_meshmin_; }, nullptr),
        acc_(gsl_interp_accel_alloc(), gsl_interp_accel_free),
        spline_(nullptr, gsl_spline_free)
    {
        using namespace boost::algorithm;

        // トークン分割
        std::vector<std::string> tokenstmp, tokens;
        split(tokenstmp, filename, is_any_of("\\"), token_compress_on);
        split(tokens, tokenstmp.back(), is_any_of("_"), token_compress_on);

        if (tokens[0].find("rho") != std::string::npos) {
            rho_wf_type_ = GetData::Rho_Wf_type::RHO;
        }
        else if (tokens[0].find("wf") != std::string::npos) {
            rho_wf_type_ = GetData::Rho_Wf_type::WF;
        }
        else {
            throw std::runtime_error("ファイル名が異常です！");
        }

        if (tokens[1] == "H") {
            atomname_ = "Hydrogen";
        }
        else if (tokens[1] == "He") {
            atomname_ = "Helium";
        }
        else {
            throw std::runtime_error("ファイル名が異常です！");
        }

        orbital_ = tokens[2][0];
        n_ = static_cast<std::int32_t>(tokens[2][0] - '0');

        switch (tokens[2][1]) {
        case 's':
            l_ = 0;
            orbital_ += 's';
            break;

        case 'p':
            l_ = 1;
            orbital_ += 'p';
            break;

        case 'd':
            l_ = 2;
            orbital_ += 'd';
            break;

        case 'f':
            l_ = 3;
            orbital_ += 'f';
            break;

        case 'g':
            l_ = 4;
            orbital_ += 'g';
            break;

        default:
            throw std::runtime_error("ファイル名が異常です！");
            break;
        }

        auto const restuple = ReadDataFile().readdatafile(filename);
        r_mesh_.assign(std::get<0>(restuple).begin(), std::get<0>(restuple).end());
        phi_.assign(std::get<1>(restuple).begin(), std::get<1>(restuple).end());

        BOOST_ASSERT(r_mesh_.size() == phi_.size());

        phimax_ = *boost::max_element(phi_);

        r_meshmin_ = r_mesh_[0];

        spline_.reset();
        spline_ = std::unique_ptr<gsl_spline, decltype(&gsl_spline_free)>(
            gsl_spline_alloc(gsl_interp_cspline, r_mesh_.size()), gsl_spline_free);

        gsl_spline_init(spline_.get(), r_mesh_.data(), phi_.data(), r_mesh_.size());

        std::vector<double> temp(phi_);

        auto const size = temp.size();
        for (auto i = 0U; i < size; i++)
        {
            temp[i] *= (temp[i] * r_mesh_[i] * r_mesh_[i]);
        }

        r2rhomaxr_ = r_mesh_[std::distance(temp.begin(), boost::max_element(temp))];
    }

    // #endregion コンストラクタ

}
