#include <nuttx/config.h>

#include <debug.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/param.h>

#include <nuttx/mutex.h>
#include <nuttx/kmalloc.h>
#include <rpmsg/rpmsg_internal.h>

struct syslog_rpmsg_s
{
  volatile size_t       head;       /* The head index (where data is added) */
  volatile size_t       tail;       /* The tail index (where data is removed) */
  volatile size_t       flush;      /* The tail index of flush (where data is removed) */
  size_t                size;       /* Size of the RAM buffer */
  FAR char              *buffer;    /* Circular RAM buffer */
  struct work_s         work;       /* Used for deferred callback work */

  struct rpmsg_endpoint ept;
  bool                  suspend;
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: rpmsg_audio_init
 *
 * Description:
 *   This function is used to initialize the rpmsg audio.
 *
 *
 * Returned Values:
 *   OK on success; A negated errno value is returned on any failure.
 *
 ****************************************************************************/

int rpmsg_audio_init()
{
  return 0;
}
