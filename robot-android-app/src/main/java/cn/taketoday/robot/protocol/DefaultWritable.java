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

import java.io.DataOutput;
import java.time.Instant;
import java.util.List;
import java.util.Map;
import java.util.function.BiConsumer;
import java.util.function.Consumer;

import infra.lang.Enumerable;
import infra.util.function.ThrowingConsumer;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2026/2/7 22:57
 */
public class DefaultWritable implements Writable {

  private final DataOutput dataOutput;

  public DefaultWritable(DataOutput dataOutput) {
    this.dataOutput = dataOutput;
  }

  private void doWrite(ThrowingConsumer<DataOutput> consumer) {
    try {
      consumer.acceptWithException(dataOutput);
    }
    catch (Throwable e) {
      throw new SerializationException("write failed", e);
    }
  }

  @Override
  public void write(byte @Nullable [] b) {
    doWrite(output -> {
      if (b != null) {
        output.write((short) b.length);
        output.write(b);
      }
      else {
        output.write((short) 0);
      }
    });
  }

  @Override
  public void write(byte[] b, int off, int len) {
    doWrite(output -> output.write(b, off, len));
  }

  @Override
  public void writeFully(byte[] b) {
    doWrite(output -> output.write(b));
  }

  @Override
  public void writeFully(byte[] b, int off, int len) {
    doWrite(output -> output.write(b, off, len));
  }

  @Override
  public void write(boolean v) {
    doWrite(output -> output.writeBoolean(v));
  }

  @Override
  public void write(byte b) {
    doWrite(output -> output.writeByte(b));
  }

  @Override
  public void write(short v) {
    doWrite(output -> output.writeShort(v));
  }

  @Override
  public void write(int v) {
    doWrite(output -> output.writeInt(v));
  }

  @Override
  public void write(long v) {
    doWrite(output -> output.writeLong(v));
  }

  @Override
  public void write(float v) {
    doWrite(output -> output.writeFloat(v));
  }

  @Override
  public void write(double v) {
    doWrite(output -> output.writeDouble(v));
  }

  @Override
  public void write(@Nullable String v) {
    doWrite(output -> output.writeUTF(v));
  }

  @Override
  public void write(Enumerable<Integer> v) {
    doWrite(output -> output.writeShort(v.getValue()));
  }

  @Override
  public void write(Instant v) {

  }

  @Override
  public void writeTimestamp(long epochSecond, int nanoAdjustment) {

  }

  @Override
  public void writeTimestamp(long millis) {

  }

  @Override
  public void write(Message v) {
    v.writeTo(this);
  }

  @Override
  public <V> void writeNullable(@Nullable V v, BiConsumer<Writable, V> valueMapper) {

  }

  @Override
  public <T> void write(T[] v, Consumer<T> mapper) {

  }

  @Override
  public <T> void write(T[] v, BiConsumer<Writable, T> mapper) {

  }

  @Override
  public <T> void write(List<T> v, Consumer<T> mapper) {

  }

  @Override
  public <T> void write(List<T> v, BiConsumer<Writable, T> mapper) {

  }

  @Override
  public <K, V> void write(Map<K, V> v, Consumer<K> keyMapper, Consumer<V> valueMapper) {

  }

  @Override
  public <K, V> void write(Map<K, V> v, BiConsumer<Writable, K> keyMapper, BiConsumer<Writable, V> valueMapper) {

  }
}
