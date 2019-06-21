/*! \file getdata.h
    \brief rのメッシュと、そのメッシュにおける電子密度を与えるクラスの宣言

    Copyright © 2015-2019 @dc1394 All Rights Reserved.
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
        rのメッシュと、そのメッシュにおける動径波動関数を与えるクラス
    */
    class GetData final {
        // #region 列挙型

    public:
        //!  A enumerated type
        /*!
            密度か動径波動関数かを表す列挙型
        */
        enum class Rho_Wf_type {
            // 密度
            RHO,
            // 動径波動関数
            WF
        };

        // #endregion 列挙型

        // #region コンストラクタ・デストラクタ

        //! A constructor.
        /*!
            唯一のコンストラクタ
            \param filename rのメッシュと、そのメッシュにおける電子密度が記録されたデータファイル名
        */
        GetData(std::string const& filename);

        //! A destructor.
        /*!
            デフォルトデストラクタ
        */
        ~GetData() = default;

        // #endregion コンストラクタ・デストラクタ

        // #region メンバ関数

        //!  A public member function (const).
        /*!
            関数の値を返す
            \param r rの値
            \return 関数の値
        */
        double operator()(double r) const;

        // #endregion メンバ関数

        // #region プロパティ

        //! A property.
        /*!
            元素名
        */
        Property<std::string const&> Atomname;

        //! A property.
        /*!
            関数の最大値のプロパティ
        */
        Property<double> const Funcmax;

        //! A property.
        /*!
            関数の最小値のプロパティ
        */
        Property<double> const Funcmin;

        //!  A property.
        /*!
            方位量子数へのプロパティ
        */
        Property<std::uint32_t> const L;

        //!  A property.
        /*!
            主量子数へのプロパティ
        */
        Property<std::int32_t> const N;

        //!  A property.
        /*!
            軌道へのプロパティ
        */
        Property<std::string> const Orbital;

        //!  A private member variable.
        /*!
            解く方程式のタイプへのプロパティ
        */
        Property<GetData::Rho_Wf_type> const Rho_wf_type_;

        //! A property.
        /*!
            rのメッシュの最小値のプロパティ
        */
        Property<double> const R_meshmin;

        // #endregion プロパティ

        // #region メンバ変数

    private:
        //! A private member variable.
        /*!
            gsl_interp_accelへのスマートポインタ
        */
        std::unique_ptr<gsl_interp_accel, decltype(&gsl_interp_accel_free)> const acc_;

        //!  A private member variable.
        /*!
            元素名
        */
        std::string atomname_;

        //!  A private member variable.
        /*!
            関数の最大値
        */
        double funcmax_;

        //!  A private member variable.
        /*!
            関数の最小値
        */
        double funcmin_;

        //!  A private member variable.
        /*!
            方位量子数
        */
        std::uint32_t l_;

        //!  A private member variable.
        /*!
            主量子数
        */
        std::int32_t n_;

        //!  A private member variable.
        /*!
            軌道
        */
        std::string orbital_;

        //!  A private member variable.
        /*!
            解く方程式のタイプ
        */
        GetData::Rho_Wf_type rho_wf_type_;

        //!  A private member variable.
        /*!
            rのメッシュの最小値
        */
        double r_meshmin_;

        //! A private member variable.
        /*!
            gsl_interp_typeへのスマートポインタ
        */
        std::unique_ptr<gsl_spline, decltype(&gsl_spline_free)> spline_;

        // #endregion メンバ変数

        // #region 禁止されたコンストラクタ・メンバ関数

    public:
        //! A public constructor (deleted).
        /*!
            デフォルトコンストラクタ（禁止）
        */
        GetData() = delete;

        //! A public copy constructor (deleted).
        /*!
            コピーコンストラクタ（禁止）
            \param dummy コピー元のオブジェクト（未使用）
        */
        GetData(GetData const& dummy) = delete;

        //! A public member function (deleted).
        /*!
            operator=()の宣言（禁止）
            \param dummy コピー元のオブジェクト（未使用）
            \return コピー元のオブジェクト
        */
        GetData& operator=(GetData const& dummy) = delete;

        // #endregion 禁止されたコンストラクタ・メンバ関数
    };
}

#endif  // _GETRMESHANDRHO_H_
