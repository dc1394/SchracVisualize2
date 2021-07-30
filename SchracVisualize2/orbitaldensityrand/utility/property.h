/*! \file property.h
    \brief C++でプロパティを実現するクラスの宣言と実装

    Copyright ©  2015 @dc1394 All Rights Reserved.
    (but this is originally adapted by επιστημη for list-07 from http://codezine.jp/article/detail/7571 )
    This software is released under the BSD 2-Clause License.
*/

#ifndef _PROPERTY_H_
#define _PROPERTY_H_

#pragma once

#include <functional>   // for std::function
#include <utility>      // for std::move

namespace utility {
    template <typename T>
    //! A template class.
    /*!
        C++でプロパティを実現するクラス
    */
    class Property final {
        // #region コンストラクタ・デストラクタ

    public:
        //! A constructor.
        /*!
            getterとsetterをセットする
            \param getter getterの関数オブジェクト
            \param setter setterの関数オブジェクト
        */
        Property(std::function<T()> && getter, std::function<T(T const &)> && setter) :
            get(std::move(getter)), set(std::move(setter))
        {
        }

        //! A destructor.
        /*!
            デフォルトデストラクタ
        */
        ~Property() = default;

        // #endregion コンストラクタ・デストラクタ

        // #region メンバ関数
        
        //! A public member function (const).
        /*!
            operator()()の実装（getterを呼び出す）
            \return getterの戻り値
        */
        T operator()() const
        {
            return get();
        }

        //! A public member function.
        /*!
            operator()(T const &)の実装（setterを呼び出す）
            \param value setterに渡す引数
            \return setterの戻り値
        */
        T operator()(T const & value)
        {
            return set(value);
        }

        //! A public member function (const).
        /*!
            型変換キャスト演算子の実装（getterを呼び出す）
            \return getterの戻り値
        */
        operator T() const
        {
            return get();
        }
        
        //! A public member function (const).
        /*!
            operator=()の実装（setterを呼び出す）
            \return setterの戻り値
        */
        T operator=(T const & value)
        {
            return set(value);
        }

        // #endregion メンバ関数

    private:
        // #region メンバ変数

        //! A private member variable.
        /*!
            getterに対応するstd::function<T const &()>
        */
        std::function<T()> const get;

        //! A private member variable.
        /*!
            setterに対応するstd::function<T const &()>
        */
        std::function<T(T const &)> const set;

        // #endregion メンバ変数

        // #region 禁止されたコンストラクタ・メンバ関数

        //! A private constructor (deleted).
        /*!
        デフォルトコンストラクタ（禁止）
        */
        Property() = delete;

        //! A private copy constructor (deleted).
        /*!
        コピーコンストラクタ（禁止）
        */
        Property(Property const &) = delete;

        //! A private member function (deleted).
        /*!
            operator=()の宣言（禁止）
            \param コピー元のオブジェクト（未使用）
            \return コピー元のオブジェクト
        */
        Property & operator=(Property const &) = delete;

        // #endregion 禁止されたコンストラクタ・メンバ関数
    };
}

#endif  // _PROPERTY_H_
