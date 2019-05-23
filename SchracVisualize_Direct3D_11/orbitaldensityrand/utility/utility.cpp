/*! \file utility.h
    \brief ユーティリティ関数の実装

    Copyright ©  2015 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#include "DXUT.h"
#include "utility.h"
#include <vector>           // for std::vector
#include <system_error>     // for std::system_category

namespace utility {
    std::wstring my_mbstowcs(std::string const & mbs, std::int32_t codeMulti)
    {
        // ワイド文字変換後のサイズを調べる
        auto const sizeWide = ::MultiByteToWideChar(codeMulti, 0, mbs.c_str(), -1, nullptr, 0);
        if (!sizeWide) {
            // 失敗
            throw std::system_error(std::error_code(::GetLastError(), std::system_category()));
        }

        std::vector<wchar_t> wcsvec(sizeWide);

        // ワイド文字に変換
        ::MultiByteToWideChar(codeMulti, 0, mbs.c_str(), -1, wcsvec.data(), sizeWide);

        return std::wstring(wcsvec.begin(), wcsvec.end() - 1);
    }

    std::string myOpenFile()
    {
        // ファイルを開く
        static std::array<wchar_t, MAX_PATH> filename_full;   // ファイル名(フルパス)を受け取る領域
        static std::array<wchar_t, MAX_PATH> filename;        // ファイル名を受け取る領域

        do {
            if (showFileDialog(nullptr, filename_full.data(), filename.data(), L"ファイルを開く", L"csv")) {
                break;
            }

            ::MessageBox(nullptr, L"ファイルを選択してください", L"エラー", MB_OK | MB_ICONWARNING);
        } while (true);

        return my_wcstombs(filename_full);
    }

    std::string my_wcstombs(std::array<wchar_t, MAX_PATH> const & wcs, std::int32_t codeMulti)
    {
        // マルチバイト変換後のサイズを調べる
        auto const sizeMulti = ::WideCharToMultiByte(codeMulti, 0, wcs.data(), -1, nullptr, 0, nullptr, nullptr);
        if (!sizeMulti) {
            // 失敗
            throw std::system_error(std::error_code(::GetLastError(), std::system_category()));
        }

        std::array<char, MAX_PATH> mbs;

        // マルチバイトに変換
        ::WideCharToMultiByte(codeMulti, 0, wcs.data(), -1, mbs.data(), sizeMulti, nullptr, nullptr);

        return std::string(mbs.data());
    }

    BOOL showFileDialog(HWND hWnd, wchar_t * filepath, wchar_t * filename, wchar_t * title, wchar_t * defextension)
    {
        OPENFILENAME ofn = { 0 };

        // 構造体に情報をセット
        ofn.lStructSize = sizeof(ofn);			                                // 構造体のサイズ
        ofn.hwndOwner = hWnd;					                                // コモンダイアログの親ウィンドウハンドル
        ofn.lpstrFilter = L"csv files(*.csv)\0*.csv\0All files(*.*)\0*.*\0\0";  // ファイルの種類
        ofn.lpstrFile = filepath;				                                // 選択されたファイル名(フルパス)を受け取る変数のアドレス
        ofn.lpstrFileTitle = filename;			                                // 選択されたファイル名を受け取る変数のアドレス
        ofn.nMaxFile = MAX_PATH;				                                // lpstrFileに指定した変数のサイズ
        ofn.nMaxFileTitle = MAX_PATH;			                                // lpstrFileTitleに指定した変数のサイズ
        ofn.Flags = OFN_FILEMUSTEXIST;			                                // フラグ指定
        ofn.lpstrTitle = title;					                                // コモンダイアログのキャプション
        ofn.lpstrDefExt = defextension; 		                                // デフォルトのファイルの種類

        // ファイルを開くコモンダイアログを作成
        return ::GetOpenFileName(&ofn);
    }
}