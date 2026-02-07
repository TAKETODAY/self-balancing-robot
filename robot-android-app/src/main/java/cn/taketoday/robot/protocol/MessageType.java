/*
 * Copyright 2025 - 2026 the original author or authors.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see [https://www.gnu.org/licenses/]
 */

package cn.taketoday.robot.protocol;

import infra.lang.Enumerable;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2025/12/20 14:40
 */
public enum MessageType implements Message, Enumerable<Integer> {
  CONTROL(0x01),
  EMERGENCY_STOP(0x02),

  CONFIG_SET(0x10),
  CONFIG_GET(0x11),
  FIRMWARE_INFO(0x12),
  STATUS_REPORT(0x13),
  ACTION_PLAY(0x20),

  ACK(0x80),
  ERROR(0x81),
  SENSOR_DATA(0x82);

  public final int value;

  MessageType(int value) {
    this.value = value;
  }

  @Override
  public Integer getValue() {
    return value;
  }

  @Override
  public void writeTo(Writable writable) {
    writable.write((byte) value);
  }

}
