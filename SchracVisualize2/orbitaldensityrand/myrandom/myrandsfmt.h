/*! \file myrandsfmt.h
    \brief SFMTを使った自作乱数クラスの宣言

    Copyright © 2017-2021 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#ifndef _MYRANDSFMT_H_
#define _MYRANDSFMT_H_

#pragma once

#include "../SFMT-src-1.5.1/SFMT.h"
#include <optional>                             // for std::optional
#include <random>						        // for std::random_device
#include <boost/math/constants/constants.hpp>   // for boost::math::constants::pi

namespace myrandom {
    //! A class.
    /*!
        自作乱数クラス
    */
    class MyRandSfmt final {
        // #region コンストラクタ・デストラクタ

    public:
        //! A constructor.
        /*!
            デフォルトコンストラクタ
        */
        MyRandSfmt();

        //! A destructor.
        /*!
            デフォルトデストラクタ
        */
        ~MyRandSfmt() = default;

        // #endregion コンストラクタ・デストラクタ

        // #region メンバ関数

        //!  A public member function.
        /*!
            [0.0, 1.0]の閉区間で一様乱数を生成する
            \return [0.0, 1.0]の閉区間における一様乱数
        */
        double myrand()
        {
            return sfmt_genrand_real1(&sfmt_);
        }

        //!  A public member function.
        /*!
            平均0、分散1の正規乱数を生成する
            \return 平均0、分散1の正規乱数
        */
        double normal_distribution_rand();

        //!  A public member function.
        /*!
            平均mu、分散sigma^2の正規乱数を生成する
            \param mu 正規乱数の平均
            \param sigma2 正規乱数の分散
            \return 平均mu、分散sigma^2の正規乱数
        */
        double normal_distribution_rand(double mu, double sigma2);

        // #endregion メンバ関数

        // #region メンバ変数

    private:
        //! A private member variable.
        /*!
            生成された乱数
        */
        std::optional<double> normal_dist_rand_ = std::nullopt;
                
        //! A private member variable.
        /*!
            乱数エンジン
        */
        sfmt_t sfmt_;

        // #region 禁止されたコンストラクタ・メンバ関数

    public:
        //! A public copy constructor (deleted).
        /*!
            コピーコンストラクタ（禁止）
            \param dummy コピー元のオブジェクト（未使用）
        */
        MyRandSfmt(MyRandSfmt const & dummy) = delete;

        //! A public member function (deleted).
        /*!
            operator=()の宣言（禁止）
            \param dummy コピー元のオブジェクト（未使用）
            \return コピー元のオブジェクト
        */
        MyRandSfmt& operator=(MyRandSfmt const & dummy) = delete;

        // #endregion 禁止されたコンストラクタ・メンバ関数
    };

    inline MyRandSfmt::MyRandSfmt()
    {
        // ランダムデバイス
        std::random_device rnd;

        // 乱数エンジン
        sfmt_init_gen_rand(&sfmt_, rnd());
    }
}

#endif  // _MYRANDSFMT_H_
