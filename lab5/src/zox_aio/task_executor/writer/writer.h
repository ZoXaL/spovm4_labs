#ifndef WRITER_H
#define WRITER_H

#include "task_executor/task_executor.h"

zox_aio_executor* create_writer(struct zox_aio_context*);

#endif