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

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2025/12/20 14:40
 */
public class Frame implements Message {

  public final FrameType type;

  public final byte[] data;

  public Frame(FrameType type, byte[] data) {
    this.type = type;
    this.data = data;
  }

  public byte[] toBytes() {
    ByteBuf buffer = Unpooled.buffer(16);
    new MessagePackOutput(buffer).write(this);
    return ByteBufUtil.getBytes(buffer);
  }

  public <T extends Message> T read(Class<T> type) {
    try {
      T t = type.newInstance();
      t.readFrom(new MessagePackInput(Unpooled.wrappedBuffer(data)));
      return t;
    }
    catch (Exception e) {
      throw new SerializationException("read failed", e);
    }
  }

  public <T> T read(Factory<T> factory) {
    return factory.create(new MessagePackInput(Unpooled.wrappedBuffer(data)));
  }

  @Override
  public void writeTo(Output output) {
    output.write(type);
    output.writeFully(data);
  }

  public static Frame parse(byte[] data) {
    return parse(new MessagePackInput(Unpooled.wrappedBuffer(data)));
  }

  public static Frame parse(Input source) {
    FrameType frameType = source.readEnum(FrameType.class);
    return new Frame(frameType, source.readFully());
  }

}
