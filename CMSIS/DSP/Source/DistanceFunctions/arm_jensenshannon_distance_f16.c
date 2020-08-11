
/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_jensenshannon_distance_f16.c
 * Description:  Jensen-Shannon distance between two vectors
 *
 *
 * Target Processor: Cortex-M cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2020 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dsp/distance_functions_f16.h"

#if defined(ARM_FLOAT16_SUPPORTED)

#include <limits.h>
#include <math.h>


/**
  @addtogroup FloatDist
  @{
 */

#if !defined(ARM_MATH_MVEF) || defined(ARM_MATH_AUTOVECTORIZE)
/// @private
__STATIC_INLINE float16_t rel_entr(float16_t x, float16_t y)
{
    return (x * logf(x / y));
}
#endif


#if defined(ARM_MATH_MVEF) && !defined(ARM_MATH_AUTOVECTORIZE)

#include "arm_helium_utils.h"
#include "arm_vec_math_f16.h"

float16_t arm_jensenshannon_distance_f16(const float16_t *pA,const float16_t *pB, uint32_t blockSize)
{
    uint32_t        blkCnt;
    float16_t       tmp;
    f16x8_t         a, b, t, tmpV, accumV;

    accumV = vdupq_n_f16(0.0f);

    blkCnt = blockSize >> 3;
    while (blkCnt > 0U) {
        a = vld1q(pA);
        b = vld1q(pB);

        t = vaddq(a, b);
        t = vmulq(t, 0.5f);

        tmpV = vmulq(a, vrecip_medprec_f16(t));
        tmpV = vlogq_f16(tmpV);
        accumV = vfmaq(accumV, a, tmpV);

        tmpV = vmulq_f16(b, vrecip_medprec_f16(t));
        tmpV = vlogq_f16(tmpV);
        accumV = vfmaq(accumV, b, tmpV);

        pA += 8;
        pB += 8;
        blkCnt--;
    }

    /*
     * tail
     * (will be merged thru tail predication)
     */
    blkCnt = blockSize & 7;
    if (blkCnt > 0U) {
        mve_pred16_t    p0 = vctp16q(blkCnt);

        a = vldrhq_z_f16(pA, p0);
        b = vldrhq_z_f16(pB, p0);

        t = vaddq(a, b);
        t = vmulq(t, 0.5f);

        tmpV = vmulq_f16(a, vrecip_medprec_f16(t));
        tmpV = vlogq_f16(tmpV);
        accumV = vfmaq_m_f16(accumV, a, tmpV, p0);

        tmpV = vmulq_f16(b, vrecip_medprec_f16(t));
        tmpV = vlogq_f16(tmpV);
        accumV = vfmaq_m_f16(accumV, b, tmpV, p0);

    }

    arm_sqrt_f16(vecAddAcrossF16Mve(accumV) / 2.0f, &tmp);
    return (tmp);
}

#else


/**
 * @brief        Jensen-Shannon distance between two vectors
 *
 * This function is assuming that elements of second vector are > 0
 * and 0 only when the corresponding element of first vector is 0.
 * Otherwise the result of the computation does not make sense
 * and for speed reasons, the cases returning NaN or Infinity are not
 * managed.
 *
 * When the function is computing x log (x / y) with x == 0 and y == 0,
 * it will compute the right result (0) but a division by zero will occur
 * and should be ignored in client code.
 *
 * @param[in]    pA         First vector
 * @param[in]    pB         Second vector
 * @param[in]    blockSize  vector length
 * @return distance
 *
 */


float16_t arm_jensenshannon_distance_f16(const float16_t *pA,const float16_t *pB, uint32_t blockSize)
{
    float16_t left, right,sum, result, tmp;
    uint32_t i;

    left = 0.0f; 
    right = 0.0f;
    for(i=0; i < blockSize; i++)
    {
      tmp = (pA[i] + pB[i]) / 2.0f;
      left  += rel_entr(pA[i], tmp);
      right += rel_entr(pB[i], tmp);
    }


    sum = left + right;
    arm_sqrt_f16(sum/2.0f, &result);
    return(result);

}

#endif /* defined(ARM_MATH_MVEF) && !defined(ARM_MATH_AUTOVECTORIZE) */

/**
 * @} end of FloatDist group
 */

#endif /* #if defined(ARM_FLOAT16_SUPPORTED) */ 
