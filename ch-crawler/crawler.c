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
 * \file   crawl.c
 *
 * \author sandeepprakash
 *
 * \date   Feb 22, 2013
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
#include "crawler_cmd.h"
#include "crawler_utils.h"

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/************************* STRUCTURE/UNION DATA TYPES *************************/

/************************ STATIC FUNCTION PROTOTYPES **************************/
static CRAWL_RET_E crawl_init_resources (
   CRAWL_CTXT_X *px_crawl_ctxt);

static CRAWL_RET_E crawl_deinit_resources (
   CRAWL_CTXT_X *px_crawl_ctxt);

/****************************** LOCAL FUNCTIONS *******************************/
CRAWL_RET_E crawl_init (
   CRAWL_HDL *phl_crawl_hdl,
   CRAWL_CREATE_PARAMS_X *px_create_params)
{
   CRAWL_RET_E e_ret_val = eCRAWL_RET_FAILURE;
   CRAWL_RET_E e_ret_val_pvt = eCRAWL_RET_FAILURE;
   CRAWL_CTXT_X *px_crawl_ctxt = NULL;

   if ((NULL == phl_crawl_hdl) || (NULL == px_create_params))
   {
      CRAWL_LOG_LOW("Invalid Args");
      e_ret_val = eCRAWL_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if (NULL == px_create_params->hl_sockmon_hdl)
   {
      CRAWL_LOG_LOW("Invalid Args");
      e_ret_val = eCRAWL_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_crawl_ctxt = pal_malloc (sizeof(CRAWL_CTXT_X), NULL);
   if (NULL == px_crawl_ctxt)
   {
      CRAWL_LOG_LOW("pal_malloc failed");
      e_ret_val = eCRAWL_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }
   (void) pal_memmove (&(px_crawl_ctxt->x_init_params), px_create_params,
      sizeof(px_crawl_ctxt->x_init_params));

   e_ret_val = crawl_init_resources(px_crawl_ctxt);
   if (eCRAWL_RET_SUCCESS != e_ret_val)
   {
      CRAWL_LOG_LOW("crawl_init_resources failed: %d", e_ret_val);

      e_ret_val_pvt = crawl_deinit_resources (px_crawl_ctxt);
      if (eCRAWL_RET_SUCCESS != e_ret_val_pvt)
      {
         CRAWL_LOG_LOW("crawl_deinit_resources failed: %d", e_ret_val_pvt);
      }
   }
   else
   {
      CRAWL_LOG_LOW("Crawler Init Done");
      *phl_crawl_hdl = (CRAWL_HDL) px_crawl_ctxt;
   }
CLEAN_RETURN:
   if (eCRAWL_RET_SUCCESS != e_ret_val)
   {
      if (NULL != px_crawl_ctxt)
      {
         pal_free (px_crawl_ctxt);
         px_crawl_ctxt = NULL;
      }
   }
   return e_ret_val;
}

static CRAWL_RET_E crawl_init_resources (
   CRAWL_CTXT_X *px_crawl_ctxt)
{
   CRAWL_RET_E e_ret_val = eCRAWL_RET_FAILURE;
   CRAWL_RET_E e_ret_val_pvt = eCRAWL_RET_FAILURE;
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;
   TASK_RET_E e_task_ret = eTASK_RET_FAILURE;
   TASK_CREATE_PARAM_X  x_task_params = {{0}};
   HM_INIT_PARAMS_X x_hm_params = {0};

   if (NULL == px_crawl_ctxt)
   {
      CRAWL_LOG_LOW("Invalid Args");
      e_ret_val = eCRAWL_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   x_hm_params.ui_hm_table_size = CRAWL_URL_HM_TABLE_SIZE;
   x_hm_params.e_hm_key_type = eHM_KEY_TYPE_STRING;
   e_hm_ret = hm_create (&(px_crawl_ctxt->hl_url_ctxt_hm_hdl), &x_hm_params);
   if ((eHM_RET_SUCCESS != e_hm_ret)
      || (NULL == px_crawl_ctxt->hl_url_ctxt_hm_hdl))
   {
      CRAWL_LOG_MED("hm_delete failed: %d", e_hm_ret);
      goto CLEAN_RETURN;
   }

   x_task_params.b_msgq_needed = true;
   x_task_params.ui_msgq_size = CRAWL_TASK_MSG_Q_SIZE;
   x_task_params.fn_task_routine = crawl_task;
   x_task_params.p_app_data = px_crawl_ctxt;
   (void) pal_strncpy (x_task_params.uca_task_name_str,
      (const uint8_t *) "Crawl Task", sizeof(x_task_params.uca_task_name_str));
   e_task_ret = task_create(&(px_crawl_ctxt->hl_task_hdl), &x_task_params);
   if (eTASK_RET_SUCCESS != e_task_ret)
   {
      e_ret_val = eCRAWL_RET_RESOURCE_FAILURE;
   }
   else
   {
      e_task_ret = task_start (px_crawl_ctxt->hl_task_hdl);
      if (eTASK_RET_SUCCESS != e_task_ret)
      {
         CRAWL_LOG_MED("task_start failed: %d", e_task_ret);
         e_task_ret = task_delete (px_crawl_ctxt->hl_task_hdl);
         if (eTASK_RET_SUCCESS != e_task_ret)
         {
            CRAWL_LOG_MED("task_delete failed: %d", e_task_ret);
         }
         px_crawl_ctxt->hl_task_hdl = NULL;
         e_ret_val = eCRAWL_RET_RESOURCE_FAILURE;
      }
      else
      {
         e_ret_val = eCRAWL_RET_SUCCESS;
      }
   }
CLEAN_RETURN:
   if (eCRAWL_RET_SUCCESS != e_ret_val)
   {
      e_ret_val_pvt = crawl_deinit_resources (px_crawl_ctxt);
      if (eCRAWL_RET_SUCCESS != e_ret_val_pvt)
      {
         CRAWL_LOG_LOW("crawl_deinit_resources failed: %d", e_ret_val_pvt);
      }
   }
   return e_ret_val;
}

static CRAWL_RET_E crawl_deinit_resources (
   CRAWL_CTXT_X *px_crawl_ctxt)
{
   CRAWL_RET_E e_ret_val = eCRAWL_RET_FAILURE;
   TASK_RET_E e_task_ret = eTASK_RET_FAILURE;
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;

   if (NULL == px_crawl_ctxt)
   {
      CRAWL_LOG_LOW("Invalid Args");
      e_ret_val = eCRAWL_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if (NULL != px_crawl_ctxt->hl_task_hdl)
   {
      e_task_ret = task_delete (px_crawl_ctxt->hl_task_hdl);
      if (eTASK_RET_SUCCESS != e_task_ret)
      {
         CRAWL_LOG_MED("task_delete failed: %d", e_task_ret);
      }
      px_crawl_ctxt->hl_task_hdl = NULL;
   }

   if (NULL != px_crawl_ctxt->hl_url_ctxt_hm_hdl)
   {
      e_hm_ret = hm_delete (px_crawl_ctxt->hl_url_ctxt_hm_hdl);
      if (eHM_RET_SUCCESS != e_hm_ret)
      {
         CRAWL_LOG_MED("hm_delete failed: %d", e_hm_ret);
      }
      px_crawl_ctxt->hl_url_ctxt_hm_hdl = NULL;
   }
   e_ret_val = eCRAWL_RET_SUCCESS;
CLEAN_RETURN:
   return e_ret_val;
}

CRAWL_RET_E crawl_deinit (
   CRAWL_HDL hl_crawl_hdl)
{
   CRAWL_RET_E e_ret_val = eCRAWL_RET_FAILURE;
   CRAWL_CTXT_X *px_crawl_ctxt = NULL;

   if (NULL == hl_crawl_hdl)
   {
      CRAWL_LOG_LOW("Invalid Args");
      e_ret_val = eCRAWL_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_crawl_ctxt = (CRAWL_CTXT_X *) hl_crawl_hdl;

   e_ret_val = crawl_deinit_resources (px_crawl_ctxt);
   if (eCRAWL_RET_SUCCESS != e_ret_val)
   {
      CRAWL_LOG_LOW("crawl_deinit_resources failed: %d", e_ret_val);
   }

   if (NULL != px_crawl_ctxt)
   {
      pal_free (px_crawl_ctxt);
      px_crawl_ctxt = NULL;
   }

CLEAN_RETURN:
   return e_ret_val;
}

CRAWL_RET_E crawl_send_cmd (
   CRAWL_HDL hl_crawl_hdl,
   CRAWL_CMD_HDR_X *px_cmd_hdr)
{
   CRAWL_RET_E e_ret_val = eCRAWL_RET_FAILURE;
   CRAWL_CTXT_X *px_crawl_ctxt = NULL;

   if ((NULL == hl_crawl_hdl) || (NULL == px_cmd_hdr))
   {
      CRAWL_LOG_LOW("Invalid Args");
      e_ret_val = eCRAWL_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   px_crawl_ctxt = (CRAWL_CTXT_X *) hl_crawl_hdl;
   switch (px_cmd_hdr->ui_cmd)
   {
      case eCRAWL_CMD_START:
      {
         e_ret_val = crawl_handle_cmd_start (px_crawl_ctxt, px_cmd_hdr);
         break;
      }
      case eCRAWL_CMD_PAUSE:
      {
         e_ret_val = crawl_handle_cmd_pause (px_crawl_ctxt, px_cmd_hdr);
         break;
      }
      case eCRAWL_CMD_STOP:
      {
         e_ret_val = crawl_handle_cmd_stop (px_crawl_ctxt, px_cmd_hdr);
         break;
      }
      case eCRAWL_CMD_RESET:
      {
         e_ret_val = crawl_handle_cmd_reset (px_crawl_ctxt, px_cmd_hdr);
         break;
      }
      default:
      {
         CRAWL_LOG_LOW("Invalid Args");
         e_ret_val = eCRAWL_RET_INVALID_ARGS;
         break;
      }
   }
   if (eCRAWL_RET_SUCCESS != e_ret_val)
   {
      CRAWL_LOG_LOW("Handling %s failed: %d",
         crawl_get_cmd_str (px_cmd_hdr->ui_cmd), e_ret_val);
   }
CLEAN_RETURN:
   return e_ret_val;
}
