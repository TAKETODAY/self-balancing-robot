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

package cn.taketoday.robot.fragment;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
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
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;
import androidx.lifecycle.ViewModelProvider;
import androidx.recyclerview.widget.LinearLayoutManager;

import com.google.android.material.snackbar.Snackbar;

import org.jspecify.annotations.Nullable;

import java.util.ArrayList;

import cn.taketoday.robot.LoggingSupport;
import cn.taketoday.robot.bluetooth.BluetoothItem;
import cn.taketoday.robot.bluetooth.BluetoothViewModel;
import cn.taketoday.robot.databinding.FragmentDeviceConnectionBinding;
import cn.taketoday.robot.model.BluetoothDeviceListAdapter;
import cn.taketoday.robot.model.BluetoothItemClickListener;

import static cn.taketoday.robot.util.RobotUtils.showDialog;

/**
 * A fragment for managing Bluetooth device connections.
 * <p>
 * This fragment provides a user interface for enabling/disabling Bluetooth,
 * scanning for nearby devices, and establishing a connection. It displays a list
 * of discovered devices and allows the user to initiate a connection by tapping
 * on a device. The UI state, such as scanning status and connection status, is
 * managed through a {@link BluetoothViewModel}.
 * </p>
 * <p>
 * It handles Bluetooth permissions and user requests to enable the Bluetooth adapter.
 * The list of devices is presented using a {@link androidx.recyclerview.widget.RecyclerView}
 * populated by a {@link BluetoothDeviceListAdapter}. User interactions, like toggling
 * Bluetooth or selecting a device, are delegated to the {@code BluetoothViewModel}.
 * </p>
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @see BluetoothViewModel
 * @see ViewBindingFragment
 * @see BluetoothDeviceListAdapter
 * @since 1.0 2025/12/20 16:46
 */
public class DeviceConnectionFragment extends ViewBindingFragment<FragmentDeviceConnectionBinding>
        implements CompoundButton.OnCheckedChangeListener, BluetoothItemClickListener, LoggingSupport, ActivityResultCallback<ActivityResult> {

  private BluetoothDeviceListAdapter bluetoothAdapter;

  private BluetoothViewModel viewModel;

  private ActivityResultLauncher<Intent> enableBluetoothLauncher;

  private ActivityResultLauncher<String[]> requestPermissionLauncher;

  public DeviceConnectionFragment() {
  }

  @Override
  protected FragmentDeviceConnectionBinding createBinding(LayoutInflater inflater, @Nullable ViewGroup container) {
    return FragmentDeviceConnectionBinding.inflate(inflater, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    debug("onViewCreated");
    viewModel = new ViewModelProvider(requireActivity()).get(BluetoothViewModel.class);

    binding.swipeRefresh.setOnRefreshListener(viewModel::startScan);

    binding.switchBluetooth.setOnCheckedChangeListener(this);

    binding.bluetoothList.setNestedScrollingEnabled(true);
    binding.bluetoothList.setLayoutManager(new LinearLayoutManager(requireContext()));

    bluetoothAdapter = new BluetoothDeviceListAdapter(this);
    binding.bluetoothList.setAdapter(bluetoothAdapter);

    viewModel.devices.observe(getViewLifecycleOwner(), bluetoothAdapter::submitList);

    viewModel.connected.observe(getViewLifecycleOwner(), connected -> {
      Snackbar.make(view, connected ? "连接成功" : "连接断开", Snackbar.LENGTH_SHORT).show();
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

  //

  @Override
  public void onCreate(@Nullable Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    enableBluetoothLauncher = registerForActivityResult(new ActivityResultContracts.StartActivityForResult(), this);
    requestPermissionLauncher = registerForActivityResult(new ActivityResultContracts.RequestMultiplePermissions(), result -> {
      boolean allGranted = result.values().stream().allMatch(isGranted -> isGranted);
      if (allGranted) {
        debug("All Bluetooth permissions granted.");
      }
      else {
        debug("Some Bluetooth permissions were denied.");
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
  public void onResume() {
    super.onResume();
    // It's a good practice to check for permissions when the fragment becomes active.
    checkAndRequestBluetoothPermissions();
  }

  @Override
  public void onActivityResult(ActivityResult result) {
    if (result.getResultCode() != AppCompatActivity.RESULT_OK) {
      showDialog(requireContext(), "错误", "打开蓝牙失败");
      binding.switchBluetooth.setChecked(false);
    }
  }

}
