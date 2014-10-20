/*******************************************************************************
 *  Repository for C modules.
 *  Copyright (C) 2012 Sandeep Prakash
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 ******************************************************************************/

/*******************************************************************************
 * Copyright (c) 2013, Sandeep Prakash <123sandy@gmail.com>
 *
 * \file   exp_crawl.h
 *
 * \author sandeepprakash
 *
 * \date   Feb 22, 2013
 *
 * \brief
 *
 ******************************************************************************/

#ifndef __EXP_CRAWL_H__
#define __EXP_CRAWL_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/
typedef enum _CRAWL_RET_E
{
   eCRAWL_RET_SUCCESS                = 0x00000000,

   eCRAWL_RET_FAILURE                = 0x00000001,

   eCRAWL_RET_INVALID_ARGS           = 0x00000002,

   eCRAWL_RET_INVALID_HANDLE         = 0x00000003,

   eCRAWL_RET_RESOURCE_FAILURE       = 0x00000004,

   eCRAWL_RET_MAX
} CRAWL_RET_E;

typedef enum _CRAWL_CMD_E
{
   eCRAWL_CMD_INVALID   = 0x00000000,

   eCRAWL_CMD_START,

   eCRAWL_CMD_PAUSE,

   eCRAWL_CMD_STOP,

   eCRAWL_CMD_RESET,

   eCRAWL_CMD_MAX       = 0x0000FFFF
} CRAWL_CMD_E;

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/
typedef struct CRAWL_CTXT_X       *CRAWL_HDL;

typedef struct _CRAWL_CREATE_PARAMS_X
{
   SOCKMON_HDL hl_sockmon_hdl;
} CRAWL_CREATE_PARAMS_X;

typedef struct _CRAWL_CMD_HDR_X
{
   uint32_t ui_cmd;

   uint32_t ui_payload_len;

   uint8_t uca_reserved[56];
} CRAWL_CMD_HDR_X;

typedef struct _CRAWL_CMD_START_DATA_X
{
   CRAWL_CMD_HDR_X x_msg_hdr;

   HM_HDL hl_domain_hm;

   uint32_t ui_domain_buf_len;

   // uint8_t uca_domain_name[];
} CRAWL_CMD_START_DATA_X;

typedef struct _CRAWL_CMD_PAUSE_DATA_X
{
   CRAWL_CMD_HDR_X x_msg_hdr;
} CRAWL_CMD_PAUSE_DATA_X;

typedef struct _CRAWL_CMD_STOP_DATA_X
{
   CRAWL_CMD_HDR_X x_msg_hdr;
} CRAWL_CMD_STOP_DATA_X;

typedef struct _CRAWL_CMD_RESET_DATA_X
{
   CRAWL_CMD_HDR_X x_msg_hdr;
} CRAWL_CMD_RESET_DATA_X;

/***************************** FUNCTION PROTOTYPES ****************************/
CRAWL_RET_E crawl_init (
   CRAWL_HDL *phl_crawl_hdl,
   CRAWL_CREATE_PARAMS_X *px_create_params);

CRAWL_RET_E crawl_deinit (
   CRAWL_HDL hl_crawl_hdl);

CRAWL_RET_E crawl_send_cmd (
   CRAWL_HDL hl_crawl_hdl,
   CRAWL_CMD_HDR_X *px_cmd_hdr);

#ifdef   __cplusplus
}
#endif

#endif /* __EXP_CRAWL_H__ */
