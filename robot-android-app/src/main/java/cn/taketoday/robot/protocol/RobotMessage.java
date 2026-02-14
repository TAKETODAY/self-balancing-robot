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

import org.jspecify.annotations.Nullable;

import java.util.Arrays;
import java.util.concurrent.atomic.AtomicInteger;

import cn.taketoday.robot.protocol.message.ControlLegMessage;
import cn.taketoday.robot.protocol.message.PercentageValue;
import infra.lang.Enumerable;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2025/12/20 14:40
 */
public class RobotMessage implements Message {

  private static final AtomicInteger SEQUENCE = new AtomicInteger(1);

  public final short sequence;

  public final MessageType type;

  public final byte flags;

  public final byte @Nullable [] data;

  public RobotMessage(short sequence, MessageType type, byte flags, byte @Nullable [] data) {
    this.sequence = sequence;
    this.type = type;
    this.flags = flags;
    this.data = data;
  }

  public byte[] toBytes() {
    ByteBuf buffer = Unpooled.buffer(16);
    new MessagePackWritable(buffer).write(this);
    return ByteBufUtil.getBytes(buffer);
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

  @Override
  public void writeTo(Writable writable) {
    writable.write(sequence);
    writable.write((Message) type);
    writable.write(flags);

    if (data != null) {
      writable.writeFully(data);
    }
  }

  @Override
  public String toString() {
    return String.format("RobotMessage[%s, %s, %s, %s]", sequence, type, flags, Arrays.toString(data));
  }

  public static RobotMessage parse(byte[] data) {
    return parse(new DefaultReadable(data));
  }

  public static RobotMessage parse(Readable source) {
    int sequence = source.readUnsignedShort();
    int type = source.readByte();
    byte flags = source.readByte();
    MessageType messageType = Enumerable.of(MessageType.class, type);
    return new RobotMessage((short) sequence, messageType, flags, source.readFully());
  }

  public static RobotMessage forControl(int leftWheelSpeed, int rightWheelSpeed) {
    ControlMessage controlMessage = new ControlMessage(leftWheelSpeed, rightWheelSpeed);
    return new RobotMessage(generateSequence(), MessageType.CONTROL, (byte) 0, controlMessage.toByteArray());
  }

  public static RobotMessage forEmergencyStop() {
    return new RobotMessage(generateSequence(), MessageType.EMERGENCY_STOP, (byte) 0, null);
  }

  public static RobotMessage forEmergencyRecover() {
    return new RobotMessage(generateSequence(), MessageType.EMERGENCY_RECOVER, (byte) 0, null);
  }

  public static RobotMessage forControlLeg(int leftPercentage, int rightPercentage) {
    ControlLegMessage controlMessage = new ControlLegMessage(leftPercentage, rightPercentage);
    return new RobotMessage(generateSequence(), MessageType.CONTROL_LEG, (byte) 0, controlMessage.toByteArray());
  }

  public static RobotMessage forControlHeight(int percentage) {
    PercentageValue controlMessage = new PercentageValue(percentage);
    return new RobotMessage(generateSequence(), MessageType.CONTROL_HEIGHT, (byte) 0, controlMessage.toByteArray());
  }

  static short generateSequence() {
    int val = SEQUENCE.getAndIncrement();
    if (val > Short.MAX_VALUE) {
      val = 0;
      SEQUENCE.set(val);
    }
    return (short) val;
  }

}
