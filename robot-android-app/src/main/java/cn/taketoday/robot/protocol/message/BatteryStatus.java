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

package cn.taketoday.robot.protocol.message;

import java.util.Objects;

import cn.taketoday.robot.protocol.Message;
import cn.taketoday.robot.protocol.Readable;
import cn.taketoday.robot.protocol.Writable;

/**
 * 电池状态消息类，用于表示机器人的电池电压和电量百分比信息。
 *
 * <p>该类实现了 {@link Message} 接口，支持序列化和反序列化操作，
 * 可通过 {@link Writable} 写入数据，通过 {@link Readable} 读取数据。
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2026/2/14 13:24
 */
public class BatteryStatus implements Message {

  private float voltage;

  private byte percentage;

  @Override
  public void writeTo(Writable writable) {
    writable.write(voltage);
    writable.write(percentage);
  }

  @Override
  public void readFrom(Readable readable) {
    voltage = readable.readFloat();
    percentage = readable.readByte();
  }

  public int getPercentage() {
    return percentage;
  }

  public float getVoltage() {
    return voltage;
  }

  @Override
  public boolean equals(Object o) {
    if (o == null || getClass() != o.getClass())
      return false;
    BatteryStatus that = (BatteryStatus) o;
    return Float.compare(voltage, that.voltage) == 0
            && percentage == that.percentage;
  }

  @Override
  public int hashCode() {
    return Objects.hash(voltage, percentage);
  }

  @Override
  public String toString() {
    return String.format("BatteryStatus[%s, %s]", voltage, percentage);
  }
}
