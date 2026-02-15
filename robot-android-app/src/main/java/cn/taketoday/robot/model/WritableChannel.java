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

package cn.taketoday.robot.model;

/**
 * 表示一个可写的通道接口，用于向目标设备发送数据。
 *
 * <p>该接口定义了向通道写入字节数据的基本操作，通常用于与硬件设备或网络进行通信。
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2026/2/2 22:55
 */
public interface WritableChannel {

  /**
   * 将指定的字节数组数据写入通道。
   *
   * @param data 要写入的字节数据，不能为 {@code null}
   * @throws IllegalArgumentException 如果传入的 {@code data} 为 {@code null}
   */
  void write(byte[] data);
}
