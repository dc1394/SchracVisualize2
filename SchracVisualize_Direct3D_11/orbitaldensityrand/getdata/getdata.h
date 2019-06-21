/*! \file getdata.h
    \brief r???????????????????????????????

    Copyright � 2015-2019 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#ifndef _GETRMESHANDRHO_H_
#define _GETRMESHANDRHO_H_

#pragma once

#include "../utility/property.h"
#include <cstdint>          // for std::int32_t, std::uint32_t
#include <memory>           // for std::unique_ptr
#include <string>           // for std::string
#include <gsl/gsl_spline.h> // for gsl_interp_accel, gsl_interp_accel_free, gsl_spline, gsl_spline_free

namespace getdata {
    using namespace utility;

    //! A class.
    /*!
        r??????????????????????????????
    */
    class GetData final {
        // #region ???

    public:
        //!  A enumerated type
        /*!
            ????????????????
        */
        enum class Rho_Wf_type {
            // ??
            RHO,
            // ??????
            WF
        };

        // #endregion ???

        // #region ??????????????

        //! A constructor.
        /*!
            ??????????
            \param filename r???????????????????????????????????
        */
        GetData(std::string const& filename);

        //! A destructor.
        /*!
            ???????????
        */
        ~GetData() = default;

        // #endregion ??????????????

        // #region ?????

        //!  A public member function (const).
        /*!
            ???????
            \param r r??
            \return ????
        */
        double operator()(double r) const;

        // #endregion ?????

        // #region ?????

        //! A property.
        /*!
            ???
        */
        Property<std::string const&> Atomname;

        //! A property.
        /*!
            ????????????
        */
        Property<double> const Funcmax;

        //! A property.
        /*!
            ????????????
        */
        Property<double> const Funcmin;

        //!  A property.
        /*!
            ????????????
        */
        Property<std::uint32_t> const L;

        //!  A property.
        /*!
            ???????????
        */
        Property<std::int32_t> const N;

        //!  A property.
        /*!
            ?????????
        */
        Property<std::string> const Orbital;

        //!  A private member variable.
        /*!
            ????????????????
        */
        Property<GetData::Rho_Wf_type> const Rho_wf_type_;

        //! A property.
        /*!
            r???????????????
        */
        Property<double> const R_meshmin;

        // #endregion ?????

        // #region ?????

    private:
        //! A private member variable.
        /*!
        gsl_interp_accel??????????
        */
        std::unique_ptr<gsl_interp_accel, decltype(&gsl_interp_accel_free)> const acc_;

        //!  A private member variable.
        /*!
            ???
        */
        std::string atomname_;

        //!  A private member variable.
        /*!
            ??????
        */
        double funcmax_;

        //!  A private member variable.
        /*!
            ??????
        */
        double funcmin_;

        //!  A private member variable.
        /*!
            ?????
        */
        std::uint32_t l_;

        //!  A private member variable.
        /*!
            ????
        */
        std::int32_t n_;

        //!  A private member variable.
        /*!
            ??
        */
        std::string orbital_;

        //!  A private member variable.
        /*!
            ?????????
        */
        GetData::Rho_Wf_type rho_wf_type_;

        //!  A private member variable.
        /*!
            r?????????
        */
        double r_meshmin_;

        //! A private member variable.
        /*!
            gsl_interp_type??????????
        */
        std::unique_ptr<gsl_spline, decltype(&gsl_spline_free)> spline_;

        // #endregion ?????

        // #region ??????????????????

    public:
        //! A public constructor (deleted).
        /*!
            ????????????(??)
        */
        GetData() = delete;

        //! A public copy constructor (deleted).
        /*!
            ??????????(??)
            \param dummy ???????????(???)
        */
        GetData(GetData const& dummy) = delete;

        //! A public member function (deleted).
        /*!
            operator=()???(??)
            \param dummy ???????????(???)
            \return ???????????
        */
        GetData& operator=(GetData const& dummy) = delete;

        // #endregion ??????????????????
    };
}

#endif  // _GETRMESHANDRHO_H_
