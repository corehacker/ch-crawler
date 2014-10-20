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
 * \file   crawl_task.c
 *
 * \author sandeepprakash
 *
 * \date   Mar 16, 2013
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
#include "crawler_task.h"
#include "crawler_task_cmd.h"

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/************************* STRUCTURE/UNION DATA TYPES *************************/

/************************ STATIC FUNCTION PROTOTYPES **************************/
static CRAWL_RET_E crawl_task_init (
   CRAWL_CTXT_X *px_crawl_ctxt);

static CRAWL_RET_E crawl_task_deinit (
   CRAWL_CTXT_X *px_crawl_ctxt);

/****************************** LOCAL FUNCTIONS *******************************/
void *crawl_task(
   void *p_thread_args)
{
   CRAWL_RET_E e_ret_val = eCRAWL_RET_FAILURE;
   TASK_RET_E e_task_ret = eTASK_RET_FAILURE;
   CRAWL_CTXT_X *px_crawl_ctxt = NULL;
   MSGQ_DATA_X x_data = {NULL};
   CRAWL_CMD_HDR_X *px_cmd_hdr = NULL;

   if (NULL == p_thread_args)
   {
      goto CLEAN_RETURN;
   }

   px_crawl_ctxt = (CRAWL_CTXT_X *) p_thread_args;

   e_ret_val = crawl_task_init (px_crawl_ctxt);
   if (eCRAWL_RET_SUCCESS != e_ret_val)
   {
      CRAWL_LOG_LOW("crawl_task_init failed: %d", e_ret_val);
      goto CLEAN_RETURN;
   }

   while (task_is_in_loop (px_crawl_ctxt->hl_task_hdl))
   {
      e_task_ret = task_get_msg_from_q (px_crawl_ctxt->hl_task_hdl, &x_data,
         CRAWL_TASK_Q_WAIT_TIMEOUT);
      if ((eTASK_RET_SUCCESS == e_task_ret) && (NULL != x_data.p_data)
         && (0 != x_data.ui_data_size))
      {
         px_cmd_hdr = x_data.p_data;
         e_ret_val = crawl_handle_task_cmd (px_crawl_ctxt, px_cmd_hdr);
         if (eCRAWL_RET_SUCCESS != e_ret_val)
         {
            CRAWL_LOG_LOW("crawl_handle_task_cmd failed: %d", e_ret_val);
         }
      }
      CRAWL_LOG_HIGH("Crawler Task Active");
   }

   e_ret_val = crawl_task_deinit (px_crawl_ctxt);
   if (eCRAWL_RET_SUCCESS != e_ret_val)
   {
      CRAWL_LOG_LOW("crawl_task_deinit failed: %d", e_ret_val);
   }
CLEAN_RETURN:
   e_task_ret = task_notify_exit (px_crawl_ctxt->hl_task_hdl);
   if (eTASK_RET_SUCCESS != e_task_ret)
   {
      CRAWL_LOG_MED("task_notify_exit failed: %d", e_task_ret);
   }
   else
   {
      CRAWL_LOG_HIGH("task_notify_exit success");
   }
   return p_thread_args;
}

static CRAWL_RET_E crawl_task_init (
   CRAWL_CTXT_X *px_crawl_ctxt)
{
   CRAWL_RET_E e_ret_val = eCRAWL_RET_FAILURE;
   MSGQ_RET_E e_msgq_ret = eMSGQ_RET_FAILURE;
   MSGQ_INIT_PARAMS_X x_msgq_param = { 0 };

   if (NULL == px_crawl_ctxt)
   {
      CRAWL_LOG_LOW("Invalid Args");
      goto CLEAN_RETURN;
   }

   x_msgq_param.ui_msgq_size = CRAWL_URL_MSGQ_SIZE;
   e_msgq_ret = msgq_init (&(px_crawl_ctxt->hl_url_msgq_hdl), &x_msgq_param);
   if ((eMSGQ_RET_SUCCESS != e_msgq_ret)
      || (NULL == px_crawl_ctxt->hl_url_msgq_hdl))
   {
      CRAWL_LOG_MED("msgq_init failed: %d", e_msgq_ret);
      e_ret_val = eCRAWL_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }
   else
   {
      CRAWL_LOG_MED("msgq_init success");
      e_ret_val = eCRAWL_RET_SUCCESS;
   }
CLEAN_RETURN:
   return e_ret_val;
}

static CRAWL_RET_E crawl_task_deinit (
   CRAWL_CTXT_X *px_crawl_ctxt)
{
   CRAWL_RET_E e_ret_val = eCRAWL_RET_FAILURE;
   MSGQ_RET_E e_msgq_ret = eMSGQ_RET_FAILURE;

   if (NULL == px_crawl_ctxt)
   {
      CRAWL_LOG_LOW("Invalid Args");
      goto CLEAN_RETURN;
   }

   if (NULL != px_crawl_ctxt->hl_url_msgq_hdl)
   {
      e_msgq_ret = msgq_deinit (px_crawl_ctxt->hl_url_msgq_hdl);
      if (eMSGQ_RET_SUCCESS != e_msgq_ret)
      {
         CRAWL_LOG_MED ("msgq_deinit failed: %d", e_msgq_ret);
      }
      px_crawl_ctxt->hl_url_msgq_hdl = NULL;
   }

   e_ret_val = eCRAWL_RET_SUCCESS;
CLEAN_RETURN:
   return e_ret_val;
}
