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

import cn.taketoday.robot.protocol.DefaultReadable;
import cn.taketoday.robot.protocol.Message;
import cn.taketoday.robot.protocol.Readable;
import cn.taketoday.robot.protocol.SerializationException;
import cn.taketoday.robot.protocol.Writable;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2026/2/13 23:30
 */
public class StatusReport implements Message {

  public static final Factory<StatusReport> factory = new StatusReportFactory();

  private final ReportType type;

  private final byte[] data;

  public StatusReport(ReportType type, byte[] data) {
    this.type = type;
    this.data = data;
  }

  @Override
  public void writeTo(Writable writable) {
    writable.write(type);
  }

  public <T extends Message> T read(Class<T> type) {
    try {
      T t = type.newInstance();
      t.readFrom(new DefaultReadable(data));
      return t;
    }
    catch (Exception e) {
      throw new SerializationException("read failed", e);
    }
  }

  public <T> T read(Factory<T> factory) {
    return factory.create(new DefaultReadable(data));
  }

  public Readable createBodyReadable() {
    return new DefaultReadable(data);
  }

  public ReportType getType() {
    return type;
  }

  public static StatusReport parse(Readable readable) {
    return factory.create(readable);
  }

  private static final class StatusReportFactory implements Factory<StatusReport> {

    @Override
    public StatusReport create(Readable readable) {
      ReportType reportType = ReportType.parse(readable);
      return new StatusReport(reportType, readable.readFully());
    }

    @Override
    public StatusReport[] newArray(int size) {
      return new StatusReport[size];
    }
  }

}
