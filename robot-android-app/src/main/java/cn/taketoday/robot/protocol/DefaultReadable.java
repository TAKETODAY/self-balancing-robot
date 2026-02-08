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

import java.io.ByteArrayInputStream;
import java.io.DataInput;
import java.io.DataInputStream;
import java.time.Instant;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.function.Function;
import java.util.function.Supplier;

import infra.lang.Enumerable;
import infra.util.function.ThrowingFunction;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2026/2/8 12:47
 */
public class DefaultReadable implements Readable {

  private final DataInputStream dataInput;

  public DefaultReadable(byte[] bytes) {
    this(new DataInputStream(new ByteArrayInputStream(bytes)));
  }

  public DefaultReadable(DataInputStream dataInput) {
    this.dataInput = dataInput;
  }

  @Override
  public void read(byte[] b) {
    doRead(input -> {
      input.readFully(b);
      return null;
    });
  }

  @Override
  public void read(byte[] b, int off, int len) {
    doRead(input -> {
      input.readFully(b, off, len);
      return null;
    });
  }

  @Override
  public byte[] read() {
    return doRead(input -> {
      int len = input.readUnsignedShort();
      byte[] bytes = new byte[len];
      input.readFully(bytes);
      return bytes;
    });
  }

  @Override
  public byte[] read(int len) {
    return doRead(input -> {
      byte[] bytes = new byte[len];
      input.readFully(bytes);
      return bytes;
    });
  }

  @Override
  public byte[] readFully() {
    return doRead(input -> {
      byte[] bytes = new byte[input.available()];
      input.readFully(bytes);
      return bytes;
    });
  }

  @Override
  public int skipBytes(int n) {
    return doRead(input -> input.skipBytes(n));
  }

  @Override
  public boolean readBoolean() {
    return doRead(DataInput::readBoolean);
  }

  @Override
  public byte readByte() {
    return doRead(DataInput::readByte);
  }

  @Override
  public int readUnsignedByte() {
    return doRead(DataInput::readUnsignedByte);
  }

  @Override
  public short readShort() {
    return doRead(DataInput::readShort);
  }

  @Override
  public int readUnsignedShort() {
    return doRead(DataInput::readUnsignedShort);
  }

  @Override
  public int readInt() {
    return doRead(DataInput::readInt);
  }

  @Override
  public long readLong() {
    return doRead(DataInput::readLong);
  }

  @Override
  public float readFloat() {
    return doRead(DataInput::readFloat);
  }

  @Override
  public double readDouble() {
    return doRead(DataInput::readDouble);
  }

  @Override
  public String readString() {
    return doRead(DataInput::readUTF);
  }

  @Override
  public <V extends Enumerable<Integer>> V readEnum(Class<V> type) {
    return Enumerable.of(type, readInt());
  }

  @Override
  public Instant readTimestamp() {
    return null;
  }

  @Override
  public void read(Message message) {

  }

  @Override
  public <V> @Nullable V readNullable(Function<Readable, V> valueMapper) {
    return null;
  }

  @Override
  public <T> T[] read(Class<T> type, Function<Readable, T> mapper) {
    return null;
  }

  @Override
  public <T> T[] read(Class<T> type, Supplier<T> supplier) {
    return null;
  }

  @Override
  public <T> List<T> read(Function<Readable, T> mapper) {
    return Collections.emptyList();
  }

  @Override
  public <T> List<T> read(Supplier<T> supplier) {
    return Collections.emptyList();
  }

  @Override
  public <K, V> Map<K, V> read(Function<Readable, K> keyMapper, Function<Readable, V> valueMapper) {
    return Collections.emptyMap();
  }

  private <T> T doRead(ThrowingFunction<DataInputStream, T> consumer) {
    try {
      return consumer.applyWithException(dataInput);
    }
    catch (Throwable e) {
      throw new SerializationException("read failed", e);
    }
  }
}
