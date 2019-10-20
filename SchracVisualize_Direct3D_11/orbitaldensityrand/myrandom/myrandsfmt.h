/*! \file myrandsfmt.h
    \brief SFMTを使った自作乱数クラスの宣言

    Copyright © 2017 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#ifndef _MYRANDSFMT_H_
#define _MYRANDSFMT_H_

#pragma once

#include "../SFMT-src-1.5.1/SFMT.h"
#include <cstdint>						// for std::uint32_t, std::uint_least32_t
#include <functional>					// for std::ref
#include <random>						// for std::random_device
#include <vector>						// for std::vector
#include <boost/range/algorithm.hpp>    // for boost::generate

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
            唯一のコンストラクタ
            \param min 乱数分布の最小値
            \param max 乱数分布の最大値
        */
        MyRandSfmt(double min, double max);

        //! A destructor.
        /*!
            デフォルトデストラクタ
        */
        ~MyRandSfmt() = default;

        // #endregion コンストラクタ・デストラクタ

        // #region メンバ関数

        //!  A public member function.
        /*!
            [min, max]の閉区間で一様乱数を生成する
        */
        double myrand()
        {
            return sfmt_genrand_real1(&sfmt_) * (max_ - min_) + min_;
        }

        // #endregion メンバ関数

        // #region メンバ変数

    private:
        //! A private static member variable (constant expression).
        /*!
            初期乱数生成用のstd::vectorのサイズ
        */
        static std::vector<std::uint_least32_t>::size_type const SIZE = 1;

        //! A private member variable (constant).
        /*!
            乱数分布の最大値
        */
        double const max_;

        //! A private member variable (constant).
        /*!
            乱数分布の最小値
        */
        double const min_;

        //! A private member variable.
        /*!
            乱数エンジン
        */
        sfmt_t sfmt_;

        // #region 禁止されたコンストラクタ・メンバ関数

    public:
        //! A public constructor (deleted).
        /*!
            デフォルトコンストラクタ（禁止）
        */
        MyRandSfmt() = delete;

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

    inline MyRandSfmt::MyRandSfmt(double min, double max)
        : max_(max),
          min_(min)
    {
        // ランダムデバイス
        std::random_device rnd;

        // 初期化用ベクタ
        std::vector<std::uint_least32_t> v(SIZE);

        // ベクタの初期化
        // 非決定的な乱数でシード列を構築する
        boost::generate(v, std::ref(rnd));

        // 乱数エンジン
        sfmt_init_gen_rand(&sfmt_, v[0]);
    }
}

#endif  // _MYRANDSFMT_H_
