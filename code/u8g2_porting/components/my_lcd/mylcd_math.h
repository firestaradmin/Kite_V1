/**
 * @file mylcd_math.h
 * @author @firestaradmin
 * @brief What is rational is actual and what is actual is rational.
 * @version v0.0.1
 * @date 2024/07/18
 * 
 * @copyright Copyright (c) 2024 LXG 
 * 
 * @history:
 *  - 2024/07/18: v0.0.1 Initial Version
 *  - ----/--/--: --
 */


#ifndef __LCD_MATH_H__
#define __LCD_MATH_H__


/*********************
 *      DEFINES
 *********************/
#define UG_MATH_MIN(a, b) ((a) < (b) ? (a) : (b))
#define UG_MATH_MIN3(a, b, c) (UG_MATH_MIN(UG_MATH_MIN(a,b), c))
#define UG_MATH_MIN4(a, b, c, d) (UG_MATH_MIN(UG_MATH_MIN(a,b), UG_MATH_MIN(c,d)))

#define UG_MATH_MAX(a, b) ((a) > (b) ? (a) : (b))
#define UG_MATH_MAX3(a, b, c) (UG_MATH_MAX(UG_MATH_MAX(a,b), c))
#define UG_MATH_MAX4(a, b, c, d) (UG_MATH_MAX(UG_MATH_MAX(a,b), UG_MATH_MAX(c,d)))

#define UG_MATH_ABS(x) ((x) > 0 ? (x) : (-(x)))

#define UG_MATH_UDIV255(x) ((uint32_t)((uint32_t) (x) * 0x8081) >> 0x17)

#define UG_IS_SIGNED(t) (((t)(-1)) < ((t) 0))
#define UG_UMAX_OF(t) (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | (0xFULL << ((sizeof(t) * 8ULL) - 4ULL)))
#define UG_SMAX_OF(t) (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | (0x7ULL << ((sizeof(t) * 8ULL) - 4ULL)))
#define UG_MAX_OF(t) ((unsigned long) (UG_IS_SIGNED(t) ? UG_SMAX_OF(t) : UG_UMAX_OF(t)))

#define UG_TRIGO_SIN_MAX 32767
#define UG_TRIGO_SHIFT 15 /**<  >> UG_TRIGO_SHIFT to normalize*/

#define UG_BEZIER_VAL_MAX 1024 /**< Max time in Bezier functions (not [0..1] to use integers) */
#define UG_BEZIER_VAL_SHIFT 10 /**< log2(UG_BEZIER_VAL_MAX): used to normalize up scaled values*/

#define SWAP(a, b) { uint16_t t = a; a = b; b = t; }

#endif // !__LCD_MATH_H__
