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

package cn.taketoday.robot.protocol.message;

import cn.taketoday.robot.protocol.Message;
import cn.taketoday.robot.protocol.Writable;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2026/2/9 21:44
 */
public class ControlLegMessage implements Message {

  public final byte leftPercentage;

  public final byte rightPercentage;

  public ControlLegMessage(int leftPercentage, int rightPercentage) {
    this.leftPercentage = percentage(leftPercentage);
    this.rightPercentage = percentage(rightPercentage);
  }

  @Override
  public void writeTo(Writable writable) {
    writable.write(leftPercentage);
    writable.write(rightPercentage);
  }

  private static byte percentage(int percentage) {
    if (percentage > 100) {
      return 100;
    }

    if (percentage < 0) {
      percentage = 10;
    }

    return (byte) percentage;
  }
}
