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

package cn.taketoday.robot.bluetooth;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import cn.taketoday.robot.LoggingSupport;

/**
 * A centralized manager for all Bluetooth-related event listeners.
 *
 * <p>This class acts as a hub, implementing various Bluetooth listener interfaces
 * ({@link StatusListener}, {@link BindingStatusListener}, {@link ScanningListener},
 * and {@link ConnectionListener}). It maintains lists of registered listeners for
 * each category and dispatches events to all relevant listeners when a callback
 * method is invoked.
 *
 * <p>This class follows the Singleton pattern to provide a single, globally accessible
 * instance, simplifying listener management throughout the application.
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @see StatusListener
 * @see BindingStatusListener
 * @see ScanningListener
 * @see ConnectionListener
 */
class BluetoothListeners implements StatusListener, BindingStatusListener, ScanningListener, ConnectionListener, LoggingSupport {

  private final List<StatusListener> statusListeners = new ArrayList<>(4);

  private final List<BindingStatusListener> bindingListeners = new ArrayList<>(4);

  private final List<ScanningListener> scanningListeners = new ArrayList<>(4);

  private final List<ConnectionListener> connectionListeners = new ArrayList<>(4);

  public void addBindingListener(BindingStatusListener... bindingListeners) {
    if (isDebugEnabled()) {
      debug("添加配对状态监听器", Arrays.toString(bindingListeners));
    }
    Collections.addAll(this.bindingListeners, bindingListeners);
  }

  public void addStatusListener(StatusListener... statusListeners) {
    if (isDebugEnabled()) {
      debug("添加状态监听器", Arrays.toString(statusListeners));
    }
    Collections.addAll(this.statusListeners, statusListeners);
  }

  public void addScanningListener(ScanningListener... scanningListeners) {
    if (isDebugEnabled()) {
      debug("添加扫描监听器", Arrays.toString(scanningListeners));
    }
    Collections.addAll(this.scanningListeners, scanningListeners);
  }

  public void addConnectionListener(ConnectionListener... connectionListeners) {
    if (isDebugEnabled()) {
      debug("添加连接监听器", Arrays.toString(connectionListeners));
    }
    Collections.addAll(this.connectionListeners, connectionListeners);
  }

  @Override
  public void onStatusChange(int status) {
    for (StatusListener statusListener : statusListeners) {
      statusListener.onStatusChange(status);
    }
  }

  @Override
  public void onBindingStatusChange(BluetoothDevice device) {
    for (BindingStatusListener bindingListener : bindingListeners) {
      bindingListener.onBindingStatusChange(device);
    }
  }

  // BluetoothScanningListener
  // -------------------
  @Override
  public void onScanningStarted() {
    for (ScanningListener scanningListener : scanningListeners) {
      scanningListener.onScanningStarted();
    }
  }

  @Override
  public void onScanningFinished() {
    for (ScanningListener scanningListener : scanningListeners) {
      scanningListener.onScanningFinished();
    }
  }

  @Override
  public void onDeviceFound(BluetoothDevice device, short rssi) {
    for (ScanningListener scanningListener : scanningListeners) {
      scanningListener.onDeviceFound(device, rssi);
    }
  }

  // ConnectionListener
  //-------------------------------------------------

  @Override
  public void onConnecting(BluetoothDevice device) {
    for (ConnectionListener connectionListener : connectionListeners) {
      connectionListener.onConnecting(device);
    }
  }

  @Override
  public void onConnected(BluetoothDevice device) {
    for (ConnectionListener connectionListener : connectionListeners) {
      connectionListener.onConnected(device);
    }
  }

  @Override
  public void onDisconnecting(BluetoothDevice device) {
    for (ConnectionListener connectionListener : connectionListeners) {
      connectionListener.onDisconnecting(device);
    }
  }

  @Override
  public void onServicesDiscovered(BluetoothGatt gatt) {
    for (ConnectionListener listener : connectionListeners) {
      listener.onServicesDiscovered(gatt);
    }
  }

  @Override
  public void onRssiUpdated(BluetoothGatt device, int rssi) {
    for (ConnectionListener connectionListener : connectionListeners) {
      connectionListener.onRssiUpdated(device, rssi);
    }
  }

  @Override
  public void onDisconnected(BluetoothDevice device) {
    for (ConnectionListener connectionListener : connectionListeners) {
      connectionListener.onDisconnected(device);
    }
  }

}