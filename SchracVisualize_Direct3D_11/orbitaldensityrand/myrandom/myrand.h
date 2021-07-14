/*! \file myrand.h
    \brief 正規分布乱数クラスの宣言

    Copyright © 2019 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#ifndef _MYRAND_H_
#define _MYRAND_H_

#pragma once

#include <random>   // for std::default_random_engine, std::normal_distribution, std::random_device

namespace myrandom {
    //! A template class.
    /*!
        自作乱数クラス
        \tparam Distribution 乱数分布クラス
    */
    class MyRand final {
        // #region コンストラクタ・デストラクタ

    public:
        //! A constructor.
        /*!
            唯一のコンストラクタ
            \param mean 乱数の平均
            \param variance 乱数の分散
        */
        MyRand(double mean, double variance);

        //! A destructor.
        /*!
            デフォルトデストラクタ
        */
        ~MyRand() = default;

        // #endregion コンストラクタ・デストラクタ

        // #region メンバ関数

        //!  A public member function.
        /*!
            [min, max]の閉区間で一様乱数を生成する
        */
        double myrand()
        {
            return distribution_(randengine_);
        }

        // #endregion メンバ関数

        // #region メンバ変数

    private:
        //! A private member variable.
        /*!
            乱数エンジン
        */
        std::default_random_engine randengine_;

        //! A private member variable.
        /*!
            乱数の分布
        */
        std::normal_distribution<> distribution_;
                        
        // #region 禁止されたコンストラクタ・メンバ関数

        //! A private constructor (deleted).
        /*!
            デフォルトコンストラクタ（禁止）
        */
        MyRand() = delete;

        //! A private copy constructor (deleted).
        /*!
            コピーコンストラクタ（禁止）
            \param dummy コピー元のオブジェクト（未使用）
        */
        MyRand(MyRand const & dummy) = delete;

        //! A private member function (deleted).
        /*!
            operator=()の宣言（禁止）
            \param dummy コピー元のオブジェクト（未使用）
            \return コピー元のオブジェクト
        */
        MyRand& operator=(MyRand const & dummy) = delete;

        // #endregion 禁止されたコンストラクタ・メンバ関数
    };

    inline MyRand::MyRand(double mean, double variance)
        :   distribution_(mean, std::sqrt(variance))
    {
        // ランダムデバイス
        std::random_device rnd;

        randengine_ = std::default_random_engine(rnd());
    }
}

#endif  // _MYRAND_H_
