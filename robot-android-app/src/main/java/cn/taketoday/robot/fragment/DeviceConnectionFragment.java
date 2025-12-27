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
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Toast;

import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.RequiresPermission;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;
import androidx.lifecycle.ViewModelProvider;
import androidx.recyclerview.widget.LinearLayoutManager;

import org.jspecify.annotations.Nullable;

import java.util.ArrayList;

import cn.taketoday.robot.LoggingSupport;
import cn.taketoday.robot.R;
import cn.taketoday.robot.bluetooth.BluetoothBindingListener;
import cn.taketoday.robot.bluetooth.BluetoothConnectionListener;
import cn.taketoday.robot.bluetooth.BluetoothController;
import cn.taketoday.robot.bluetooth.BluetoothViewModel;
import cn.taketoday.robot.databinding.FragmentDeviceConnectionBinding;
import cn.taketoday.robot.model.BluetoothDeviceClickListener;
import cn.taketoday.robot.model.BluetoothDeviceListAdapter;
import cn.taketoday.robot.model.DeviceItem;

import static cn.taketoday.robot.util.RobotUtils.showDialog;
import static cn.taketoday.robot.util.ToastUtils.makeToast;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2025/12/20 16:46
 */
public class DeviceConnectionFragment extends ViewBindingFragment<FragmentDeviceConnectionBinding> implements CompoundButton.OnCheckedChangeListener,
        BluetoothDeviceClickListener, BluetoothBindingListener, BluetoothConnectionListener, LoggingSupport, ActivityResultCallback<ActivityResult> {

  private BluetoothDeviceListAdapter bluetoothAdapter;

  private BluetoothViewModel viewModel;

  private final ActivityResultLauncher<Intent> enableBluetoothLauncher;

  public DeviceConnectionFragment() {
    this.enableBluetoothLauncher = registerForActivityResult(new ActivityResultContracts.StartActivityForResult(), this);
  }

  @Override
  protected FragmentDeviceConnectionBinding createBinding(LayoutInflater inflater, @Nullable ViewGroup container) {
    return FragmentDeviceConnectionBinding.inflate(inflater, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    logger("onViewCreated");
    viewModel = new ViewModelProvider(this).get(BluetoothViewModel.class);

    BluetoothController bluetoothController = getBluetoothController();
    bluetoothController.addScanningListener(viewModel);
    bluetoothController.addConnectionListener(this);

    binding.swipeRefresh.setOnRefreshListener(viewModel::startScan);

    binding.switchBluetooth.setChecked(bluetoothController.isEnabled()); // 蓝牙初始状态
    binding.switchBluetooth.setOnCheckedChangeListener(this);

    binding.bluetoothList.setNestedScrollingEnabled(true);
    binding.bluetoothList.setLayoutManager(new LinearLayoutManager(getActivity()));

    bluetoothAdapter = new BluetoothDeviceListAdapter(this);
    binding.bluetoothList.setAdapter(bluetoothAdapter);

    viewModel.scanning.observe(this, scanning -> {
      if (scanning) {
        logger("成功开始搜索");
        makeToast(requireContext(), "开始搜索", Toast.LENGTH_SHORT);
      }
      else {
        binding.swipeRefresh.setRefreshing(false);
      }
    });

    viewModel.bluetoothEnabled.observe(this, enabled -> {
      binding.switchBluetooth.setChecked(enabled);
      if (enabled) {
        Toast.makeText(requireContext(), "蓝牙已打开", Toast.LENGTH_SHORT).show();
        viewModel.startScan();
      }
      else {
        binding.swipeRefresh.setRefreshing(false);
        Toast.makeText(requireContext(), "蓝牙已关闭", Toast.LENGTH_SHORT).show();

        bluetoothAdapter.clear();
        viewModel.stopScan();
      }
    });

    logger("create end");
  }

  @Override
  public void onCheckedChanged(CompoundButton buttonView, boolean enabled) {
    if (enabled) {
      Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
      enableBluetoothLauncher.launch(enableBtIntent);
    }
    else {
      if (!viewModel.disableBluetooth()) {
        binding.switchBluetooth.setChecked(true);
        showDialog(requireContext(), "错误", "关闭蓝牙失败");
      }
    }
  }

  @Override
  public void onBluetoothDeviceClickListener(BluetoothDevice device) {
    logger("列表点击了：%s", device);

    BluetoothController bluetoothController = getBluetoothController();
    if (!bluetoothController.isConnected(device)) {
      bluetoothController.connect(device);
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
      deviceItem.setStatus(status);
      bluetoothAdapter.notifyDataSetChanged();
    }
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
  public void onBindingStatusChange(BluetoothDevice device) {
    if (device.getBondState() == BluetoothDevice.BOND_BONDED) {
      DeviceItem deviceItem = getBluetoothDeviceItem(device);
      deviceItem.setStatus(DeviceItem.STATUS_BONDED);
      bluetoothAdapter.notifyDataSetChanged();
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
        deviceItem.setStatus(DeviceItem.STATUS_BONDING);
        bluetoothAdapter.notifyDataSetChanged();
      }
    }
  }

  private DeviceItem getBluetoothDeviceItem(BluetoothDevice device) {
//    if (!bluetoothAdapter.isEmpty()) {
//      for (DeviceItem data : bluetoothAdapter.getData()) {
//        if (data.getNativeResource().equals(device)) {
//          return data;
//        }
//      }
//    }
    return buildBluetoothDeviceItem(device);
  }

  protected BluetoothController getBluetoothController() {
    return BluetoothController.getInstance();
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

  @Override
  public void onActivityResult(ActivityResult result) {
    if (result.getResultCode() != AppCompatActivity.RESULT_OK) {
      showDialog(requireContext(), "错误", "打开蓝牙失败");
      binding.switchBluetooth.setChecked(false);
    }
  }

}
