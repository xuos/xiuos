/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
 * @file ch376fs.h
 * @brief CH376 file system define
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2021.03.18
 */

#ifndef __INC_CH376FS_H__
#define __INC_CH376FS_H__

#include <xiuos.h>

#if defined(FS_VFS)
#include <iot-vfs.h>
#endif

#define PARAM_LEN 256

/* CH376 file system directory info */
struct Ch376FatDirInfo
{
	uint8	dir_name[11];
	uint8	dir_attr;
	uint8	dir_ntres;
	uint8	dir_crt_time_tenth;
	uint16	dir_crt_time;
	uint16	dir_crt_date;
	uint16	dir_lst_acc_date;
	uint16	dir_fst_clus_hi;
	uint16	dir_wrt_time;
	uint16	dir_wrt_date;
	uint16	dir_fst_clus_lo;
	uint32	dir_file_size;
};

#endif
