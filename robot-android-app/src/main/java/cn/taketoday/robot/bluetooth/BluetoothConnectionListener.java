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

import java.util.Collection;

/**
 * BluetoothConnectionListener
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public interface BluetoothConnectionListener {

  void onConnecting(BluetoothDevice device);//连接中

  void onConnected(BluetoothDevice device);//连接成功

  void onDisconnecting(BluetoothDevice device);//断开中

  void onDisconnect(BluetoothDevice device);//断开

  void onConnectedDevice(Collection<BluetoothDevice> devices);//已连接的设备
}
