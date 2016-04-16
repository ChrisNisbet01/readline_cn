#ifndef __SPLIT_PATH_H__
#define __SPLIT_PATH_H__

#include <stddef.h>

int split_path(char const * const path, 
               char const * * const dir_part, 
               char const * * const file_part, 
               size_t * const completion_start_index);

#endif /* __SPLIT_PATH_H__ */
