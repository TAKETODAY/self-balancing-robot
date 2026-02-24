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

/**
 * 控制消息类，用于封装左右轮速度控制指令。
 *
 * <p>该类实现了 {@link Message} 接口，表示一个控制命令消息，
 * 包含左轮和右轮的速度值，并支持将数据写入到可写对象中。</p>
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2026/2/7 22:59
 */
public class ControlMessage implements Message {

  /**
   * 左轮速度，单位为短整型（short）。
   */
  public final short leftWheelSpeed;

  /**
   * 右轮速度，单位为短整型（short）。
   */
  public final short rightWheelSpeed;

  /**
   * 构造方法，初始化左右轮速度。
   *
   * @param leftWheelSpeed 左轮速度（int 类型，会被转换为 short）
   * @param rightWheelSpeed 右轮速度（int 类型，会被转换为 short）
   */
  public ControlMessage(int leftWheelSpeed, int rightWheelSpeed) {
    this.leftWheelSpeed = (short) leftWheelSpeed;
    this.rightWheelSpeed = (short) rightWheelSpeed;
  }

  /**
   * 将左右轮速度数据写入到指定的可写对象中。
   *
   * @param writable 可写对象，用于接收数据
   */
  @Override
  public void writeTo(Writable writable) {
    writable.write(leftWheelSpeed);
    writable.write(rightWheelSpeed);
  }

}
