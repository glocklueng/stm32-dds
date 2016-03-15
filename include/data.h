#ifndef _DATA_H
#define _DATA_H

#define MAX_DATA_SEGMENTS 20

struct binary_data
{
  char name[8];
  void* begin;
  void* end;
};

struct binary_data* new_data_segment(void);
struct binary_data* get_data_segment(const char*);
void free_data_segment(struct binary_data*);
void free_all_data_segments(void);
void delete_data_segment(const char*);

#endif /* _DATA_H */
