/****************************************************************************
 * arch/arm/src/goldfish/goldfish_timer.c
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

#include <nuttx/timers/arch_alarm.h>
#include <nuttx/timers/goldfish_timer.h>
#include <nuttx/fdt.h>

#include "arm_timer.h"
#include "chip.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void up_timer_initialize(void)
{
#if defined(CONFIG_GOLDFISH_TIMER) && defined(CONFIG_LIBC_FDT)
  struct oneshot_lowerhalf_s *lower;
  const void *fdt = fdt_get();

  DEBUGASSERT(fdt != NULL);

  lower = goldfish_timer_initialize(
            fdt_get_reg_base_by_path(fdt, "/goldfish_rtc"),
            fdt_get_irq_by_path(fdt, 1, "/goldfish_rtc", QEMU_SPI_IRQ_BASE));

  DEBUGASSERT(lower != NULL);

  up_alarm_set_lowerhalf(lower);
#else
  up_alarm_set_lowerhalf(arm_timer_initialize(0));
#endif
}
