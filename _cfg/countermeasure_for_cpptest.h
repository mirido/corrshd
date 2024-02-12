#pragma once

/*
	Although the following definitions are just casts,
	it has a special name so that we can see they are countermeasure for C++Test.
*/

#ifdef __cplusplus
#define C_INT(x)				(static_cast<int>(x))
#else
#define C_INT(x)				((int)(x))
#endif

#ifdef __cplusplus
#define C_UINT(x)				(static_cast<unsigned>(x))
#else
#define C_UINT(x)				((unsigned)(x))
#endif

#ifdef __cplusplus
#define C_CHAR(x)				(static_cast<char>(x))
#else
#define C_CHAR(x)				((char)(x))
#endif

#ifdef __cplusplus
#define C_UCHAR(x)				(static_cast<unsigned char>(x))
#else
#define C_UCHAR(x)				((unsigned char)(x))
#endif

#ifdef __cplusplus
#define C_ULONG(x)				(static_cast<unsigned long>(x))
#else
#define C_ULONG(x)				((unsigned long)(x))
#endif

#ifdef __cplusplus
#define C_DBL(x)				(static_cast<double>(x))
#else
#define C_DBL(x)				((double)(x))
#endif

#ifdef __cplusplus
#define ZT(x)					(static_cast<size_t>(x))
#else
#define ZT(x)					((size_t)(x))
#endif

#ifdef __cplusplus
#define DW(x)					(static_cast<unsigned long>(x))
#else
#define DW(x)					((unsigned long)(x))
#endif

#ifdef __cplusplus
#define CHAR_TO_INT(x)		(static_cast<size_t>(static_cast<unsigned char>(x)))
#else
#define CHAR_TO_INT(x)		((int)(unsigned char)(x))
#endif

// Override somewhat rude OpenCV macros.
//#define CV_MAT_DEPTH(flags)     ((flags) & CV_MAT_DEPTH_MASK)
//#define CV_MAKETYPE(depth,cn) (CV_MAT_DEPTH(depth) + (((cn)-1) << CV_CN_SHIFT))
#undef CV_MAT_DEPTH
#undef CV_MAKETYPE
#define CV_MAT_DEPTH(flags)		(C_UINT((flags)) & C_UINT(CV_MAT_DEPTH_MASK))
#define CV_MAKETYPE(depth,cn)	(C_INT(CV_MAT_DEPTH(depth) + (C_UINT(((cn)-1)) << CV_CN_SHIFT)))

// Customizd cv::Scalar for integer components.
#define CVSCALAR(a, b, c, d)	(cv::Scalar((C_INT(a), C_INT(b), C_INT(c), C_INT(d))))
