/*! \file readdatafile.cpp
    \brief ???????????????????????

    Copyright © 2015 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/

#include "readdatafile.h"
#include <array>                        // for std::array
#include <fstream>                      // for std::ifstream
#include <stdexcept>                    // for std::runtime_error
#include <boost/algorithm/string.hpp>   // for boost::algorithm

namespace getdata {
    ReadDataFile::mypair ReadDataFile::readdatafile(std::string const & filename) const
    {
        std::ifstream ifs(filename);
        std::array<char, BUFSIZE> buf;
        std::vector<double> r_mesh, phiorrho;

        // ??????
        std::vector<std::string> tokens;
        
        for (auto i = 0;; i++) {
            using namespace boost::algorithm;

            ifs.getline(buf.data(), BUFSIZE);
            std::string line(buf.data());

            split(tokens, line, is_any_of(","), token_compress_on);
                        
            // ?????????????
            if (!ifs.gcount() && !i) {
                throw std::runtime_error("???????????!");
            }
            else if (!ifs.gcount()) {
                r_mesh.shrink_to_fit();
                phiorrho.shrink_to_fit();

                return std::make_pair(r_mesh, phiorrho);
            }
            else if (tokens.size() != 2) {
                throw std::runtime_error("????????????!");
            }

            std::size_t index1, index2;

            r_mesh.push_back(std::stod(tokens[0], &index1));
            phiorrho.push_back(std::stod(tokens[1], &index2));

            if (tokens[0].size() != index1 || tokens[1].size() != index2) {
                throw std::runtime_error("????????????!");
            }
        }
    }
}
