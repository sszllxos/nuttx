/****************************************************************************
 * drivers/analog/dac7554.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/kmalloc.h>
#include <nuttx/arch.h>
#include <nuttx/analog/dac.h>
#include <nuttx/spi/spi.h>

/****************************************************************************
 * Preprocessor definitions
 ****************************************************************************/

#if !defined(CONFIG_SPI)
#  error SPI Support Required.
#endif

#if defined(CONFIG_DAC7554)

#ifndef CONFIG_DAC7554_SPI_FREQUENCY
#  define CONFIG_DAC7554_SPI_FREQUENCY 1000000
#endif

#define DAC7554_SPI_MODE (SPIDEV_MODE2) /* SPI Mode 2: CPOL=1,CPHA=0 */

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct dac7554_dev_s
{
  FAR struct spi_dev_s *spi;  /* SPI interface */
  int spidev;                 /* SPI Chip Select number */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/* I2C Helpers */

/* DAC methods */

static void dac7554_reset(FAR struct dac_dev_s *dev);
static int  dac7554_setup(FAR struct dac_dev_s *dev);
static void dac7554_shutdown(FAR struct dac_dev_s *dev);
static void dac7554_txint(FAR struct dac_dev_s *dev, bool enable);
static int  dac7554_send(FAR struct dac_dev_s *dev,
                         FAR struct dac_msg_s *msg);
static int  dac7554_ioctl(FAR struct dac_dev_s *dev, int cmd,
                          unsigned long arg);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct dac_ops_s g_dacops =
{
  dac7554_reset,        /* ao_reset */
  dac7554_setup,        /* ao_setup */
  dac7554_shutdown,     /* ao_shutdown */
  dac7554_txint,        /* ao_txint */
  dac7554_send,         /* ao_send */
  dac7554_ioctl         /* ao_ioctl */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: dac7554_configspi
 *
 * Description:
 *   Configure the SPI interface
 *
 ****************************************************************************/

static inline void dac7554_configspi(FAR struct spi_dev_s *spi)
{
  SPI_SETMODE(spi, DAC7554_SPI_MODE);
  SPI_SETBITS(spi, 8);
  SPI_HWFEATURES(spi, 0);
  SPI_SETFREQUENCY(spi, CONFIG_DAC7554_SPI_FREQUENCY);
}

/****************************************************************************
 * Name: dac7554_reset
 *
 * Description:
 *   Reset the DAC device. Called early to initialize the hardware. This
 *   is called, before ao_setup() and on error conditions.
 *
 ****************************************************************************/

static void dac7554_reset(FAR struct dac_dev_s *dev)
{
}

/****************************************************************************
 * Name: dac7554_setup
 *
 * Description:
 *   Configure the DAC. This method is called the first time that the DAC
 *   device is opened.  This will occur when the port is first opened.
 *   This setup includes configuring and attaching DAC interrupts. Interrupts
 *   are all disabled upon return.
 *
 ****************************************************************************/

static int dac7554_setup(FAR struct dac_dev_s *dev)
{
  return OK;
}

/****************************************************************************
 * Name: dac7554_shutdown
 *
 * Description:
 *   Disable the DAC. This method is called when the DAC device is closed.
 *   This method reverses the operation the setup method.
 *
 ****************************************************************************/

static void dac7554_shutdown(FAR struct dac_dev_s *dev)
{
}

/****************************************************************************
 * Name: dac7554_txint
 *
 * Description:
 *   Call to enable or disable TX interrupts
 *
 ****************************************************************************/

static void dac7554_txint(FAR struct dac_dev_s *dev, bool enable)
{
}

/****************************************************************************
 * Name: dac7554_send
 ****************************************************************************/

static int dac7554_send(FAR struct dac_dev_s *dev, FAR struct dac_msg_s *msg)
{
  FAR struct dac7554_dev_s *priv = (FAR struct dac7554_dev_s *)dev->ad_priv;
  uint16_t data;

  /* Sanity check */

  DEBUGASSERT(priv->spi != NULL);

  /* Set up message to send */

  ainfo("value: %08x\n", msg->am_data);

  SPI_LOCK(priv->spi, true);

  dac7554_configspi(priv->spi);

  data = ((msg->am_data >> 16) & 0x0fff) | ((msg->am_channel & 0x03) << 12) |
    (1 << 15);

  SPI_SELECT(priv->spi, priv->spidev, true);

  SPI_SEND(priv->spi, (data >> 8));
  SPI_SEND(priv->spi, (data & 0xff));

  SPI_SELECT(priv->spi, priv->spidev, false);

  SPI_LOCK(priv->spi, false);

  dac_txdone(dev);
  return 0;
}

/****************************************************************************
 * Name: dac7554_ioctl
 ****************************************************************************/

static int dac7554_ioctl(FAR struct dac_dev_s *dev, int cmd,
                         unsigned long arg)
{
  return -ENOTTY;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: dac7554_initialize
 *
 * Description:
 *   Initialize DAC
 *
 * Input Parameters:
 *   Port number (for hardware that has multiple DAC interfaces)
 *
 * Returned Value:
 *   Valid DAC7554 device structure reference on success; a NULL on failure
 *
 ****************************************************************************/

FAR struct dac_dev_s *dac7554_initialize(FAR struct spi_dev_s *spi,
                                         int spidev)
{
  FAR struct dac7554_dev_s *priv;
  FAR struct dac_dev_s *g_dacdev;

  /* Sanity check */

  DEBUGASSERT(spi != NULL);

  /* Initialize the DAC7554 device structure */

  priv = kmm_malloc(sizeof(struct dac7554_dev_s));
  priv->spi = spi;
  priv->spidev = spidev;

  g_dacdev = kmm_malloc(sizeof(struct dac_dev_s));
  g_dacdev->ad_ops = &g_dacops;
  g_dacdev->ad_priv = priv;

  return g_dacdev;
}

#endif /* CONFIG_DAC7554 */
