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
import cn.taketoday.robot.protocol.Readable;
import cn.taketoday.robot.protocol.Writable;

import static cn.taketoday.robot.util.RobotUtils.constrain;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2026/2/14 13:27
 */
public class PercentageValue implements Message {

  public static final Factory<PercentageValue> factory = new PercentageValueFactory();

  public final int value;

  public PercentageValue(int value) {
    this.value = percentage(value);
  }

  @Override
  public void writeTo(Writable writable) {
    writable.write(value);
  }

  @Override
  public byte[] toByteArray() {
    return new byte[] { (byte) value };
  }

  public static PercentageValue parse(Readable readable) {
    return factory.create(readable);
  }

  public static byte percentage(int percentage) {
    return (byte) constrain(percentage, 0, 100);
  }

  private static final class PercentageValueFactory implements Factory<PercentageValue> {

    @Override
    public PercentageValue create(Readable readable) {
      return new PercentageValue(readable.readByte());
    }

    @Override
    public PercentageValue[] newArray(int size) {
      return new PercentageValue[size];
    }
  }

}
