﻿/*! \file orbitaldensityrand.h
    \brief OrbitalDensityRandクラスの宣言

    Copyright © 2019-2021 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#ifndef _ORBITALDENSITYRAND_H_
#define _ORBITALDENSITYRAND_H_

#pragma once

#include "DXUT.h"
#include "getdata/getdata.h"
#include "myfunctional/functional.h"
#include "myrandom/myrandsfmt.h"
#include "utility/property.h"
#include <array>                // for std::array
#include <atomic>               // for std::atomic
#include <memory>               // for std::shared_ptr, for std::unique_ptr
#include <thread>               // for std::thread
#include <vector>               // for std::vector

namespace orbitaldensityrand {
    //! A struct.
    /*!
        頂点構造体
    */
    struct SimpleVertex
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT4 Color;
    };

    //! A class.
    /*!
        軌道・電子密度の乱数生成クラス
    */
    class OrbitalDensityRand final {
    public:
        // #region 列挙型

        //! A enumerated type
        /*!
            Nelsonの確率力学を使うかどうかを表す列挙型
        */
        enum class Normal_Nelson_type {
            // Nelsonの確率力学を使わない
            NORMAL,
            // Nelsonの確率力学を使う
            NELSON
        };

        // #endregion 列挙型

        // #region コンストラクタ・デストラクタ

        //! A constructor.
        /*!
            唯一のコンストラクタ
            \param pgd rのメッシュとデータ
        */
        explicit OrbitalDensityRand(std::shared_ptr<getdata::GetData> const & pgd);

        //! A default destructor.
        /*!
            デフォルトデストラクタ
        */
        ~OrbitalDensityRand() = default;

        // #endregion コンストラクタ・デストラクタ

        // #region メンバ関数

        //! A public member function.
        /*!
            再描画する
            \param m 磁気量子数
            \param nornel ネルソンの確率力学を使用するかどうか
            \return 再描画が成功したかどうか
        */
        void operator()(std::int32_t m, Normal_Nelson_type nornel);

    private:
        //! A private member function.
        /*!
            SimpleVertexのデータをクリアし、新しいデータを詰める
            \param m 磁気量子数
            \param nornel ネルソンの確率力学を使用するかどうか
        */
        void ClearFillSimpleVertex(std::int32_t m, Normal_Nelson_type nornel);

        //! A private member function.
        /*!
            SimpleVertexにデータを詰める
            \param m 磁気量子数
        */
        void FillSimpleVertex(std::int32_t m);

        //! A private member function.
        /*!
            SimpleVertexにデータを詰める
            \param m 磁気量子数
            \param starti 描画開始の際のiのインデックス
            \param endi 描画終了の際のiのインデックス
        */
        void FillSimpleVertex(std::int32_t m, std::int32_t starti, std::int32_t endi);

        //! A private member function.
        /*!
            関数の数値微分を求める
            \param x 微分する座標x
            \param func 微分対象の関数
        */
        template <typename FUNCTYPE>
        double Numerical_diff(double x, myfunctional::Functional<FUNCTYPE> const & func);

        //! A private member function.
        /*!
            現在の座標を初期値に戻す
        */
        void Resetq()
        {
            q_ = q0_;
        }

        // #endregion メンバ関数

        // #region プロパティ

    public:
        //! A property.
        /*!
            描画スレッドの作業が完了したかどうかへのプロパティ
        */
        utility::Property<bool> const Complete;

        //! A property.
        /*!
            時間刻み（アト秒）へのプロパティ
        */
        utility::Property<double> Dt;

        //! A property.
        /*!
            経過時間のカウントへのプロパティ
        */
        utility::Property<double> Elapsed_time;
        
        //! A property.
        /*!
            スレッドへのスマートポインタのプロパティ
        */
        utility::Property<std::shared_ptr<std::thread> const &> const Pth;

        //! A property.
        /*!
            再描画するかどうかへのプロパティ
        */
        utility::Property<bool> Redraw;

        //! A property.
        /*!
            描画するrの最大値へのプロパティ
        */
        utility::Property<double> Rmax;

        //! A property.
        /*!
            スレッドを強制終了するかどうかへのプロパティ
        */
        utility::Property<bool> Thread_end;

        //! A property.
        /*!
            頂点へのプロパティ
        */
        utility::Property<std::vector<SimpleVertex> const &> Vertex;

        //! A property.
        /*!
            頂点数へのプロパティ
        */
        utility::Property<std::vector<SimpleVertex>::size_type> Vertexsize;

        // #endregion プロパティ

        // #region メンバ変数

        //! A public static member variable (constant).
        /*!
            頂点数の初期値（電子密度）
        */
        static std::vector<SimpleVertex>::size_type const RHO_VERTEXSIZE_INIT_VALUE = 5000000;

        //! A public static member variable (constant).
        /*!
            頂点数の初期値（ネルソンの確率力学）
        */
        static std::vector<SimpleVertex>::size_type const VERTEXSIZE_INIT_VALUE_FOR_NELSON = 5000000;

        //! A public static member variable (constant).
        /*!
            頂点数の初期値（波動関数）
        */
        static std::vector<SimpleVertex>::size_type const WF_VERTEXSIZE_INIT_VALUE = 1000000;

    private:
        //! A private member variable (constant expression).
        /*!
            アト秒から原子単位（秒）へ変換するときの定数
        */
        static constexpr auto ATTOSECTOAU = 0.04134137333518131;

        //! A private member variable (constant expression).
        /*!
            数値微分の刻み幅
        */
        static auto constexpr DH = 1.0E-7;

        //! A private member variable (constant expression).
        /*!
            時間刻み（アト秒）の初期値
        */
        static auto constexpr DT = 0.1;

        //! A private member variable (constant expression).
        /*!
            許容誤差
        */
        static auto constexpr EPS = 1.0E-15;

        //! A private member variable (constant expression).
        /*!
            0の判定に使う閾値
        */
        static auto constexpr THRESHOLD = 1.0E-15;
                
        //! A private member variable.
        /*!
            描画スレッドの作業が完了したかどうか
        */
        std::atomic<bool> complete_ = false;

        //! A private member variable.
        /*!
            時間経過のカウント
        */
        std::uint32_t count_ = 0U;

        //! A private member variable.
        /*!
            時間刻み（アト秒）
        */
        double dt_ = DT;

        //! A private member variable.
        /*!
            SFMT乱数オブジェクト
        */
        myrandom::MyRandSfmt mr_;

        //! A private member variable.
        /*!
            rのメッシュとデータ
        */
        std::shared_ptr<getdata::GetData> pgd_;

        //! A private member variable.
        /*!
            スレッドへのスマートポインタ
        */
        std::shared_ptr<std::thread> pth_;

        //! A private member variable (constant expression).
        /*!
            座標の初期値
        */
        std::array<double, 3> const q0_;

        //! A private member variable.
        /*!
            現在の座標
        */
        std::array<double, 3> q_;

        //! A private member variable.
        /*!
            再描画するかどうか
        */
        bool redraw_ = true;
        
        //! A private member variable.
        /*!
            描画するrの最大値
        */
        double rmax_;

        //! A private member variable.
        /*!
            スレッドを強制終了するかどうか
        */
        std::atomic<bool> thread_end_ = false;

        //! A private member variable.
        /*!
            頂点数
        */
        std::atomic<std::vector<SimpleVertex>::size_type> vertexsize_;

        //! A private member variable.
        /*!
            vertex buffer
        */
        std::vector<SimpleVertex> vertex_;

    public:
        // #region 禁止されたコンストラクタ・メンバ関数

        //! A private constructor (deleted).
        /*!
            デフォルトコンストラクタ（禁止）
        */
        OrbitalDensityRand() = delete;

        //! A private copy constructor (deleted).
        /*!
            コピーコンストラクタ（禁止）
            \param dummy コピー元のオブジェクト（未使用）
        */
        OrbitalDensityRand(OrbitalDensityRand const& dummy) = delete;

        //! A private member function (deleted).
        /*!
            operator=()の宣言（禁止）
            \param dummy コピー元のオブジェクト（未使用）
            \return コピー元のオブジェクト
        */
        OrbitalDensityRand& operator=(OrbitalDensityRand const& dummy) = delete;

        // #endregion 禁止されたコンストラクタ・メンバ関数
    };

    //! A function.
    /*!
        データオブジェクトからrmaxを求める
        \param pgd データオブジェクト
        \return rmaxの値
    */
    double GetRmax(std::shared_ptr<getdata::GetData> const& pgd);

    //! A function.
    /*!
        実関数表示の球面調和関数を求める
        \param l 方位量子数
        \param m 磁気量子数
        \param theta θ
        \param phi φ
        \return 実関数表示の球面調和関数
    */
    double Spherical_harmonic(std::int32_t l, std::int32_t m, double theta, double phi);

    template <typename FUNCTYPE>
    inline double OrbitalDensityRand::Numerical_diff(double x, myfunctional::Functional<FUNCTYPE> const & func)
    {
        auto const term1 = func(x + 3.0 * DH) / 60.0;
        auto const term2 = -3.0 / 20.0 * func(x + 2.0 * DH);
        auto const term3 = 3.0 / 4.0 * func(x + DH);
        auto const term4 = -3.0 / 4.0 * func(x - DH);
        auto const term5 = 3.0 / 20.0 * func(x - 2.0 * DH);
        auto const term6 = -func(x - 3.0 * DH) / 60.0;

        return (term1 + term2 + term3 + term4 + term5 + term6) / DH;
    }
}

#endif  // _ORBITALDENSITYRAND_H_
