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

package cn.taketoday.robot.fragment;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Toast;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresPermission;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.LinearLayoutManager;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import cn.taketoday.robot.LoggingSupport;
import cn.taketoday.robot.R;
import cn.taketoday.robot.RobotController;
import cn.taketoday.robot.bluetooth.BluetoothBindingListener;
import cn.taketoday.robot.bluetooth.BluetoothConnectionListener;
import cn.taketoday.robot.bluetooth.BluetoothController;
import cn.taketoday.robot.bluetooth.BluetoothScanningListener;
import cn.taketoday.robot.bluetooth.BluetoothStatusListener;
import cn.taketoday.robot.bluetooth.CompositeBluetoothListener;
import cn.taketoday.robot.databinding.FragmentDeviceConnectionBinding;
import cn.taketoday.robot.model.DeviceItem;
import cn.taketoday.robot.model.RecyclerViewAdapter;

import static cn.taketoday.robot.util.RobotUtils.showDialog;
import static cn.taketoday.robot.util.ToastUtils.makeToast;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2025/12/20 16:46
 */
public class DeviceConnectionFragment extends ViewBindingFragment<FragmentDeviceConnectionBinding> implements CompoundButton.OnCheckedChangeListener,
        RecyclerViewAdapter.ItemClickListener, BluetoothStatusListener, BluetoothBindingListener,
        BluetoothConnectionListener, BluetoothScanningListener, LoggingSupport {

  private RecyclerViewAdapter bluetoothAdapter;

  public DeviceConnectionFragment() {
    CompositeBluetoothListener listener = CompositeBluetoothListener.getInstance();
    listener.addStatusListener(this);
    listener.addBindingListener(this);
    listener.addScanningListener(this);
    listener.addConnectionListener(this);
  }

  @Override
  protected FragmentDeviceConnectionBinding createBinding(LayoutInflater inflater, ViewGroup container) {
    return FragmentDeviceConnectionBinding.inflate(inflater, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    logger("onViewCreated");

    BluetoothController bluetoothController = getBluetoothController();

    binding.switchRefreshBluetooth.setOnCheckedChangeListener((button, enabled) -> {
      boolean pressed = button.isPressed();
      binding.switchRefreshBluetooth.setPressed(false);

      if (!pressed)
        return;
      if (enabled)
        startDiscovery(bluetoothController);
      else
        stopDiscovery(bluetoothController);
    });

    binding.switchBluetooth.setChecked(bluetoothController.isEnabled()); // 蓝牙初始状态
    binding.switchBluetooth.setOnCheckedChangeListener(this);

    binding.bluetoothList.setNestedScrollingEnabled(true);
    binding.bluetoothList.setLayoutManager(new LinearLayoutManager(getActivity()));

    bluetoothAdapter = new RecyclerViewAdapter();
    bluetoothAdapter.setItemResource(R.layout.recycler_item);
    bluetoothAdapter.setItemClickListener(this);
    binding.bluetoothList.setAdapter(bluetoothAdapter);

    CompositeBluetoothListener.getInstance()
            .onConnectedDevice(bluetoothController.getAllAvailableDevices());

    logger("create end");

  }

  @Override
  public void onCheckedChanged(CompoundButton buttonView, boolean enabled) {
    BluetoothController bluetoothController = getBluetoothController();
    if (enabled) {
      bluetoothController.openBluetooth().onSuccess(status -> {
        requireActivity().runOnUiThread(() -> {
          binding.switchBluetooth.setChecked(status);
          if (!status) {
            showDialog(requireContext(), "错误", "打开蓝牙失败");
          }
        });
      });
    }
    else {
      if (!bluetoothController.close()) {
        binding.switchBluetooth.setChecked(true);
        showDialog(requireContext(), "错误", "关闭蓝牙失败");
      }
    }
  }

  @Override
  public void onItemClickListener(DeviceItem item) {
    logger("列表点击了：%s", item);

    Object nativeResource = item.getNativeResource();
    if (nativeResource instanceof BluetoothDevice bluetoothDevice) {
      BluetoothController bluetoothController = getBluetoothController();
      if (!bluetoothController.isConnected(bluetoothDevice)) {
        bluetoothController.connect(bluetoothDevice);
      }
    }
    else {
      logger("点击错误");
    }
  }

  @Override
  @RequiresPermission(allOf = { Manifest.permission.BLUETOOTH_CONNECT, Manifest.permission.BLUETOOTH_SCAN })
  public void onConnecting(BluetoothDevice device) {
    logger("设备正在连接中: %s", device.getName());
    BluetoothController bluetoothController = getBluetoothController();
    bluetoothController.stopDiscovery();
    applyBluetoothDeviceStatus(device, DeviceItem.STATUS_CONNECTING);
  }

  @Override
  public void onConnected(BluetoothDevice device) {
    logger("设备已连接: %s", device.getName());
    applyBluetoothDeviceStatus(device, DeviceItem.STATUS_CONNECTED);
  }

  @Override
  public void onDisconnecting(BluetoothDevice device) {
    logger("正在断开设备: %s", device.getName());
    applyBluetoothDeviceStatus(device, DeviceItem.STATUS_DISCONNECTING);
  }

  @Override
  public void onDisconnect(BluetoothDevice device) {
    logger("已经断开设备: %s", device.getName());
    applyBluetoothDeviceStatus(device, DeviceItem.STATUS_DISCONNECTED);
  }

  public void applyBluetoothDeviceStatus(BluetoothDevice device, String status) {
    DeviceItem deviceItem = getBluetoothDeviceItem(device);

    if (deviceItem != null) {
      deviceItem.setStatus(status); bluetoothAdapter.notifyDataSetChanged();
    }
  }

  @Override
  public void onConnectedDevice(Collection<BluetoothDevice> devices) {
    logger("onConnectedDevice接收设备: %s", devices.size());
    if (!devices.isEmpty()) {
      List<DeviceItem> items = new ArrayList<>(); for (BluetoothDevice device : devices) {
        items.add(buildBluetoothDeviceItem(device));
      }
      bluetoothAdapter.addAll(items);
    }
  }

  @Override
  public void onScanningStart() {
    binding.switchRefreshBluetooth.setChecked(true);
  }

  @Override
  public void onScanningStop() {
    binding.switchRefreshBluetooth.setChecked(false);

    BluetoothController bluetoothController = getBluetoothController();

    List<BluetoothDevice> devices = bluetoothController.getConnectedDevices();
    Set<BluetoothDevice> bondedDevices = bluetoothController.getBondedDevices();
    List<BluetoothDevice> deviceList = new ArrayList<>(devices);

    deviceList.addAll(bondedDevices);

    logger("停止扫描 :%s", deviceList.size());

    if (deviceList.isEmpty()) {
      return;
    }

    onConnectedDevice(deviceList);
  }

  @Override
  public void onDeviceFound(BluetoothDevice device) {
    logger("========================================");
    makeToast(requireContext(), "发现新设备", Toast.LENGTH_SHORT);

    if (device == null) {
      logger("接收到设备不能为空");
    }
    logger("接收到设备: %s", device);

    bluetoothAdapter.add(buildBluetoothDeviceItem(device));

    logger("========================================");
  }

  protected DeviceItem buildBluetoothDeviceItem(BluetoothDevice device) {
    DeviceItem deviceItem = new DeviceItem(device);

    deviceItem.setIcon(R.mipmap.icon_bluetooth);
    deviceItem.setName(device.getName());
    switch (device.getBondState()) {
      case BluetoothDevice.BOND_NONE:
        deviceItem.setStatus(DeviceItem.STATUS_BOND_NONE);
        break;
      case BluetoothDevice.BOND_BONDING:
        deviceItem.setStatus(DeviceItem.STATUS_BONDING);
        break;
      case BluetoothDevice.BOND_BONDED:
        deviceItem.setStatus(DeviceItem.STATUS_BONDED);
        break;
      default:
        deviceItem.setStatus(DeviceItem.STATUS_UNKNOWN);
        break;
    }

    BluetoothController bluetoothController = getBluetoothController();

    if (bluetoothController.isConnected(device)) {
      deviceItem.setStatus(DeviceItem.STATUS_CONNECTED);
    }

    return deviceItem;
  }

  @Override
  public void onStatusChange(int status) {
    BluetoothController bluetoothController = getBluetoothController();

    switch (status) {
      case BluetoothAdapter.STATE_OFF:
        binding.switchRefreshBluetooth.setPressed(false);
        Toast.makeText(requireContext(), "蓝牙已关闭", Toast.LENGTH_SHORT).show();
        binding.switchRefreshBluetooth.setChecked(bluetoothController.isEnabled());
        binding.switchBluetooth.setChecked(bluetoothController.isEnabled());
        bluetoothAdapter.clear();
        bluetoothAdapter.notifyDataSetChanged();

        stopDiscovery(bluetoothController);

        break;
      case BluetoothAdapter.STATE_ON: {
        Toast.makeText(requireContext(), "蓝牙已打开", Toast.LENGTH_SHORT).show();
        binding.switchBluetooth.setChecked(bluetoothController.isEnabled());
        startDiscovery(bluetoothController); break;
      }
      case BluetoothAdapter.STATE_TURNING_OFF:
      case BluetoothAdapter.STATE_TURNING_ON:
        break;
    }
  }

  public void startDiscovery(BluetoothController bluetoothController) {
    if (bluetoothController.startDiscovery()) {
      logger("成功开始搜索");
      makeToast(requireContext(), "开始搜索", Toast.LENGTH_SHORT);
    }
    else {
      logger("开始搜索失败");
      binding.switchRefreshBluetooth.setChecked(false);
    }
  }

  public void stopDiscovery(BluetoothController bluetoothController) {
    if (bluetoothController.stopDiscovery()) {
      logger("成功暂停搜索");
    }
    else {
      logger("暂停搜索失败");
    }
  }

  @Override
  public void onBindingStatusChange(BluetoothDevice device) {
    if (device.getBondState() == BluetoothDevice.BOND_BONDED) {
      DeviceItem deviceItem = getBluetoothDeviceItem(device);
      deviceItem.setStatus(DeviceItem.STATUS_BONDED); bluetoothAdapter.notifyDataSetChanged();
    }
    else if (device.getBondState() == BluetoothDevice.BOND_BONDING) {//配对中
      DeviceItem deviceItem = getBluetoothDeviceItem(device);
      deviceItem.setStatus(DeviceItem.STATUS_BONDING);
      bluetoothAdapter.notifyDataSetChanged();
    }
    else {//未配对
      DeviceItem deviceItem = getBluetoothDeviceItem(device);
      // showDialog("错误", "不存在该设备: " + device.getName());
      if (deviceItem != null && deviceItem.getStatus().equals(DeviceItem.STATUS_BONDING)) {
        Toast.makeText(requireContext(), "请确认配对设备已打开且在通信范围内", Toast.LENGTH_SHORT).show();
        deviceItem.setStatus(DeviceItem.STATUS_BONDING); bluetoothAdapter.notifyDataSetChanged();
      }
    }
  }

  private DeviceItem getBluetoothDeviceItem(BluetoothDevice device) {
    if (!bluetoothAdapter.isEmpty()) {
      for (DeviceItem data : bluetoothAdapter.getData()) {
        if (data.getNativeResource().equals(device)) {
          return data;
        }
      }
    }
    return buildBluetoothDeviceItem(device);
  }

  protected BluetoothController getBluetoothController() {
    BluetoothController bluetoothController = RobotController.getBluetoothController();
    if (bluetoothController == null) {
      bluetoothController = new BluetoothController((AppCompatActivity) requireActivity());
      RobotController.setBluetoothController(bluetoothController);
    }
    return bluetoothController;
  }

  //
  private ActivityResultLauncher<String[]> requestPermissionLauncher;

  @Override
  public void onCreate(@Nullable Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    requestPermissionLauncher = registerForActivityResult(new ActivityResultContracts.RequestMultiplePermissions(), result -> {
      boolean allGranted = true;
      for (Boolean isGranted : result.values()) {
        if (!isGranted) {
          allGranted = false;
          break;
        }
      }
      if (allGranted) {
        logger("allGranted");
      }
      else {
        logger("allGranted failed");
      }
    });
  }

  private void checkAndRequestBluetoothPermissions() {
    ArrayList<String> permissionsNeeded = new ArrayList<>();

    if (ContextCompat.checkSelfPermission(requireContext(), Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
      permissionsNeeded.add(Manifest.permission.BLUETOOTH_CONNECT);
    }

    requestPermissionLauncher.launch(permissionsNeeded.toArray(new String[0]));
  }

}
