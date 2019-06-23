/*! \file orbitaldensityrand.h
    \brief OrbitalDensityRandクラスの宣言

    Copyright © 2019 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#ifndef _ORBITALDENSITYRAND_H_
#define _ORBITALDENSITYRAND_H_

#pragma once

#include "DXUT.h"
#include "getdata/getdata.h"
#include "utility/property.h"
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
            実部か虚部かを表す列挙型
        */
        enum class Re_Im_type {
            // 実部
            REAL,
            // 虚部
            IMAGINARY
        };

        // #endregion 列挙型

        // #region コンストラクタ・デストラクタ

        //! A constructor.
        /*!
            唯一のコンストラクタ
            \param pgd rのメッシュとデータ
        */
        explicit OrbitalDensityRand(std::shared_ptr<getdata::GetData> const& pgd);

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
            \param reim 実部を描画するか、虚部を描画するか
            \return 再描画が成功したかどうか
        */
        void operator()(std::int32_t m, OrbitalDensityRand::Re_Im_type reim);

    private:
        //! A private member function.
        /*!
            SimpleVertexのデータをクリアし、新しいデータを詰める
            \param m 磁気量子数
            \param reim 実部を描画するか、虚部を描画するか
        */
        void ClearFillSimpleVertex(std::int32_t m, OrbitalDensityRand::Re_Im_type reim);

        //! A private member function.
        /*!
            SimpleVertexにデータを詰める
            \param m 磁気量子数
            \param reim 実部を描画するか、虚部を描画するか
            \param ver 対象のSimpleVertex
        */
        void FillSimpleVertex(std::int32_t m, OrbitalDensityRand::Re_Im_type reim, SimpleVertex& ver) const;

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
            スレッドへのスマートポインタのプロパティ
        */
        utility::Property<std::shared_ptr<std::thread> const&> const Pth;

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
        utility::Property<std::vector<SimpleVertex> const&> Vertices;

        //! A property.
        /*!
            頂点数へのプロパティ
        */
        utility::Property<std::vector<SimpleVertex>::size_type> Verticessize;

        // #endregion プロパティ

        // #region メンバ変数

    public:
        //! A public static member variable (constant).
        /*!
            頂点数の初期値
        */
        static std::vector<SimpleVertex>::size_type const VERTICESSIZE_INIT_VALUE = 100000;

    private:
        //! A private member variable.
        /*!
            描画スレッドの作業が完了したかどうか
        */
        std::atomic<bool> complete_ = false;

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
        std::atomic<std::vector<SimpleVertex>::size_type> verticessize_ = VERTICESSIZE_INIT_VALUE;

        //! A private member variable.
        /*!
            vertex buffer
        */
        std::vector<SimpleVertex> vertices_;

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
}

#endif  // _ORBITALDENSITYRAND_H_
