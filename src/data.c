#include "data.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static struct binary_data bin_data_list[MAX_DATA_SEGMENTS];

struct binary_data*
new_data_segment()
{
  for (int i = 0; i < MAX_DATA_SEGMENTS; i++) {
    if (bin_data_list[i].name[0] != 0) {
      continue;
    }

    return bin_data_list + i;
  }

  /* we're out of unused data segments */
  return NULL;
}

struct binary_data*
get_data_segment(const char* id)
{
  for (int i = 0; i < MAX_DATA_SEGMENTS; i++) {
    if (strncmp(id, bin_data_list[i].name, 7) == 0) {
      return bin_data_list + i;
    }
  }

  /* nothing found */
  return NULL;
}

void
free_data_segment(struct binary_data* data)
{
  if (data->begin != NULL) {
    free(data->begin);
  }

  data->begin = NULL;
  data->end = NULL;
  for (int i = 0; i < 8; ++i) {
    data->name[i] = 0;
  }
}

void
free_all_data_segments()
{
  for (int i = 0; i < MAX_DATA_SEGMENTS; i++) {
    free_data_segment(bin_data_list + i);
  }
}

void
delete_data_segment(const char* id)
{
  struct binary_data* data = get_data_segment(id);
  if (data != NULL) {
    free_data_segment(data);
  }
}
