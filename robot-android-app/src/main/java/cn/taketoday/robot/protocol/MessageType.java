/*
 * Copyright 2017 - 2025 the original author or authors.
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
  CONTROL(1),
  CONTROL_LEG(2),
  CONTROL_HEIGHT(3),
  CONTROL_JOY(4),

  CONFIG_SET(20),
  CONFIG_GET(21),
  FIRMWARE_INFO(22),
  STATUS_REPORT(23),

  ACTION_PLAY(40),

  ACK(60),
  ERROR(61),
  SENSOR_DATA(62),

  EMERGENCY_STOP(80),
  EMERGENCY_RECOVER(81);

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
