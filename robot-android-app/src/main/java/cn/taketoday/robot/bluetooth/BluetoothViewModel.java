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

import android.app.Application;
import android.bluetooth.BluetoothA2dp;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothHeadset;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Looper;

import androidx.lifecycle.AndroidViewModel;
import androidx.lifecycle.MutableLiveData;

import org.jspecify.annotations.Nullable;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import cn.taketoday.robot.ApplicationSupport;
import cn.taketoday.robot.LoggingSupport;

public class BluetoothViewModel extends AndroidViewModel implements BluetoothScanningListener, ApplicationSupport {

  public final MutableLiveData<List<BluetoothItem>> devices = new MutableLiveData<>();

  public final MutableLiveData<Boolean> scanning = new MutableLiveData<>(false);

  public final MutableLiveData<Boolean> connected = new MutableLiveData<>(false);

  public final MutableLiveData<Boolean> bluetoothEnabled = new MutableLiveData<>(false);

  public final MutableLiveData<@Nullable BluetoothProfile> bluetoothProfile = new MutableLiveData<>();

  private final Map<String, BluetoothItem> deviceMap = new HashMap<>();

  private final Handler handler = new Handler(Looper.getMainLooper());

  private final Runnable stopScanTask = this::stopScan;

  private final BluetoothAdapter bluetoothAdapter;

  private final BluetoothManager bluetoothManager;

  private final MyBroadcastReceiver bluetoothBroadcastReceiver = new MyBroadcastReceiver();

  public BluetoothViewModel(Application application) {
    super(application);
    logger("初始化蓝牙控制器: %s", this);

    bluetoothManager = application.getSystemService(BluetoothManager.class);
    bluetoothAdapter = bluetoothManager.getAdapter();
    if (this.bluetoothAdapter == null) {
      makeLongToast("该设备不支持蓝牙");
    }
    else {
      logger("注册蓝牙监听器");
      bluetoothEnabled.setValue(bluetoothAdapter.isEnabled());
      bluetoothAdapter.getProfileProxy(application, new BluetoothProfileListener(), BluetoothProfile.GATT);
//      ContextCompat.registerReceiver(application, bluetoothBroadcastReceiver, getIntentFilter(), ContextCompat.RECEIVER_NOT_EXPORTED);
      application.registerReceiver(bluetoothBroadcastReceiver, createIntentFilter());
    }
  }

  public void startScan() {
    deviceMap.clear();
    updateDeviceList();

    if (bluetoothAdapter.isDiscovering()) {
      bluetoothAdapter.cancelDiscovery();
    }

    bluetoothAdapter.startDiscovery();
    handler.postDelayed(stopScanTask, 12000);
  }

  public void stopScan() {
    if (bluetoothAdapter.isDiscovering()) {
      bluetoothAdapter.cancelDiscovery();
    }
    handler.removeCallbacks(stopScanTask);
  }

  public void clearDevices() {
    deviceMap.clear();
    updateDeviceList();
  }

  private void updateDeviceList() {
    List<BluetoothItem> deviceList = new ArrayList<>(deviceMap.values());
    deviceList.sort(Comparator.comparing(BluetoothItem::getName));
    devices.postValue(deviceList);
  }

  public void loadPairedDevices() {
    updateDeviceList();
  }

  @Override
  protected void onCleared() {
    super.onCleared();
    stopScan();

    destroy();
  }

  public void destroy() {
    try {
      getApplication().unregisterReceiver(bluetoothBroadcastReceiver);
      BluetoothProfile profile = bluetoothProfile.getValue();
      if (profile != null) {
        bluetoothAdapter.closeProfileProxy(BluetoothProfile.GATT, profile);
      }
    }
    catch (IllegalArgumentException e) {
      logger("ex : %s", e);
    }
  }

  @Override
  public void onDeviceFound(BluetoothDevice device) {
    if (device.getName() != null) {
      logger("发现新设备: %s address: %s", device.getName(), device.getAddress());
      BluetoothItem deviceItem = new BluetoothItem(device);
      deviceMap.put(device.getAddress(), deviceItem);
      updateDeviceList();
    }
  }

  @Override
  public void onScanningStarted() {
    scanning.setValue(true);
  }

  @Override
  public void onScanningFinished() {
    scanning.setValue(false);
  }

  @Override
  public Context getContext() {
    return getApplication();
  }

  /**
   * @return true to indicate adapter shutdown has begun, or false on immediate error
   */
  public boolean disableBluetooth() {
    return bluetoothAdapter.disable();
  }

  private IntentFilter createIntentFilter() {
    IntentFilter filter = new IntentFilter();
    filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);//状态改变
    filter.addAction(BluetoothDevice.ACTION_FOUND);//蓝牙发现新设备(未配对)
    filter.addAction(BluetoothDevice.ACTION_ACL_CONNECTED);//设备建立连接
    filter.addAction(BluetoothDevice.ACTION_ACL_DISCONNECTED); //设备断开连接

    filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED); //蓝牙开关状态
    filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_STARTED); //蓝牙开始搜索
    filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED); //蓝牙搜索结束
    filter.addAction(BluetoothAdapter.ACTION_CONNECTION_STATE_CHANGED); //BluetoothAdapter连接状态
    return filter;
  }

  private class BluetoothProfileListener implements BluetoothProfile.ServiceListener {

    @Override
    public void onServiceConnected(int profile, BluetoothProfile proxy) {
      bluetoothProfile.setValue(proxy);
      logger("蓝牙服务已连接 %s", proxy);
    }

    @Override
    public void onServiceDisconnected(int profile) {
      bluetoothProfile.setValue(null);
    }

  }

  private class MyBroadcastReceiver extends BroadcastReceiver implements LoggingSupport {

    private final BluetoothListeners listeners;

    public MyBroadcastReceiver() {
      this.listeners = BluetoothListeners.getInstance();
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
          switch (state) {
            case BluetoothAdapter.STATE_OFF:
              logger("蓝牙关闭 => 事件");
              bluetoothEnabled.setValue(false);
              break;
            case BluetoothAdapter.STATE_ON:
              logger("蓝牙开启 => 事件");
              bluetoothEnabled.setValue(true);
              break;
          }
          listeners.onStatusChange(state);
          break;
        }
        case BluetoothAdapter.ACTION_DISCOVERY_STARTED: { /* 蓝牙开始搜索*/
          logger("蓝牙开始搜索 => 事件");
          listeners.onScanningStarted();
          break;
        }
        case BluetoothAdapter.ACTION_DISCOVERY_FINISHED: { /* 蓝牙搜索结束*/
          logger("蓝牙扫描结束 => 事件");
          listeners.onScanningFinished();
          break;
        }
        case BluetoothDevice.ACTION_FOUND: { /* 发现新设备*/
          final BluetoothDevice dev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
          logger("发现新设备: %s address: %s", dev.getName(), dev.getAddress());
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
          final BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
          if (device != null) {
            logger("设备建立连接：%s => 事件", device.getBondState());
            listeners.onBindingStatusChange(device);
          }
          else {
            logger("ACTION_ACL_CONNECTED: device is null");
          }
          break;
        }
        case BluetoothDevice.ACTION_ACL_DISCONNECTED: { /* 设备断开连接 */
          final BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
          if (device != null) {
            logger("设备断开连接：%s => 事件", device);
            listeners.onDisconnecting(device);
          }
          else {
            logger("ACTION_ACL_DISCONNECTED: device is null");
          }
          break;
        } /* 地蓝牙适配器  BluetoothAdapter连接状态 */
        case BluetoothAdapter.ACTION_CONNECTION_STATE_CHANGED: {
          final BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
          if (device != null) {
            logger("BluetoothAdapter 蓝牙设备: %s, %s => 事件", device.getName(), device.getAddress());
            listeners.onBindingStatusChange(device);
          }
          else {
            logger("ACTION_CONNECTION_STATE_CHANGED: device is null");
          }
          break;
        }
        case BluetoothA2dp.ACTION_CONNECTION_STATE_CHANGED:
        case BluetoothHeadset.ACTION_CONNECTION_STATE_CHANGED: {
          final BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
          if (device != null) {
            switch (intent.getIntExtra(BluetoothProfile.EXTRA_STATE, -1)) {
              case BluetoothProfile.STATE_CONNECTING:
                logger("高质量音频设备: %s 正在连接 => 事件", device.getName());
                listeners.onConnecting(device);
                break;
              case BluetoothProfile.STATE_CONNECTED:
                logger("高质量音频设备: %s 已连接 => 事件", device.getName());
                listeners.onConnected(device);
                break;
              case BluetoothProfile.STATE_DISCONNECTING:
                logger("高质量音频设备: %s 正在断开 => 事件", device.getName());
                listeners.onDisconnecting(device);
                break;
              case BluetoothProfile.STATE_DISCONNECTED:
                logger("高质量音频设备: %s 已经断开 => 事件", device.getName());
                listeners.onDisconnect(device);
                break;
              default:
                break;
            }
          }
          else {
            logger("ACTION_CONNECTION_STATE_CHANGED: device is null");
          }
        }
        default:
          break;
      }
    }
  }

}