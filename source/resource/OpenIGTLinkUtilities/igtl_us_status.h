/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.
                 
Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.
                 
CustusX is released under a BSD 3-Clause license.
                 
See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/
/*
 * igtl_us_status.h
 *
 *  Created on: Apr 23, 2012
 *      Author: olevs
 */

#ifndef IGTL_US_STATUS_H_
#define IGTL_US_STATUS_H_

#include "igtl_win32header.h"
#include "igtl_types.h"
#include "igtl_win32header.h"

// 8 * 6 + 4 = 52
#define IGTL_US_STATUS_HEADER_SIZE 52 // Calculate number of bytes

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)     /* For 1-byte boundary in memroy */

typedef struct {
	igtl_float64 originX;
	igtl_float64 originY;
	igtl_float64 originZ;
	igtl_float64 depthStart;
	igtl_float64 depthEnd;
	igtl_float64 width;
	igtl_int32 probeType;
//	igtl_string dataFormat;
} igtl_us_status_message;

#pragma pack()

/*
 * String data size
 *
 * This function calculates size of the pixel array, which will be
 * transferred with the specified header.
 */

//igtl_uint32 igtl_export igtl_us_status_get_length(igtl_us_status_header * header);

/*
 * Byte order conversion
 *
 * This function converts endianness from host byte order to network byte order,
 * or vice versa.
 * NOTE: It is developer's responsibility to have the string body with BOM
 * (byte order mark) or in big endian ordrer.
 */

void igtl_export igtl_us_status_convert_byte_order(igtl_us_status_message * message);

/*
 * CRC calculation
 *
 * This function calculates CRC of image data body including header
 * and array of pixel data.
 *
 */

igtl_uint64 igtl_export igtl_us_status_get_crc(igtl_us_status_message * message);

#ifdef __cplusplus
}
#endif

#endif /* IGTL_US_STATUS_H_ */
