#ifndef READER_H
#define READER_H

#include "task_executor/task_executor.h"

zox_aio_executor* create_reader(struct zox_aio_context*);

#endif