/*! \file readdatafile.cpp
    \brief �d�q���x�̃f�[�^�t�@�C����ǂݍ��ރN���X�̎���

    Copyright � 2015-2019 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#include "DXUT.h"
#include "readdatafile.h"
#include <array>                        // for std::array
#include <fstream>                      // for std::ifstream
#include <stdexcept>                    // for std::runtime_error
#include <boost/algorithm/string.hpp>   // for boost::algorithm

namespace getdata {
    ReadDataFile::mypair ReadDataFile::readdatafile(std::string const& filename) const
    {
        std::ifstream ifs(filename);
        std::array<char, BUFSIZE> buf;
        std::vector<double> r_mesh, phiorrho;

        // �g�[�N������
        std::vector<std::string> tokens;

        for (auto i = 0;; i++) {
            using namespace boost::algorithm;

            ifs.getline(buf.data(), BUFSIZE);
            std::string line(buf.data());

            split(tokens, line, is_any_of(","), token_compress_on);

            // �����ꕶ�����ǂ߂Ȃ�������
            if (!ifs.gcount() && !i) {
                throw std::runtime_error("�f�[�^�t�@�C������ł��I");
            }
            else if (!ifs.gcount()) {
                r_mesh.shrink_to_fit();
                phiorrho.shrink_to_fit();

                return std::make_pair(r_mesh, phiorrho);
            }
            else if (tokens.size() != 2) {
                throw std::runtime_error("�f�[�^�t�@�C�����ُ�ł��I");
            }

            std::size_t index1, index2;

            r_mesh.push_back(std::stod(tokens[0], &index1));
            phiorrho.push_back(std::stod(tokens[1], &index2));

            if (tokens[0].size() != index1 || tokens[1].size() != index2) {
                throw std::runtime_error("�f�[�^�t�@�C�����ُ�ł��I");
            }
        }
    }
}
