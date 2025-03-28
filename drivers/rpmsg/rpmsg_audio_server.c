#include <nuttx/config.h>

#include <debug.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include <nuttx/kmalloc.h>
#include <nuttx/mutex.h>
#include <nuttx/rpmsg/rpmsg.h>
#include <rpmsg/rpmsg_internal.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define RPMSG_AUDIO_CMD_SYNC           1
#define RPMSG_AUDIO_CMD_DATA           2
#define RPMSG_AUDIO_CMD_SHUTDOWN       3

/****************************************************************************
 * Private Types
 ****************************************************************************/

begin_packed_struct struct rpmsg_audio_sync_s
{
  uint32_t                       cmd;
  uint32_t                       size;
  uint32_t                       pid;
  uint32_t                       uid;
  uint32_t                       gid;
} end_packed_struct;

begin_packed_struct struct rpmsg_audio_data_s
{
  uint32_t                       cmd;
  uint32_t                       pos;

  /* Act data len, don't include len itself when SOCK_DGRAM */

  uint32_t                       len;
  uint8_t                        data[0];
} end_packed_struct;

begin_packed_struct struct rpmsg_audio_shutdown_s
{
  uint32_t                       cmd;
  uint32_t                       how;
} end_packed_struct;

struct rpmsg_audio_conn_s {
  struct rpmsg_endpoint          ept;
};

struct audio_rpmsg_s
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

static struct audio_rpmsg_s g_audio_rpmsg;
 
static ssize_t audio_rpmsg_file_read(FAR struct file *filep,
                                     FAR char *buffer, size_t len)
{
  syslog(6, "%s\n", __func__);
  return 0;
}

static ssize_t audio_rpmsg_file_write(FAR struct file *filep,
                                      FAR const char *buffer, size_t len)
{
  printf("%s\n", __func__);

  return len;
}

static const struct file_operations g_audio_rpmsgfops =
{
  NULL,                    /* open */
  NULL,                    /* close */
  audio_rpmsg_file_read,  /* read */
  audio_rpmsg_file_write, /* write */
};

static FAR struct rpmsg_audio_conn_s *rpmsg_audio_alloc(void) {
  FAR struct rpmsg_audio_conn_s *conn;
  // int ret;

  conn = kmm_zalloc(sizeof(struct rpmsg_audio_conn_s));
  if (!conn)
    {
      return NULL;
    }


  return conn;
}

static int rpmsg_audio_ept_cb(FAR struct rpmsg_endpoint *ept,
                               FAR void *data, size_t len, uint32_t src,
                               FAR void *priv)
{
  // FAR struct rpmsg_audio_conn_s *conn = ept->priv;
  FAR struct rpmsg_audio_sync_s *head = data;

  printf("lijing in ====> %s:: cmd: %d\n", __func__, head->cmd);
  if (head->cmd == RPMSG_AUDIO_CMD_DATA) {
    FAR struct rpmsg_audio_data_s *msg = data;
    FAR uint8_t *buf = msg->data;

    printf("lijing cmd data ====> %s\n", (char *)buf);

  }

  return 0;
}

static void rpmsg_audio_free(FAR struct rpmsg_audio_conn_s *conn)
{
  kmm_free(conn);
}

static void rpmsg_audio_ns_unbind(FAR struct rpmsg_endpoint *ept)
{
}

static bool rpmsg_audio_server_ns_match(FAR struct rpmsg_device *rdev,
                                        FAR void *priv, FAR const char *name,
                                        uint32_t dest)
{
  printf("lijing in ===> %s   name: %s\n", __func__, name);

  if (strcmp(name, "test_audio") == 0)
  {
    return true;
  }

  return false;
}

static void rpmsg_audio_ns_bound(struct rpmsg_endpoint *ept)
{
  FAR struct rpmsg_audio_conn_s *conn = ept->priv;
  struct rpmsg_audio_sync_s msg;

  msg.cmd  = RPMSG_AUDIO_CMD_SYNC;
//  msg.size = circbuf_size(&conn->recvbuf);
//  msg.pid  = nxsched_getpid();
//  msg.uid  = getuid();
//  msg.gid  = getgid();

  printf("=====> %s\n", __func__);
  rpmsg_send(&conn->ept, &msg, sizeof(msg));
}

static void rpmsg_audio_server_ns_bind(FAR struct rpmsg_device *rdev,
                                       FAR void *priv, FAR const char *name,
                                       uint32_t dest)
{
  FAR struct rpmsg_audio_conn_s *new;
  printf("name: %s  ===> %s  dest: %d\n", name,  __func__, dest);
  int ret;

  new = rpmsg_audio_alloc();
  if (!new)
    {
      return;
    }

  new->ept.priv = new;
  ret = rpmsg_create_ept(&new->ept, rdev, name,
                         RPMSG_ADDR_ANY, dest,
                         rpmsg_audio_ept_cb, rpmsg_audio_ns_unbind);
  if (ret < 0)
    {
      rpmsg_audio_free(new);
      return;
    }

  rpmsg_audio_ns_bound(&new->ept);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: rpmsg_audio_server_init
 *
 * Description:
 *   This function is used to initialize the rpmsg audio server.
 *
 *
 * Returned Values:
 *   OK on success; A negated errno value is returned on any failure.
 *
 ****************************************************************************/

int rpmsg_audio_server_init(void) {
  int ret;
  ret = register_driver("/dev/rrr", &g_audio_rpmsgfops,
                        0666, &g_audio_rpmsg);
  if (ret < 0)
    {
      return ret;
    }

  rpmsg_register_callback(&g_audio_rpmsg, 
      NULL, 
      NULL, 
      rpmsg_audio_server_ns_match,
      rpmsg_audio_server_ns_bind);

  return 0;
}
