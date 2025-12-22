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

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AppCompatActivity;

import org.jspecify.annotations.Nullable;

import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import cn.taketoday.robot.ApplicationSupport;
import infra.util.ObjectUtils;
import infra.util.StringUtils;
import infra.util.concurrent.Future;
import infra.util.concurrent.Promise;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public class BluetoothController implements ApplicationSupport {

  private final AppCompatActivity activity;

  private final BluetoothAdapter bluetoothAdapter;

  private final BluetoothManager bluetoothManager;

  private final EnableBluetoothLauncher enableBluetoothLauncher;

  private final BluetoothProfileListener bluetoothProfileListener = new BluetoothProfileListener(this);

  private final BluetoothBroadcastReceiver bluetoothBroadcastReceiver = new BluetoothBroadcastReceiver(this);

  private @Nullable BluetoothProfile bluetoothProfile;

  private final Set<BluetoothDevice> bluetoothDevices = new HashSet<>();

  public BluetoothController(AppCompatActivity activity) {
    this.activity = activity;
    logger("初始化蓝牙控制器: %s", this);

    bluetoothManager = activity.getSystemService(BluetoothManager.class);
    bluetoothAdapter = bluetoothManager.getAdapter();
    if (this.bluetoothAdapter == null) {
      makeLongToast("该设备不支持蓝牙");
    }
    else {
      logger("注册蓝牙监听器");
      this.bluetoothAdapter.getProfileProxy(activity, bluetoothProfileListener, BluetoothProfile.A2DP);
      activity.registerReceiver(bluetoothBroadcastReceiver, getIntentFilter());
    }

    this.enableBluetoothLauncher = new EnableBluetoothLauncher(activity);
    logger("初始化蓝牙控制器完毕!");
  }

  @Override
  public Context getContext() {
    return activity;
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

  public final BluetoothAdapter getBluetoothAdapter() {
    return this.bluetoothAdapter;
  }

  // --------------------------------------------------

  public String getLocalName() {//获取本地蓝牙名称
    return bluetoothAdapter.getName();
  }

  public String getLocalAddress() {//获取本地蓝牙地址
    return bluetoothAdapter.getAddress();
  }

  public Future<Boolean> openBluetooth() {
    if (bluetoothAdapter.isEnabled()) {
      return Future.ok(true);
    }
    return enableBluetoothLauncher.enable();
  }

  public boolean close() {
    return bluetoothAdapter.disable();
  }

  public boolean startDiscovery() {
    if (bluetoothAdapter.isDiscovering()) {
      bluetoothAdapter.cancelDiscovery();
    }
    return bluetoothAdapter.startDiscovery();
  }

  public boolean stopDiscovery() {
    return !bluetoothAdapter.isDiscovering() || bluetoothAdapter.cancelDiscovery();
  }

  public @Nullable BluetoothProfile getBluetoothProfile() {
    if (bluetoothProfile == null) {
      logger("蓝牙服务还未连接");
    }
    return bluetoothProfile;
  }

  public Set<BluetoothDevice> getBondedDevices() {
    return bluetoothAdapter.getBondedDevices();
  }

  public List<BluetoothDevice> getConnectedDevices() {
    final BluetoothProfile bluetoothProfile = getBluetoothProfile();
    if (bluetoothProfile == null) {
      return Collections.emptyList();
    }
    return bluetoothProfile.getConnectedDevices();
  }

  public Set<BluetoothDevice> getBluetoothDevices() {
    return bluetoothDevices;
  }

  public Set<BluetoothDevice> getAllAvailableDevices() {
    Set<BluetoothDevice> ret = new HashSet<>();

    ret.addAll(getConnectedDevices());
    ret.addAll(getBluetoothDevices());
    ret.addAll(getBondedDevices());
    return ret;
  }

  public boolean connect(BluetoothDevice device) {
    try {
      //todo
      return false;
    }
    catch (Exception e) {
      showErrorDialog(e);
      return false;
    }
  }

  public boolean isEnabled() {
    return bluetoothAdapter.isEnabled();
  }

  public boolean isConnected(final BluetoothDevice device) {
    final BluetoothProfile profile = this.bluetoothProfile;
    if (profile != null && profile.getConnectionState(device) == BluetoothProfile.STATE_CONNECTED) {
      logger("已经连接 蓝牙名字: %s", device.getName());

      List<BluetoothDevice> bluetoothDeviceList = profile.getConnectedDevices();
      if (ObjectUtils.isNotEmpty(bluetoothDeviceList)) {
        final String address = device.getAddress();

        for (final BluetoothDevice bluetoothDevice : bluetoothDeviceList) {
          if (StringUtils.isNotEmpty(address) && address.equals(bluetoothDevice.getAddress())) {
            return true;
          }
        }
      }
    }

    return false;
  }

  public void destroy() {
    try {
      activity.unregisterReceiver(bluetoothBroadcastReceiver);
      bluetoothAdapter.closeProfileProxy(BluetoothProfile.A2DP, bluetoothProfile);
    }
    catch (IllegalArgumentException e) {
      logger("ex : %s", e);
    }
  }

  protected static class BluetoothProfileListener implements BluetoothProfile.ServiceListener {

    private final BluetoothController bluetoothController;

    public BluetoothProfileListener(final BluetoothController bluetoothController) {
      this.bluetoothController = bluetoothController;
    }

    @Override
    public void onServiceConnected(int profile, BluetoothProfile proxy) {
      bluetoothController.bluetoothProfile = proxy;

      final BluetoothController bluetoothController = this.bluetoothController;

      bluetoothController.logger("蓝牙服务已连接, A2DP,: %s", proxy);
      final List<BluetoothDevice> deviceList = proxy.getConnectedDevices();
      if (deviceList.isEmpty()) {
        bluetoothController.logger("音频蓝牙设备为空");
      }
      else {
        bluetoothController.logger("音频蓝牙设备: %s", deviceList);
        bluetoothController.bluetoothDevices.addAll(deviceList);
      }
    }

    @Override
    public void onServiceDisconnected(int profile) {
      bluetoothController.logger("蓝牙服务断开");
    }
  }

  private static class EnableBluetoothLauncher implements ActivityResultCallback<ActivityResult> {

    private final ActivityResultLauncher<Intent> launcher;

    private @Nullable Promise<Boolean> promise;

    private EnableBluetoothLauncher(AppCompatActivity activity) {
      this.launcher = activity.registerForActivityResult(new ActivityResultContracts.StartActivityForResult(), this);
    }

    public Promise<Boolean> enable() {
      promise = Promise.forPromise();
      Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
      launcher.launch(enableBtIntent);
      return promise;
    }

    @Override
    public void onActivityResult(ActivityResult result) {
      if (promise != null) {
        promise.trySuccess(result.getResultCode() == AppCompatActivity.RESULT_OK);
        promise = null;
      }
    }

  }

}
