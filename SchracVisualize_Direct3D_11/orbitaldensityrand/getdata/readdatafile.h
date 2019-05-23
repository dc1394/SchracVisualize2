/*! \file readdatafile.h
    \brief 電子密度のデータファイルを読み込むクラスの宣言

    Copyright © 2015 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#ifndef _READDATAFILE_H_
#define _READDATAFILE_H_

#pragma once

#include <string>   // for std::wstring
#include <utility>  // for std::pair
#include <vector>   // for std::vector

namespace getdata {
    //! A class.
    /*!
        電子密度のデータファイルを読み込むクラス
    */
    class ReadDataFile final {
    public:
        // #region 型エイリアス

        using mypair = std::pair < std::vector<double>, std::vector<double> > ;

        // #endregion 型エイリアス

        // #region コンストラクタ・デストラクタ

        //! A constructor.
        /*!
            唯一のコンストラクタ
        */
        ReadDataFile() = default;

        //! A destructor.
        /*!
            デフォルトデストラクタ
        */
        ~ReadDataFile() = default;

        // #endregion コンストラクタ・デストラクタ

        // #region メンバ関数

        //!  A public member function.
        /*!
            実際に電子密度のデータファイルを読み込む
            \param filename rのメッシュと、そのメッシュにおける電子密度が記録されたデータファイル名
        */
        ReadDataFile::mypair readdatafile(std::string const & filename) const;

        // #endregion メンバ関数

        // #region メンバ変数

    private:
        //!A private member variable(constant expression).
        /*!
            バッファサイズ
        */
        static std::streamsize const BUFSIZE = 1024;

        // #endregion メンバ変数

        // #region 禁止されたコンストラクタ・メンバ関数

    private:
        //! A private copy constructor (deleted).
        /*!
            コピーコンストラクタ（禁止）
        */
        ReadDataFile(ReadDataFile const &) = delete;

        //! A private member function (deleted).
        /*!
            operator=()の宣言（禁止）
            \param コピー元のオブジェクト（未使用）
            \return コピー元のオブジェクト
        */
        ReadDataFile & operator=(ReadDataFile const &) = delete;

        // #endregion 禁止されたコンストラクタ・メンバ関数
    };
}

#endif  // _READDATAFILE_H_
