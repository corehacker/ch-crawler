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
 * \file   crawl_util.c
 *
 * \author sandeepprakash
 *
 * \date   Apr 19, 2013
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
#include "crawler_utils.h"

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/************************* STRUCTURE/UNION DATA TYPES *************************/

/************************ STATIC FUNCTION PROTOTYPES **************************/

/****************************** LOCAL FUNCTIONS *******************************/
uint8_t *crawl_get_cmd_str (
   uint32_t ui_cmd)
{
   uint8_t *puc_cmd_str = NULL;
   switch (ui_cmd)
   {
      case eCRAWL_CMD_START:
      {
         puc_cmd_str = PAL_STR("eCRAWL_CMD_START");
         break;
      }
      case eCRAWL_CMD_PAUSE:
      {
         puc_cmd_str = PAL_STR("eCRAWL_CMD_PAUSE");
         break;
      }
      case eCRAWL_CMD_STOP:
      {
         puc_cmd_str = PAL_STR("eCRAWL_CMD_STOP");
         break;
      }
      case eCRAWL_CMD_RESET:
      {
         puc_cmd_str = PAL_STR("eCRAWL_CMD_RESET");
         break;
      }
      default:
      {
         puc_cmd_str = PAL_STR("eCRAWL_CMD_INVALID");
         break;
      }
   }
   return puc_cmd_str;
}
