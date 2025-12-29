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

import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.RequiresPermission;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;
import androidx.lifecycle.ViewModelProvider;
import androidx.recyclerview.widget.LinearLayoutManager;

import com.google.android.material.snackbar.Snackbar;

import org.jspecify.annotations.Nullable;

import java.util.ArrayList;

import cn.taketoday.robot.LoggingSupport;
import cn.taketoday.robot.R;
import cn.taketoday.robot.bluetooth.BindingStatusListener;
import cn.taketoday.robot.bluetooth.BluetoothItem;
import cn.taketoday.robot.bluetooth.BluetoothListeners;
import cn.taketoday.robot.bluetooth.BluetoothViewModel;
import cn.taketoday.robot.bluetooth.ConnectionListener;
import cn.taketoday.robot.databinding.FragmentDeviceConnectionBinding;
import cn.taketoday.robot.model.BluetoothDeviceListAdapter;
import cn.taketoday.robot.model.BluetoothItemClickListener;
import cn.taketoday.robot.model.DeviceItem;

import static cn.taketoday.robot.util.RobotUtils.showDialog;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2025/12/20 16:46
 */
public class DeviceConnectionFragment extends ViewBindingFragment<FragmentDeviceConnectionBinding> implements CompoundButton.OnCheckedChangeListener,
        BluetoothItemClickListener, BindingStatusListener, ConnectionListener, LoggingSupport, ActivityResultCallback<ActivityResult> {

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
    debug("onViewCreated");
    viewModel = new ViewModelProvider(this).get(BluetoothViewModel.class);

    BluetoothListeners listeners = getBluetoothListeners();
    listeners.addScanningListener(viewModel);
    listeners.addConnectionListener(this);

    binding.swipeRefresh.setOnRefreshListener(viewModel::startScan);

    binding.switchBluetooth.setOnCheckedChangeListener(this);

    binding.bluetoothList.setNestedScrollingEnabled(true);
    binding.bluetoothList.setLayoutManager(new LinearLayoutManager(getActivity()));

    bluetoothAdapter = new BluetoothDeviceListAdapter(this);
    binding.bluetoothList.setAdapter(bluetoothAdapter);

    viewModel.devices.observe(getViewLifecycleOwner(), bluetoothAdapter::submitList);

    viewModel.connected.observe(getViewLifecycleOwner(), connected -> {
      if (connected) {
        Snackbar.make(view, "连接成功", Snackbar.LENGTH_SHORT).show();
      }
      else {
        Snackbar.make(view, "连接断开", Snackbar.LENGTH_SHORT).show();
      }
    });

    viewModel.scanning.observe(getViewLifecycleOwner(), scanning -> {
      if (scanning) {
        debug("成功开始搜索");
        Snackbar.make(view, "开始搜索", Snackbar.LENGTH_SHORT).show();
      }
      else {
        binding.swipeRefresh.setRefreshing(false);
      }
    });

    viewModel.bluetoothEnabled.observe(getViewLifecycleOwner(), enabled -> {
      binding.switchBluetooth.setChecked(enabled);
      if (enabled) {
        binding.swipeRefresh.setRefreshing(true);
        Snackbar.make(view, "蓝牙已打开", Snackbar.LENGTH_LONG).show();

        viewModel.startScan();
      }
      else {
        binding.swipeRefresh.setRefreshing(false);
        Snackbar.make(view, "蓝牙已关闭", Snackbar.LENGTH_LONG).show();

        bluetoothAdapter.clear();
        viewModel.stopScan();
      }
    });

    debug("create end");
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
  public void onBluetoothItemClickListener(View view, BluetoothItem item) {
    debug("列表点击了：%s", item.getName());
    viewModel.connect(view, item);
  }

  @Override
  @RequiresPermission(allOf = { Manifest.permission.BLUETOOTH_CONNECT, Manifest.permission.BLUETOOTH_SCAN })
  public void onConnecting(BluetoothDevice device) {
    debug("设备正在连接中: %s", device.getName());

  }

  @Override
  public void onConnected(BluetoothDevice device) {
    debug("设备已连接: %s", device.getName());

  }

  @Override
  public void onDisconnecting(BluetoothDevice device) {
    debug("正在断开设备: %s", device.getName());
  }

  @Override
  public void onDataReceived(BluetoothDevice device, byte[] data) {

  }

  @Override
  public void onRssiUpdated(BluetoothDevice device, int rssi) {

  }

  @Override
  public void onDisconnected(BluetoothDevice device) {
    debug("已经断开设备: %s", device.getName());
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
        deviceItem.setStatus(BluetoothItem.STATUS_BOND_NONE);
        break;
      case BluetoothDevice.BOND_BONDING:
        deviceItem.setStatus(BluetoothItem.STATUS_BONDING);
        break;
      case BluetoothDevice.BOND_BONDED:
        deviceItem.setStatus(BluetoothItem.STATUS_BONDED);
        break;
      default:
        deviceItem.setStatus(BluetoothItem.STATUS_UNKNOWN);
        break;
    }

    return deviceItem;
  }

  @Override
  public void onBindingStatusChange(BluetoothDevice device) {

  }

  private DeviceItem getBluetoothDeviceItem(BluetoothDevice device) {
    return buildBluetoothDeviceItem(device);
  }

  protected BluetoothListeners getBluetoothListeners() {
    return BluetoothListeners.getInstance();
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
        debug("allGranted");
      }
      else {
        debug("allGranted failed");
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
