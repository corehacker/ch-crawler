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
 * \file   crawl_private.h
 *
 * \author sandeepprakash
 *
 * \date   Feb 22, 2013
 *
 * \brief
 *
 ******************************************************************************/

#ifndef __CRAWL_PRIVATE_H__
#define __CRAWL_PRIVATE_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/
#define CRAWL_TASK_MSG_Q_SIZE                   (10)

#define CRAWL_LOG_STR                           "CRAW"

#define CRAWL_TASK_Q_WAIT_TIMEOUT               (1 * 1000)

#define CRAWL_URL_MSGQ_SIZE                     (MSGQ_MAX_MSGQ_SIZE)

#define CRAWL_URL_HM_TABLE_SIZE                 (1 * 1024)

#define CRAWL_HOST_CONNECT_TIMEOUT              (5 * 1000)

#define CRAWL_LOG_LOW(format,...)                                              \
do                                                                            \
{                                                                             \
   LOG_LOW (CRAWL_LOG_STR,__FILE__,__FUNCTION__,__LINE__,format,            \
      ##__VA_ARGS__);                                                         \
} while (0)

#define CRAWL_LOG_MED(format,...)                                              \
do                                                                            \
{                                                                             \
   LOG_MED (CRAWL_LOG_STR,__FILE__,__FUNCTION__,__LINE__,format,            \
      ##__VA_ARGS__);                                                         \
} while (0)

#define CRAWL_LOG_HIGH(format,...)                                             \
do                                                                            \
{                                                                             \
   LOG_HIGH (CRAWL_LOG_STR,__FILE__,__FUNCTION__,__LINE__,format,           \
      ##__VA_ARGS__);                                                         \
} while (0)

#define CRAWL_LOG_FULL(format,...)                                             \
do                                                                            \
{                                                                             \
   LOG_FULL (CRAWL_LOG_STR,__FILE__,__FUNCTION__,__LINE__,format,           \
      ##__VA_ARGS__);                                                         \
} while (0)

/******************************** ENUMERATIONS ********************************/
typedef enum _CRAWL_STATE_E
{
   eCRAWL_STATE_INVALID = 0x00000000,

   eCRAWL_STATE_IDLE,

   eCRAWL_STATE_CRAWL,

   eCRAWL_STATE_PAUSE,

   eCRAWL_STATE_MAX
} CRAWL_STATE_E;

typedef enum _CRAWL_INTERNAL_CMD_E
{
   eCRAWL_INTERNAL_CMD_INVALID = eCRAWL_CMD_MAX + 1,

   eCRAWL_INTERNAL_CMD_MAX
} CRAWL_INTERNAL_CMD_E;

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/
typedef struct _CRAWL_CTXT_X
{
   CRAWL_CREATE_PARAMS_X x_init_params;

   TASK_HDL hl_task_hdl;

   MSGQ_HDL hl_url_msgq_hdl;

   HM_HDL hl_url_ctxt_hm_hdl;

   PAL_SOCK_HDL hl_crawl_sock_hdl;

   CRAWL_STATE_E e_state;
} CRAWL_CTXT_X;

typedef struct _CRAWL_URL_CTXT_X
{
   CRAWL_CTXT_X *px_crawl_ctxt;

   PAL_SOCK_HDL hl_sock_hdl;

   HM_HDL hl_domain_hm;

   uint8_t *puc_url_str;
} CRAWL_URL_CTXT_X;

/***************************** FUNCTION PROTOTYPES ****************************/

#ifdef   __cplusplus
}
#endif

#endif /* __CRAWL_PRIVATE_H__ */
