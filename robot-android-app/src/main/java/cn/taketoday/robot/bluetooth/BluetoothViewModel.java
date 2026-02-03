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

import android.app.Application;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
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
import androidx.lifecycle.ViewModelProvider;
import androidx.lifecycle.ViewModelStoreOwner;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import cn.taketoday.robot.ApplicationSupport;
import cn.taketoday.robot.LoggingSupport;
import cn.taketoday.robot.model.DataHandler;

public class BluetoothViewModel extends AndroidViewModel implements ScanningListener, ApplicationSupport, ConnectionListener {

  public final MutableLiveData<List<BluetoothItem>> devices = new MutableLiveData<>();

  public final MutableLiveData<Boolean> scanning = new MutableLiveData<>(false);

  public final MutableLiveData<Boolean> connected = new MutableLiveData<>(false);

  public final MutableLiveData<Boolean> robotConnected = new MutableLiveData<>(false);

  public final MutableLiveData<Boolean> bluetoothEnabled = new MutableLiveData<>(false);

  private final Map<String, BluetoothItem> deviceMap = new HashMap<>();

  private final Handler handler = new Handler(Looper.getMainLooper());

  private final Runnable stopScanTask = this::stopScan;

  private final BluetoothAdapter bluetoothAdapter;

  private final BluetoothManager bluetoothManager;

  private final BluetoothLeService bluetoothLeService;

  private final MyBroadcastReceiver bluetoothBroadcastReceiver;

  public BluetoothViewModel(Application application, DataHandler handler) {
    super(application);
    debug("初始化蓝牙控制器: %s", this);

    this.bluetoothManager = application.getSystemService(BluetoothManager.class);
    this.bluetoothAdapter = bluetoothManager.getAdapter();
    this.bluetoothLeService = new BluetoothLeService(application, handler);
    this.bluetoothBroadcastReceiver = new MyBroadcastReceiver(bluetoothLeService);
    if (this.bluetoothAdapter == null) {
      makeLongToast("该设备不支持蓝牙");
    }
    else {
      debug("注册蓝牙监听器");
      scanning.setValue(bluetoothAdapter.isDiscovering());
      bluetoothEnabled.setValue(bluetoothAdapter.isEnabled());
      application.registerReceiver(bluetoothBroadcastReceiver, createIntentFilter());
    }

    bluetoothLeService.addConnectionListener(this);
    bluetoothLeService.addScanningListener(this);
    addCloseable(this::destroy);
  }

  public void startScan() {
    deviceMap.clear();
    collectDevices();

    if (bluetoothAdapter.isDiscovering()) {
      bluetoothAdapter.cancelDiscovery();
    }

    bluetoothAdapter.startDiscovery();
    handler.postDelayed(stopScanTask, 2000);
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

  private void collectDevices() {
    for (BluetoothDevice bondedDevice : bluetoothAdapter.getBondedDevices()) {
      putDevice(bondedDevice);
    }

    bluetoothManager.getConnectedDevices(BluetoothProfile.GATT).forEach(this::putDevice);
    bluetoothManager.getConnectedDevices(BluetoothProfile.GATT_SERVER).forEach(this::putDevice);

    updateDeviceList();
  }

  private void updateDeviceList() {
    List<BluetoothItem> deviceList = new ArrayList<>(deviceMap.values());
    deviceList.sort(Comparator.comparing(BluetoothItem::getRssi).thenComparing(BluetoothItem::getName));
    devices.postValue(deviceList);
  }

  @Override
  protected void onCleared() {
    super.onCleared();
    stopScan();
  }

  public void destroy() {
    debug("destroy %s", this);
    try {
      bluetoothLeService.close();
      getApplication().unregisterReceiver(bluetoothBroadcastReceiver);
    }
    catch (IllegalArgumentException e) {
      debug("ex : %s", e);
    }
  }

  @Override
  public void onDeviceFound(BluetoothDevice device, short rssi) {
    if (device.getName() != null && device.getType() == BluetoothDevice.DEVICE_TYPE_LE) {
      debug("发现新设备: %s address: %s, rssi: %s", device.getName(), device.getAddress(), rssi);
      putDevice(device, rssi);
      updateDeviceList();
    }
  }

  private void putDevice(BluetoothDevice device) {
    putDevice(device, 0);
  }

  private void putDevice(BluetoothDevice device, int rssi) {
    BluetoothItem deviceItem = new BluetoothItem(device, rssi, isConnected(device));
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

  /**
   * @return true to indicate adapter shutdown has begun, or false on immediate error
   */
  public boolean disableBluetooth() {
    return bluetoothAdapter.disable();
  }

  public boolean connect(BluetoothDevice item) {
    return bluetoothLeService.connectWithAutoReconnect(item);
  }

  @Override
  public void onConnected(BluetoothDevice device) {
    connected.postValue(true);
    bluetoothLeService.readRemoteRssi();
  }

  @Override
  public void onServicesDiscovered(BluetoothGatt gatt) {
    bluetoothLeService.setCharacteristicNotification(true);
    boolean robot = bluetoothLeService.isRobot();
    robotConnected.postValue(robot);
    if (!robot) {
      warn("连接的不是机器人");
    }

    if (isDebugEnabled()) {
      List<BluetoothGattService> services = gatt.getServices();
      for (BluetoothGattService service : services) {
        debug("Service: %s", service.getUuid());

        // 打印服务下的特征
        List<BluetoothGattCharacteristic> characteristics = service.getCharacteristics();
        for (BluetoothGattCharacteristic characteristic : characteristics) {
          debug("  Characteristic: %s", characteristic.getUuid());

          // 打印特征下的描述符
          List<BluetoothGattDescriptor> descriptors = characteristic.getDescriptors();
          for (BluetoothGattDescriptor descriptor : descriptors) {
            debug("    Descriptor: %s", descriptor.getUuid());
          }
        }
      }
    }

  }

  @Override
  public void onDisconnected(BluetoothDevice device) {
    connected.postValue(false);
  }

  @Override
  public void onRssiUpdated(BluetoothGatt gatt, int rssi) {
    putDevice(gatt.getDevice(), rssi);
    updateDeviceList();
  }

  private boolean isConnected(BluetoothDevice device) {
    return bluetoothManager.getConnectionState(device, BluetoothProfile.GATT) == BluetoothProfile.STATE_CONNECTED;
  }

  private IntentFilter createIntentFilter() {
    IntentFilter filter = new IntentFilter();
    filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);//状态改变
    filter.addAction(BluetoothDevice.ACTION_FOUND);//蓝牙发现新设备(未配对)

    filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED); //蓝牙开关状态
    filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_STARTED); //蓝牙开始搜索
    filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED); //蓝牙搜索结束
    return filter;
  }

  public static BluetoothViewModel getInstance(ViewModelStoreOwner store) {
    return new ViewModelProvider(store).get(BluetoothViewModel.class);
  }

  private class MyBroadcastReceiver extends BroadcastReceiver implements LoggingSupport {

    private final BluetoothListeners listeners;

    public MyBroadcastReceiver(BluetoothListeners listeners) {
      this.listeners = listeners;
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
              debug("蓝牙关闭 => 事件");
              bluetoothEnabled.setValue(false);
              break;
            case BluetoothAdapter.STATE_ON:
              debug("蓝牙开启 => 事件");
              bluetoothEnabled.setValue(true);
              break;
          }
          listeners.onStatusChange(state);
          break;
        }
        case BluetoothAdapter.ACTION_DISCOVERY_STARTED: { /* 蓝牙开始搜索*/
          debug("蓝牙开始搜索 => 事件");
          listeners.onScanningStarted();
          break;
        }
        case BluetoothAdapter.ACTION_DISCOVERY_FINISHED: { /* 蓝牙搜索结束*/
          debug("蓝牙扫描结束 => 事件");
          listeners.onScanningFinished();
          break;
        }
        case BluetoothDevice.ACTION_FOUND: {
          BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
          short rssi = intent.getShortExtra(BluetoothDevice.EXTRA_RSSI, Short.MIN_VALUE);
          listeners.onDeviceFound(device, rssi);
          break;
        }
        case BluetoothDevice.ACTION_BOND_STATE_CHANGED: {
          final BluetoothDevice dev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
          listeners.onBindingStatusChange(dev);
          debug("设备配对状态改变：%s => 事件", dev.getBondState());
          break;
        }
        default:
          break;
      }
    }
  }

}