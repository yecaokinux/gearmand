/* Gearman server and library
 * Copyright (C) 2008 Brian Aker, Eric Day
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 */

/**
 * @file
 * @brief Job Definitions
 */

#include "common.h"

/**
 * @addtogroup gearman_job_static Static Job Declarations
 * @ingroup gearman_job
 * @{
 */

/**
 * Send a packet for a job.
 */
static gearman_return_t _job_send(gearman_job_st *job);

/** @} */

/*
 * Public Definitions
 */

gearman_return_t gearman_job_send_data(gearman_job_st *job, const void *data,
                                       size_t data_size)
{
  gearman_return_t ret;
  const void *args[2];
  size_t args_size[2];

  if (!(job->options & GEARMAN_JOB_WORK_IN_USE))
  {
    args[0]= job->assigned.arg[0];
    args_size[0]= job->assigned.arg_size[0];
    args[1]= data;
    args_size[1]= data_size;
    ret= gearman_add_packet_args(job->worker->gearman, &(job->work),
                                 GEARMAN_MAGIC_REQUEST,
                                 GEARMAN_COMMAND_WORK_DATA,
                                 args, args_size, 2);
    if (ret != GEARMAN_SUCCESS)
      return ret;

    job->options|= GEARMAN_JOB_WORK_IN_USE;
  }

  return _job_send(job);
}

gearman_return_t gearman_job_send_warning(gearman_job_st *job,
                                          const void *warning,
                                          size_t warning_size)
{
  gearman_return_t ret;
  const void *args[2];
  size_t args_size[2];

  if (!(job->options & GEARMAN_JOB_WORK_IN_USE))
  {
    args[0]= job->assigned.arg[0];
    args_size[0]= job->assigned.arg_size[0];
    args[1]= warning;
    args_size[1]= warning_size;
    ret= gearman_add_packet_args(job->worker->gearman, &(job->work),
                                 GEARMAN_MAGIC_REQUEST,
                                 GEARMAN_COMMAND_WORK_WARNING,
                                 args, args_size, 2);
    if (ret != GEARMAN_SUCCESS)
      return ret;

    job->options|= GEARMAN_JOB_WORK_IN_USE;
  }

  return _job_send(job);
}

gearman_return_t gearman_job_send_status(gearman_job_st *job,
                                         uint32_t numerator,
                                         uint32_t denominator)
{
  gearman_return_t ret;
  char numerator_string[12];
  char denominator_string[12];
  const void *args[3];
  size_t args_size[3];

  if (!(job->options & GEARMAN_JOB_WORK_IN_USE))
  {
    snprintf(numerator_string, 12, "%u", numerator);
    snprintf(denominator_string, 12, "%u", denominator);

    args[0]= job->assigned.arg[0];
    args_size[0]= job->assigned.arg_size[0];
    args[1]= numerator_string;
    args_size[1]= strlen(numerator_string) + 1;
    args[2]= denominator_string;
    args_size[2]= strlen(denominator_string);
    ret= gearman_add_packet_args(job->worker->gearman, &(job->work),
                                 GEARMAN_MAGIC_REQUEST,
                                 GEARMAN_COMMAND_WORK_STATUS,
                                 args, args_size, 3);
    if (ret != GEARMAN_SUCCESS)
      return ret;

    job->options|= GEARMAN_JOB_WORK_IN_USE;
  }

  return _job_send(job);
}

gearman_return_t gearman_job_send_complete(gearman_job_st *job,
                                           const void *result,
                                           size_t result_size)
{
  gearman_return_t ret;
  const void *args[2];
  size_t args_size[2];

  if (job->options & GEARMAN_JOB_FINISHED)
    return GEARMAN_SUCCESS;

  if (!(job->options & GEARMAN_JOB_WORK_IN_USE))
  {
    args[0]= job->assigned.arg[0];
    args_size[0]= job->assigned.arg_size[0];
    args[1]= result;
    args_size[1]= result_size;
    ret= gearman_add_packet_args(job->worker->gearman, &(job->work),
                                 GEARMAN_MAGIC_REQUEST,
                                 GEARMAN_COMMAND_WORK_COMPLETE,
                                 args, args_size, 2);
    if (ret != GEARMAN_SUCCESS)
      return ret;

    job->options|= GEARMAN_JOB_WORK_IN_USE;
  }

  ret= _job_send(job);
  if (ret != GEARMAN_SUCCESS)
    return ret;

  job->options|= GEARMAN_JOB_FINISHED;
  return GEARMAN_SUCCESS;
}

gearman_return_t gearman_job_send_exception(gearman_job_st *job,
                                            const void *exception,
                                            size_t exception_size)
{
  gearman_return_t ret;
  const void *args[2];
  size_t args_size[2];

  if (!(job->options & GEARMAN_JOB_WORK_IN_USE))
  {
    args[0]= job->assigned.arg[0];
    args_size[0]= job->assigned.arg_size[0];
    args[1]= exception;
    args_size[1]= exception_size;
    ret= gearman_add_packet_args(job->worker->gearman, &(job->work),
                                 GEARMAN_MAGIC_REQUEST,
                                 GEARMAN_COMMAND_WORK_EXCEPTION,
                                 args, args_size, 2);
    if (ret != GEARMAN_SUCCESS)
      return ret;

    job->options|= GEARMAN_JOB_WORK_IN_USE;
  }

  return _job_send(job);
}

gearman_return_t gearman_job_send_fail(gearman_job_st *job)
{
  gearman_return_t ret;
  const void *args[1];
  size_t args_size[1];

  if (job->options & GEARMAN_JOB_FINISHED)
    return GEARMAN_SUCCESS;

  if (!(job->options & GEARMAN_JOB_WORK_IN_USE))
  {
    args[0]= job->assigned.arg[0];
    args_size[0]= job->assigned.arg_size[0] - 1;
    ret= gearman_add_packet_args(job->worker->gearman, &(job->work),
                                 GEARMAN_MAGIC_REQUEST,
                                 GEARMAN_COMMAND_WORK_FAIL,
                                 args, args_size, 2);
    if (ret != GEARMAN_SUCCESS)
      return ret;

    job->options|= GEARMAN_JOB_WORK_IN_USE;
  }

  ret= _job_send(job);
  if (ret != GEARMAN_SUCCESS)
    return ret;

  job->options|= GEARMAN_JOB_FINISHED;
  return GEARMAN_SUCCESS;
}

char *gearman_job_handle(const gearman_job_st *job)
{
  return (char *)job->assigned.arg[0];
}

char *gearman_job_function_name(const gearman_job_st *job)
{
  return (char *)job->assigned.arg[1];
}

const char *gearman_job_unique(const gearman_job_st *job)
{
  if (job->assigned.command == GEARMAN_COMMAND_JOB_ASSIGN_UNIQ)
    return (const char *)job->assigned.arg[2];
  return "";
}

const void *gearman_job_workload(const gearman_job_st *job)
{
  return job->assigned.data;
}

size_t gearman_job_workload_size(const gearman_job_st *job)
{
  return job->assigned.data_size;
}

void *gearman_job_take_workload(gearman_job_st *job, size_t *data_size)
{
  return gearman_packet_take_data(&(job->assigned), data_size);
}

/*
 * Static Definitions
 */

static gearman_return_t _job_send(gearman_job_st *job)
{
  gearman_return_t ret;

  ret= gearman_con_send(job->con, &(job->work), true);
  if (ret != GEARMAN_SUCCESS)
    return ret;

  gearman_packet_free(&(job->work));
  job->options&= (gearman_job_options_t)~GEARMAN_JOB_WORK_IN_USE;

  return GEARMAN_SUCCESS;
}
