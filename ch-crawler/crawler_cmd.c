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
 * \file   crawl_cmd.c
 *
 * \author sandeepprakash
 *
 * \date   Apr 10, 2013
 *
 * \brief
 *
 ******************************************************************************/

/********************************** INCLUDES **********************************/
#include <ch-pal/exp_pal.h>
#include <ch-utils/exp_list.h>
#include <ch-utils/exp_q.h>
#include <ch-utils/exp_hashmap.h>
#include <ch-utils/exp_msgq.h>
#include <ch-utils/exp_task.h>
#include <ch-utils/exp_sock_utils.h>
#include <ch-sockmon/exp_sockmon.h>

#include <ch-crawler/exp_crawler.h>
#include "crawler_private.h"

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/************************* STRUCTURE/UNION DATA TYPES *************************/

/************************ STATIC FUNCTION PROTOTYPES **************************/
static CRAWL_RET_E crawl_post_cmd_to_task_q (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr);

/****************************** LOCAL FUNCTIONS *******************************/
CRAWL_RET_E crawl_handle_cmd_start (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr)
{
   CRAWL_RET_E e_ret_val = eCRAWL_RET_FAILURE;
   CRAWL_CMD_START_DATA_X *px_start_data = NULL;

   if ((NULL == px_crawl_ctxt) || (NULL == px_cmd_hdr))
   {
      CRAWL_LOG_LOW("Invalid Args");
      e_ret_val = eCRAWL_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if (px_cmd_hdr->ui_payload_len
      < (sizeof(*px_start_data) - sizeof(*px_cmd_hdr)))
   {
      CRAWL_LOG_LOW("Invalid Args");
      e_ret_val = eCRAWL_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_start_data = (CRAWL_CMD_START_DATA_X *) px_cmd_hdr;
   if ((NULL == px_start_data->hl_domain_hm)
      || (0 == px_start_data->ui_domain_buf_len))
   {
      CRAWL_LOG_LOW("Invalid Args");
      e_ret_val = eCRAWL_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   e_ret_val = crawl_post_cmd_to_task_q (px_crawl_ctxt, px_cmd_hdr);
   if (eCRAWL_RET_SUCCESS != e_ret_val)
   {
      CRAWL_LOG_LOW("crawl_post_cmd_to_task_q failed: %d", e_ret_val);
   }
CLEAN_RETURN:
   return e_ret_val;
}

CRAWL_RET_E crawl_handle_cmd_pause (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr)
{
   CRAWL_RET_E e_ret_val = eCRAWL_RET_FAILURE;

   if ((NULL == px_crawl_ctxt) || (NULL == px_cmd_hdr))
   {
      CRAWL_LOG_LOW("Invalid Args");
      e_ret_val = eCRAWL_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   e_ret_val = crawl_post_cmd_to_task_q (px_crawl_ctxt, px_cmd_hdr);
   if (eCRAWL_RET_SUCCESS != e_ret_val)
   {
      CRAWL_LOG_LOW("crawl_post_cmd_to_task_q failed: %d", e_ret_val);
   }
CLEAN_RETURN:
   return e_ret_val;
}

CRAWL_RET_E crawl_handle_cmd_stop (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr)
{
   CRAWL_RET_E e_ret_val = eCRAWL_RET_FAILURE;

   if ((NULL == px_crawl_ctxt) || (NULL == px_cmd_hdr))
   {
      CRAWL_LOG_LOW("Invalid Args");
      e_ret_val = eCRAWL_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   e_ret_val = crawl_post_cmd_to_task_q (px_crawl_ctxt, px_cmd_hdr);
   if (eCRAWL_RET_SUCCESS != e_ret_val)
   {
      CRAWL_LOG_LOW("crawl_post_cmd_to_task_q failed: %d", e_ret_val);
   }
CLEAN_RETURN:
   return e_ret_val;
}

CRAWL_RET_E crawl_handle_cmd_reset (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr)
{
   CRAWL_RET_E e_ret_val = eCRAWL_RET_FAILURE;

   if ((NULL == px_crawl_ctxt) || (NULL == px_cmd_hdr))
   {
      CRAWL_LOG_LOW("Invalid Args");
      e_ret_val = eCRAWL_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   e_ret_val = crawl_post_cmd_to_task_q (px_crawl_ctxt, px_cmd_hdr);
   if (eCRAWL_RET_SUCCESS != e_ret_val)
   {
      CRAWL_LOG_LOW("crawl_post_cmd_to_task_q failed: %d", e_ret_val);
   }
CLEAN_RETURN:
   return e_ret_val;
}

static CRAWL_RET_E crawl_post_cmd_to_task_q (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr)
{
   CRAWL_RET_E e_ret_val = eCRAWL_RET_FAILURE;
   TASK_RET_E e_task_ret = eTASK_RET_FAILURE;
   MSGQ_DATA_X x_data = {NULL};

   if ((NULL == px_crawl_ctxt) || (NULL == px_cmd_hdr))
   {
      CRAWL_LOG_LOW("Invalid Args");
      e_ret_val = eCRAWL_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   x_data.p_data = px_cmd_hdr;
   x_data.ui_data_size = sizeof(*px_cmd_hdr) + px_cmd_hdr->ui_payload_len;
   e_task_ret = task_add_msg_to_q (px_crawl_ctxt->hl_task_hdl, &x_data,
      CRAWL_TASK_Q_WAIT_TIMEOUT);
   if (eTASK_RET_SUCCESS != e_task_ret)
   {
      CRAWL_LOG_LOW ("task_add_msg_to_q failed: %d", e_task_ret);
      e_ret_val = eCRAWL_RET_RESOURCE_FAILURE;
   }
   else
   {
      e_ret_val = eCRAWL_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_ret_val;
}
