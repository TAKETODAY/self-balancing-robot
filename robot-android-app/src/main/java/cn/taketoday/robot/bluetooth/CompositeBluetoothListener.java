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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import cn.taketoday.robot.LoggingSupport;

/**
 * all Bluetooth listeners
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public class CompositeBluetoothListener implements BluetoothStatusListener, BluetoothBindingListener,
        BluetoothScanningListener, BluetoothConnectionListener, LoggingSupport {

  private static final CompositeBluetoothListener BLUETOOTH_LISTENER = new CompositeBluetoothListener();

  public static CompositeBluetoothListener getInstance() {
    return BLUETOOTH_LISTENER;
  }

  private final List<BluetoothStatusListener> statusListeners = new ArrayList<>(4);
  private final List<BluetoothBindingListener> bindingListeners = new ArrayList<>(4);
  private final List<BluetoothScanningListener> scanningListeners = new ArrayList<>(4);
  private final List<BluetoothConnectionListener> connectionListeners = new ArrayList<>(4);

  public void addBindingListener(BluetoothBindingListener... bindingListeners) {
    if (isDebugEnabled()) {
      logger("添加配对状态监听器", Arrays.toString(bindingListeners));
    }
    Collections.addAll(this.bindingListeners, bindingListeners);
  }

  public void addStatusListener(BluetoothStatusListener... statusListeners) {
    if (isDebugEnabled()) {
      logger("添加状态监听器", Arrays.toString(statusListeners));
    }
    Collections.addAll(this.statusListeners, statusListeners);
  }

  public void addScanningListener(BluetoothScanningListener... scanningListeners) {
    if (isDebugEnabled()) {
      logger("添加扫描监听器", Arrays.toString(scanningListeners));
    }
    Collections.addAll(this.scanningListeners, scanningListeners);
  }

  public void addConnectionListener(BluetoothConnectionListener... connectionListeners) {

    if (isDebugEnabled()) {
      logger("添加连接监听器", Arrays.toString(connectionListeners));
    }
    Collections.addAll(this.connectionListeners, connectionListeners);
  }

  @Override
  public void onStatusChange(final int status) {
    for (BluetoothStatusListener statusListener : statusListeners) {
      statusListener.onStatusChange(status);
    }
  }

  @Override
  public void onBindingStatusChange(final BluetoothDevice device) {
    for (BluetoothBindingListener bindingListener : bindingListeners) {
      bindingListener.onBindingStatusChange(device);
    }
  }

  // BluetoothScanningListener
  // -------------------
  @Override
  public void onScanningStart() {
    for (BluetoothScanningListener scanningListener : scanningListeners) {
      scanningListener.onScanningStart();
    }
  }

  @Override
  public void onScanningStop() {
    for (BluetoothScanningListener scanningListener : scanningListeners) {
      scanningListener.onScanningStop();
    }
  }

  @Override
  public void onDeviceFound(final BluetoothDevice device) {
    for (BluetoothScanningListener scanningListener : scanningListeners) {
      scanningListener.onDeviceFound(device);
    }
  }

  // BluetoothConnectionListener
  //-------------------------------------------------

  @Override
  public void onConnecting(final BluetoothDevice device) {
    for (BluetoothConnectionListener connectionListener : connectionListeners) {
      connectionListener.onConnecting(device);
    }
  }

  @Override
  public void onConnected(final BluetoothDevice device) {
    for (BluetoothConnectionListener connectionListener : connectionListeners) {
      connectionListener.onConnected(device);
    }
  }

  @Override
  public void onDisconnecting(final BluetoothDevice device) {
    for (BluetoothConnectionListener connectionListener : connectionListeners) {
      connectionListener.onDisconnecting(device);
    }
  }

  @Override
  public void onDisconnect(final BluetoothDevice device) {
    for (BluetoothConnectionListener connectionListener : connectionListeners) {
      connectionListener.onDisconnect(device);
    }
  }

  @Override
  public void onConnectedDevice(final Collection<BluetoothDevice> devices) {
    for (BluetoothConnectionListener connectionListener : connectionListeners) {
      connectionListener.onConnectedDevice(devices);
    }
  }

  //
  // -------------------

}