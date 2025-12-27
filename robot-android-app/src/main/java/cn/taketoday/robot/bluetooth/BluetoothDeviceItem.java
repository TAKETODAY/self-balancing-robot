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

public class BluetoothDeviceItem {

  private final BluetoothDevice device;

  private final String name;

  private final String address;

  private final boolean paired;

  public BluetoothDeviceItem(BluetoothDevice device) {
    this.device = device;
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

  @Override
  public boolean equals(Object o) {
    if (this == o)
      return true;
    if (o == null || getClass() != o.getClass())
      return false;
    BluetoothDeviceItem that = (BluetoothDeviceItem) o;
    return address.equals(that.address);
  }

  @Override
  public int hashCode() {
    return Objects.hash(address);
  }

  @Override
  public String toString() {
    return "BluetoothDeviceItem{" +
            "name='" + name + '\'' +
            ", address='" + address + '\'' +
            ", isPaired=" + paired +
            '}';
  }
}