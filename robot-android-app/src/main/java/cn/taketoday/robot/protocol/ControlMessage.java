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

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2026/2/7 22:59
 */
public class ControlMessage implements Message {

  public final short leftWheelSpeed;

  public final short rightWheelSpeed;

  public final byte legHeightPercentage;

  public ControlMessage(int leftWheelSpeed, int rightWheelSpeed, int legHeightPercentage) {
    this.leftWheelSpeed = (short) leftWheelSpeed;
    this.rightWheelSpeed = (short) rightWheelSpeed;
    this.legHeightPercentage = percentage(legHeightPercentage);
  }

  @Override
  public void writeTo(Writable writable) {
    writable.write(leftWheelSpeed);
    writable.write(rightWheelSpeed);
    writable.write(legHeightPercentage);
  }

  private static byte percentage(int legHeightPercentage) {
    if (legHeightPercentage > 100) {
      return 100;
    }

    if (legHeightPercentage < 0) {
      legHeightPercentage = 10;
    }

    return (byte) legHeightPercentage;
  }

}
