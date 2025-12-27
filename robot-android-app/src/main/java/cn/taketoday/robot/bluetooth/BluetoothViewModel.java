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

import android.Manifest;
import android.app.Application;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Looper;

import androidx.annotation.RequiresPermission;
import androidx.core.content.ContextCompat;
import androidx.lifecycle.AndroidViewModel;
import androidx.lifecycle.MutableLiveData;

import org.jspecify.annotations.Nullable;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import cn.taketoday.robot.ApplicationSupport;

public class BluetoothViewModel extends AndroidViewModel implements BluetoothScanningListener, ApplicationSupport {

  public final MutableLiveData<List<BluetoothDeviceItem>> devices = new MutableLiveData<>();

  public final MutableLiveData<Boolean> scanning = new MutableLiveData<>(false);

  public final MutableLiveData<Boolean> connected = new MutableLiveData<>(false);

  public final MutableLiveData<Boolean> bluetoothEnabled = new MutableLiveData<>(false);

  public final MutableLiveData<@Nullable BluetoothProfile> bluetoothProfile = new MutableLiveData<>();

  private final Map<String, BluetoothDeviceItem> deviceMap = new HashMap<>();

  private final Handler handler = new Handler(Looper.getMainLooper());

  private final Runnable stopScanTask = this::stopScan;

  private final BluetoothAdapter bluetoothAdapter;

  private final BluetoothManager bluetoothManager;

  private final BluetoothBroadcastReceiver bluetoothBroadcastReceiver = new BluetoothBroadcastReceiver();

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
      bluetoothAdapter.getProfileProxy(application, new BluetoothProfileListener(), BluetoothProfile.GATT);
      ContextCompat.registerReceiver(application, bluetoothBroadcastReceiver, getIntentFilter(), ContextCompat.RECEIVER_NOT_EXPORTED);
    }
  }

  @RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
  public void startScan() {
    if (bluetoothAdapter.isDiscovering()) {
      bluetoothAdapter.cancelDiscovery();
    }

    bluetoothAdapter.startDiscovery();

    deviceMap.clear();
    updateDeviceList();

    handler.postDelayed(stopScanTask, 12000);
  }

  @RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
  public void stopScan() {
    bluetoothController.stopDiscovery();

    handler.removeCallbacks(stopScanTask);
  }

  public void clearDevices() {
    deviceMap.clear();
    updateDeviceList();
  }

  private void updateDeviceList() {
    List<BluetoothDeviceItem> deviceList = new ArrayList<>(deviceMap.values());
    devices.postValue(deviceList);
  }

  @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
  public void loadPairedDevices() {
    updateDeviceList();
  }

  @RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
  @Override
  protected void onCleared() {
    super.onCleared();
    stopScan();
  }

  @Override
  public void onDeviceFound(BluetoothDevice device) {
    BluetoothDeviceItem deviceItem = new BluetoothDeviceItem(device);
    deviceMap.put(device.getAddress(), deviceItem);
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

  private IntentFilter getIntentFilter() {
    IntentFilter filter = new IntentFilter();
    filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);//状态改变
    filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED); //蓝牙开关状态
    filter.addAction(BluetoothDevice.ACTION_FOUND);//蓝牙发现新设备(未配对)
    filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_STARTED); //蓝牙开始搜索
    filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED); //蓝牙搜索结束
    filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED); //设备配对状态改变
    filter.addAction(BluetoothDevice.ACTION_ACL_CONNECTED);//设备建立连接
    filter.addAction(BluetoothDevice.ACTION_ACL_DISCONNECTED); //设备断开连接
    filter.addAction(BluetoothAdapter.ACTION_CONNECTION_STATE_CHANGED); //BluetoothAdapter连接状态
    return filter;
  }

  /**
   * @return true to indicate adapter shutdown has begun, or false on immediate error
   */
  public boolean disableBluetooth() {
    return bluetoothAdapter.disable();
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

}