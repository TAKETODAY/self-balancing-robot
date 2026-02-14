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

/**
 * 报告类型枚举类，定义了机器人可发送的各种报告类型。
 *
 * <p>每个报告类型都有对应的数值标识，用于在网络协议中传输。</p>
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2026/2/13 23:31
 */
public enum ReportType implements Message {

  battery(1),

  robot_height(2);

  private final int value;

  ReportType(int value) {
    this.value = value;
  }

  @Override
  public void writeTo(Writable writable) {
    writable.write((byte) value);
  }

  public static ReportType parse(Readable readable) {
    return factory.create(readable);
  }

  private static final ReportTypeFactory factory = new ReportTypeFactory();

  public static class ReportTypeFactory implements Factory<ReportType> {

    @Override
    public ReportType create(Readable readable) {
      byte value = readable.readByte();
      return switch (value) {
        case 1 -> battery;
        case 2 -> robot_height;
        default -> throw new IllegalArgumentException("unknown report type: " + value);
      };
    }

    @Override
    public ReportType[] newArray(int size) {
      return new ReportType[size];
    }
  }
}
