/*! \file utility.h
    \brief ユーティリティ関数の宣言と実装

    Copyright © 2015-2019 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#ifndef _UTILITY_H_
#define _UTILITY_H_

#pragma once

#include <array>        // for std::array
#include <cstdint>      // for std::int32_t
#include <string>       // for std::wstring
#include <Windows.h>    // for MAX_PATH

namespace utility {
    //! A function.
    /*!
        マルチバイト文字列をワイド文字列に変換する
        \param mbs マルチバイト文字列
        \param codeMulti マルチバイト文字列のエンコード（デフォルト=932）
        \return ワイド文字列
    */
    std::wstring my_mbstowcs(std::string const& mbs, std::int32_t codeMulti = 932);

    //! A function.
    /*!
        ファイルを開く
        \return ファイル名
    */
    std::string myOpenFile();

    //! A function.
    /*!
        ワイド文字列をマルチバイト文字列に変換する
        \param wcs ワイド文字列
        \param codeMulti マルチバイト文字列のエンコード（デフォルト=932）
        \return マルチバイト文字列
    */
    std::string my_wcstombs(std::array<wchar_t, MAX_PATH> const& wcs, std::int32_t codeMulti = 932);

    //! A function.
    /*!
        ファイル選択ダイアログを開く
        \param hWnd ウィンドウハンドル
        \param filepath ファイルのパス
        \param filename ファイル名
        \param title ファイル選択ダイアログのタイトル
        \param defextension デフォルトのファイルの種類
        \return ファイル選択ダイアログの戻り値
    */
    BOOL showFileDialog(HWND hWnd, wchar_t* filepath, wchar_t* filename, wchar_t const* title, wchar_t const* defextension);

    template <typename T>
    //! A struct.
    /*!
        確保したメモリを安全に解放するクラス
        \tparam T 確保したメモリの型
    */
    struct Safe_Delete {
        //! A public member function.
        /*!
            確保したメモリを安全に解放する
            \param p 確保したメモリの先頭アドレス
        */
        void operator()(T* p) {
            if (p) {
               delete p;
                p = nullptr;
            }
        }
    };
}

#endif  // _UTILITY_H_
