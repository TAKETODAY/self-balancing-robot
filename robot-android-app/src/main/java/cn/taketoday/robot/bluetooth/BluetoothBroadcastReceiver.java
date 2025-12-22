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

import android.bluetooth.BluetoothA2dp;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothHeadset;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import cn.taketoday.robot.LoggingSupport;

/**
 * BluetoothBroadcastReceiver
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public class BluetoothBroadcastReceiver extends BroadcastReceiver implements LoggingSupport {

  private final BluetoothController bluetoothController;

  private final CompositeBluetoothListener listeners = CompositeBluetoothListener.getInstance();

  public BluetoothBroadcastReceiver(final BluetoothController bluetoothController) {
    this.bluetoothController = bluetoothController;
  }

  @Override
  public void onReceive(final Context context, final Intent intent) {
    final String action = intent.getAction();
    if (action == null) {
      return;
    }
    /*蓝牙开关状态 STATE_OFF: 蓝牙关闭 STATE_ON :蓝牙打开 STATE_TURNING_OFF: 蓝牙正在关闭 STATE_TURNING_ON :蓝牙正在打开 */
    switch (action) {
      case BluetoothAdapter.ACTION_STATE_CHANGED: {
        final int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, 0);
        listeners.onStatusChange(state);
        break;
      }
      case BluetoothAdapter.ACTION_DISCOVERY_STARTED: { /* 蓝牙开始搜索*/
        logger("蓝牙开始搜索 => 事件");
        listeners.onScanningStart();
        break;
      }
      case BluetoothAdapter.ACTION_DISCOVERY_FINISHED: { /* 蓝牙搜索结束*/
        logger("蓝牙扫描结束 => 事件");
        listeners.onScanningStop();
        break;
      }
      case BluetoothDevice.ACTION_FOUND: { /* 发现新设备*/
        final BluetoothDevice dev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
        logger("发现新设备: %s address: %s => 事件", dev.getName(), dev.getAddress());
        listeners.onDeviceFound(dev);
        break;
      }
      case BluetoothDevice.ACTION_BOND_STATE_CHANGED: {
        final BluetoothDevice dev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
        listeners.onBindingStatusChange(dev);
        logger("设备配对状态改变：%s => 事件", dev.getBondState());
        break;
      }/*设备建立连接 STATE_DISCONNECTED 未连接 STATE_CONNECTING 连接中 STATE_CONNECTED连接成功 */
      case BluetoothDevice.ACTION_ACL_CONNECTED: {
        final BluetoothDevice dev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
        logger("设备建立连接：%s => 事件", dev.getBondState());
        listeners.onBindingStatusChange(dev);
        break;
      }
      case BluetoothDevice.ACTION_ACL_DISCONNECTED: { /* 设备断开连接 */
        final BluetoothDevice dev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
        logger("设备断开连接：%s => 事件", dev);
        listeners.onDisconnecting(dev);
        break;
      } /* 地蓝牙适配器  BluetoothAdapter连接状态 */
      case BluetoothAdapter.ACTION_CONNECTION_STATE_CHANGED: {
        final BluetoothDevice dev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
        // logger("Adapter STATE: %s", intent.getIntExtra(BluetoothAdapter.EXTRA_CONNECTION_STATE, 0));
        logger("BluetoothAdapter 蓝牙设备: %s, %s => 事件", dev.getName(), dev.getAddress());
        listeners.onBindingStatusChange(dev);
        break;
      }
      case BluetoothA2dp.ACTION_CONNECTION_STATE_CHANGED:
      case BluetoothHeadset.ACTION_CONNECTION_STATE_CHANGED: {
        final BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
        //  logger("Headset STATE: %s", intent.getIntExtra(BluetoothHeadset.EXTRA_STATE, 0));
        switch (intent.getIntExtra(BluetoothA2dp.EXTRA_STATE, -1)) {
          case BluetoothA2dp.STATE_CONNECTING:
            logger("高质量音频设备: %s 正在连接 => 事件", device.getName());
            listeners.onConnecting(device);
            break;
          case BluetoothA2dp.STATE_CONNECTED:
            logger("高质量音频设备: %s 已连接 => 事件", device.getName());
            listeners.onConnected(device);
            break;
          case BluetoothA2dp.STATE_DISCONNECTING:
            logger("高质量音频设备: %s 正在断开 => 事件", device.getName());
            listeners.onDisconnecting(device);
            break;
          case BluetoothA2dp.STATE_DISCONNECTED:
            logger("高质量音频设备: %s 已经断开 => 事件", device.getName());
            listeners.onDisconnect(device);
            break;
          default:
            break;
        }
      }
      default:
        break;
    }
  }
}
