/*! \file deleter.h
    \brief gsl_interp_accelとgsl_splineのデリータを宣言・定義したヘッダファイル

    Copyright © 2015 @dc1394 All Rights Reserved.
    This software is released under the BSD 2-Clause License.
*/


#ifndef _DELETER_H_
#define _DELETER_H_

#pragma once

#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

namespace getdata {
    //! A function.
    /*!
        gsl_interp_accelへのポインタを解放するラムダ式
        \param acc gsl_interp_accelへのポインタ
    */
    static auto const gsl_interp_accel_deleter = [](gsl_interp_accel * acc) {
        gsl_interp_accel_free(acc);
    };

    //! A function.
    /*!
        gsl_splineへのポインタを解放するラムダ式
        \param spline gsl_splineへのポインタ
    */
    static auto const gsl_spline_deleter = [](gsl_spline * spline) {
        gsl_spline_free(spline);
    };
}

#endif  // _DELETER_H_
