/*! \file functional.h
    \brief std::function<double (double)>の代わりになるクラスの宣言と実装
    Copyright © 2019 @dc1394 All Rights Reserved.

	This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
    more details.

    You should have received a copy of the GNU General Public License along
    with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _FUNCTONAL_H_
#define _FUNCTONAL_H_

#pragma once

namespace myfunctional {
	template <typename FUNCTYPE>
	//! A template class.
    /*!
        std::function<double (double)>の代わりになるtemplate class
    */
    class Functional final {
        // #region コンストラクタ・デストラクタ

	public:
        //! A constructor.
        /*!
			唯一のコンストラクタ
			\param func operator()で呼び出す関数
        */
        explicit Functional(FUNCTYPE const & func) : func_(func) {}

		//! A destructor.
		/*!
			唯一のデストラクタ
		*/
		~Functional() = default;

        // #endregion コンストラクタ・デストラクタ

        // #region メンバ関数

        //! A public member function.
        /*!
            operator()の宣言と実装
            関数f(x)の値を返す
            \param x xの値
            \return f(x)の値
        */
        double operator()(double x) const
        {
            return func_(x);
        }

        // #endregion メンバ関数

    private:
        // #region メンバ変数

        //! A private const variable (reference).
        /*!
            operator()で呼び出す関数
        */
        FUNCTYPE const & func_;

        // #endregion メンバ変数

        // #region 禁止されたコンストラクタ・メンバ関数

    public:
        //! A public copy constructor (deleted).
        /*!
            コピーコンストラクタ（禁止）
            \param dummy コピー元のオブジェクト（未使用）
        */
        Functional(Functional const & dummy) = delete;

        //! A public member function (deleted).
        /*!
            operator=()の宣言（禁止）
            \param dummy コピー元のオブジェクト（未使用）
            \return コピー元のオブジェクト
        */
        Functional & operator=(Functional const & dummy) = delete;

        // #endregion 禁止されたコンストラクタ・メンバ関数
    };

	template <typename FUNCTYPE>
    //! A template function（非メンバ関数）.
    /*!
        Function<FUNCTYPE>を生成する
        \param func 格納する関数
        \return 生成されたFunction<FUNCTYPE>
    */
    Functional<FUNCTYPE> make_functional(FUNCTYPE const & func)
    {
        return Functional<FUNCTYPE>(func);
    }
}

#endif  // _FUNCTIONAL_H_

