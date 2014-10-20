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
 * \file   crawl_task_cmd.c
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
#include "crawler_task_cmd.h"
#include "crawler_utils.h"

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/************************* STRUCTURE/UNION DATA TYPES *************************/

/************************ STATIC FUNCTION PROTOTYPES **************************/
static CRAWL_RET_E crawl_handle_task_cmd_start (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr);

static CRAWL_RET_E crawl_handle_task_cmd_pause (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr);

static CRAWL_RET_E crawl_handle_task_cmd_stop (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr);

static CRAWL_RET_E crawl_handle_task_cmd_reset (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr);

/****************************** LOCAL FUNCTIONS *******************************/
CRAWL_RET_E crawl_handle_task_cmd (
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

   switch (px_cmd_hdr->ui_cmd)
   {
      case eCRAWL_CMD_START:
      {
         e_ret_val = crawl_handle_task_cmd_start (px_crawl_ctxt, px_cmd_hdr);
         break;
      }
      case eCRAWL_CMD_PAUSE:
      {
         e_ret_val = crawl_handle_task_cmd_pause (px_crawl_ctxt, px_cmd_hdr);
         break;
      }
      case eCRAWL_CMD_STOP:
      {
         e_ret_val = crawl_handle_task_cmd_stop (px_crawl_ctxt, px_cmd_hdr);
         break;
      }
      case eCRAWL_CMD_RESET:
      {
         e_ret_val = crawl_handle_task_cmd_reset (px_crawl_ctxt, px_cmd_hdr);
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

static SOCKMON_RET_E crawl_sockmon_active_sock_cbk (
   SOCKMON_SOCK_ACTIVITY_STATUS_E e_status,
   PAL_SOCK_HDL hl_sock_hdl,
   void *p_app_data)
{
   SOCKMON_RET_E e_sockmon_ret = eSOCKMON_RET_FAILURE;
   CRAWL_URL_CTXT_X *px_url_ctxt = NULL;

   if ((NULL == hl_sock_hdl) || (NULL == p_app_data))
   {
      CRAWL_LOG_LOW("Invalid Args");
      e_sockmon_ret = eSOCKMON_RET_INVALID_ARGS;
      goto CLEAN_RETURN;
   }

   if (e_SOCKMON_SOCK_ACTIVITY_STATUS_DATA_AVAILABLE == e_status)
   {
      px_url_ctxt = (CRAWL_URL_CTXT_X *) p_app_data;
      if ((NULL == px_url_ctxt->px_crawl_ctxt)
         || (NULL == px_url_ctxt->puc_url_str))
      {
         CRAWL_LOG_LOW("Invalid Args");
         e_sockmon_ret = eSOCKMON_RET_INVALID_ARGS;
         goto CLEAN_RETURN;
      }

      CRAWL_LOG_LOW("Activity on socket for domain '%s'",
         px_url_ctxt->puc_url_str);

      e_sockmon_ret = eSOCKMON_RET_SUCCESS;
   }

CLEAN_RETURN:
   return e_sockmon_ret;
}

static CRAWL_RET_E crawl_handle_task_cmd_start (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr)
{
   CRAWL_RET_E e_ret_val = eCRAWL_RET_FAILURE;
   HM_RET_E e_hm_ret = eHM_RET_FAILURE;
   SOCKMON_RET_E e_sockmon_ret = eSOCKMON_RET_FAILURE;
   PAL_RET_E e_pal_ret = ePAL_RET_FAILURE;
   CRAWL_CMD_START_DATA_X *px_start_data = NULL;
   uint8_t *puc_offset = NULL;
   SOCK_UTIL_HOST_INFO_X x_host_info = {0};
   SOCKMON_REGISTER_DATA_X x_register_data = {NULL};
   HM_NODE_DATA_X x_node_data = {eHM_KEY_TYPE_INVALID};
   CRAWL_URL_CTXT_X *px_url_ctxt = NULL;
   uint32_t ui_url_str_len = 0;

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

   px_start_data = (CRAWL_CMD_START_DATA_X *) px_cmd_hdr;
   puc_offset = (uint8_t *) px_start_data;
   puc_offset += (sizeof(*px_start_data));
   CRAWL_LOG_LOW("Got eCRAWL_CMD_START: %s", puc_offset);

   /*
    * Strip the URL sent to its base string and then check if it is present in
    * the hash map.
    */
   x_node_data.e_hm_key_type = eHM_KEY_TYPE_STRING;
   x_node_data.u_hm_key.puc_str_key = puc_offset;
   e_hm_ret = hm_search_node (px_crawl_ctxt->hl_url_ctxt_hm_hdl, &x_node_data);
   if (eHM_RET_HM_NODE_FOUND == e_hm_ret)
   {
      CRAWL_LOG_LOW("hm_search_node says URL: '%s' is already being crawled.",
         puc_offset);
      e_ret_val = eCRAWL_RET_SUCCESS;
      goto CLEAN_RETURN;
   }

   px_url_ctxt = pal_malloc (sizeof(CRAWL_URL_CTXT_X), NULL);
   if (NULL == px_url_ctxt)
   {
      CRAWL_LOG_LOW("pal_malloc failed");
      e_ret_val = eCRAWL_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   ui_url_str_len = pal_strlen (puc_offset) + 1;
   px_url_ctxt->puc_url_str = pal_malloc (ui_url_str_len, NULL);
   if (NULL == px_url_ctxt->puc_url_str)
   {
      CRAWL_LOG_LOW("pal_malloc failed");
      e_ret_val = eCRAWL_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   (void) pal_strncpy(px_url_ctxt->puc_url_str, puc_offset, ui_url_str_len);
   px_url_ctxt->hl_domain_hm = px_start_data->hl_domain_hm;
   px_url_ctxt->px_crawl_ctxt = px_crawl_ctxt;

   x_host_info.ui_bitmask |= eSOCK_UTIL_HOST_INFO_DNS_NAME_BM;
   x_host_info.puc_dns_name_str = puc_offset;
   e_pal_ret = tcp_connect_sock_create (&(px_url_ctxt->hl_sock_hdl),
      &x_host_info, CRAWL_HOST_CONNECT_TIMEOUT);
   if ((ePAL_RET_SUCCESS != e_pal_ret) || (NULL == px_url_ctxt->hl_sock_hdl))
   {
      CRAWL_LOG_LOW("tcp_connect_sock_create failed: %d", e_pal_ret);
      e_ret_val = eCRAWL_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   CRAWL_LOG_LOW("Connect to %s success", puc_offset);

   x_register_data.hl_sock_hdl = px_url_ctxt->hl_sock_hdl;
   x_register_data.fn_active_sock_cbk = crawl_sockmon_active_sock_cbk;
   x_register_data.p_app_data = px_url_ctxt;
   e_sockmon_ret = sockmon_register_sock (
      px_crawl_ctxt->x_init_params.hl_sockmon_hdl, &x_register_data);
   if (eSOCKMON_RET_SUCCESS != e_sockmon_ret)
   {
      CRAWL_LOG_LOW("tcp_connect_sock_create failed: %d", e_sockmon_ret);
      e_ret_val = eCRAWL_RET_RESOURCE_FAILURE;
      goto CLEAN_RETURN;
   }

   x_node_data.e_hm_key_type = eHM_KEY_TYPE_STRING;
   x_node_data.u_hm_key.puc_str_key = puc_offset;
   x_node_data.p_data = px_url_ctxt;
   x_node_data.ui_data_size = sizeof(*px_url_ctxt);
   e_hm_ret = hm_add_node (px_crawl_ctxt->hl_url_ctxt_hm_hdl, &x_node_data);
   if (eHM_RET_SUCCESS != e_hm_ret)
   {
      CRAWL_LOG_LOW("hm_add_node failed: %d", e_hm_ret);
      e_ret_val = eCRAWL_RET_SUCCESS;
   }
   else
   {
      e_ret_val = eCRAWL_RET_SUCCESS;
   }
CLEAN_RETURN:
   if (eCRAWL_RET_SUCCESS != e_ret_val)
   {
      if (NULL != px_url_ctxt)
      {
         if (NULL == px_url_ctxt->puc_url_str)
         {
            pal_free (px_url_ctxt->puc_url_str);
            px_url_ctxt->puc_url_str = NULL;
         }
         pal_free (px_url_ctxt);
         px_url_ctxt = NULL;
      }
   }
   return e_ret_val;
}

static CRAWL_RET_E crawl_handle_task_cmd_pause (
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

CLEAN_RETURN:
   return e_ret_val;
}

static CRAWL_RET_E crawl_handle_task_cmd_stop (
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

CLEAN_RETURN:
   return e_ret_val;
}

static CRAWL_RET_E crawl_handle_task_cmd_reset (
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

CLEAN_RETURN:
   return e_ret_val;
}
