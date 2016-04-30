/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#ifndef __READLINE_STATUS_H__
#define __READLINE_STATUS_H__

typedef enum readline_status_t readline_status_t;
enum readline_status_t
{
    readline_status_done,
    readline_status_error,
    readline_status_continue,
    readline_status_ctrl_c,
    readline_status_timed_out,
    readline_status_eof
}; 


#endif /* __READLINE_STATUS_H__ */
