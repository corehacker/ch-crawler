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
 * \file   crawl_cmd.h
 *
 * \author sandeepprakash
 *
 * \date   Apr 10, 2013
 *
 * \brief
 *
 ******************************************************************************/

#ifndef __CRAWL_CMD_H__
#define __CRAWL_CMD_H__

#ifdef  __cplusplus
extern  "C"
{
#endif

/********************************* CONSTANTS **********************************/

/*********************************** MACROS ***********************************/

/******************************** ENUMERATIONS ********************************/

/*********************** CLASS/STRUCTURE/UNION DATA TYPES *********************/

/***************************** FUNCTION PROTOTYPES ****************************/
CRAWL_RET_E crawl_handle_cmd_start (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr);

CRAWL_RET_E crawl_handle_cmd_pause (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr);

CRAWL_RET_E crawl_handle_cmd_stop (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr);

CRAWL_RET_E crawl_handle_cmd_reset (
   CRAWL_CTXT_X *px_crawl_ctxt,
   CRAWL_CMD_HDR_X *px_cmd_hdr);

#ifdef   __cplusplus
}
#endif

#endif /* __CRAWL_CMD_H__ */
