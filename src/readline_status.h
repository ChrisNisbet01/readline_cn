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
