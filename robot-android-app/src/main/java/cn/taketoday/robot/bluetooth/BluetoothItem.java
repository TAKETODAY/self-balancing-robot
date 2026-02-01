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

import java.util.Objects;

public class BluetoothItem {

  public static final String STATUS_BONDED = "已配对";
  public static final String STATUS_CONNECTED = "已连接";
  public static final String STATUS_BOND_NONE = "未配对";

  private final BluetoothDevice device;

  private final String name;

  private final String address;

  private final boolean paired;

  private final boolean connected;

  private final int rssi;

  public BluetoothItem(BluetoothDevice device, int rssi, boolean connected) {
    this.rssi = rssi;
    this.device = device;
    this.connected = connected;
    String name = device.getName();
    this.name = name != null ? name : "Unknown";
    this.address = device.getAddress();
    this.paired = device.getBondState() == BluetoothDevice.BOND_BONDED;
  }

  public BluetoothDevice getDevice() {
    return device;
  }

  public String getName() {
    return name;
  }

  public String getAddress() {
    return address;
  }

  public boolean isPaired() {
    return paired;
  }

  public boolean isConnected() {
    return connected;
  }

  public int getRssi() {
    return rssi;
  }

  public String getStatusText() {
    if (connected) {
      return STATUS_CONNECTED;
    }
    return isPaired() ? STATUS_BONDED : STATUS_BOND_NONE;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o)
      return true;
    if (o == null || getClass() != o.getClass())
      return false;
    BluetoothItem that = (BluetoothItem) o;
    return address.equals(that.address);
  }

  @Override
  public int hashCode() {
    return Objects.hash(address);
  }

  @Override
  public String toString() {
    return "BluetoothItem{" +
            "name='" + name + '\'' +
            ", address='" + address + '\'' +
            ", isPaired=" + paired +
            ", rssi=" + rssi +
            '}';
  }
}