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
package cn.taketoday.robot.bluetooth;

import android.bluetooth.BluetoothDevice;

/**
 * A listener for receiving notifications about changes in the Bluetooth device pairing (bonding) state.
 * Implement this interface to handle events related to the bonding process.
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @see BluetoothDevice#BOND_NONE
 * @see BluetoothDevice#BOND_BONDING
 * @see BluetoothDevice#BOND_BONDED
 */
public interface BindingStatusListener {

  /**
   * 设备配对状态改变
   * int BOND_NONE = 10; //配对没有成功
   * int BOND_BONDING = 11; //配对中
   * int BOND_BONDED = 12; //配对成功
   */
  void onBindingStatusChange(BluetoothDevice device);
}
