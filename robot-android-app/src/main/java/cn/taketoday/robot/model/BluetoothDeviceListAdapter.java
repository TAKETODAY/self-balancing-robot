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

package cn.taketoday.robot.model;

import android.bluetooth.BluetoothDevice;
import android.view.LayoutInflater;
import android.view.ViewGroup;

import androidx.recyclerview.widget.DiffUtil;
import androidx.recyclerview.widget.ListAdapter;
import androidx.recyclerview.widget.RecyclerView;

import org.jspecify.annotations.Nullable;

import java.util.List;

import cn.taketoday.robot.databinding.BluetoothItemBinding;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2025/12/22 22:01
 */
public class BluetoothDeviceListAdapter extends ListAdapter<BluetoothDevice, BluetoothDeviceListAdapter.BluetoothDeviceViewHolder> {

  @Nullable
  private final BluetoothDeviceClickListener listener;

  public BluetoothDeviceListAdapter(@Nullable BluetoothDeviceClickListener listener) {
    super(DIFF_CALLBACK);
    this.listener = listener;
  }

  @Override
  public BluetoothDeviceViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
    return new BluetoothDeviceViewHolder(BluetoothItemBinding.inflate(LayoutInflater.from(parent.getContext()), parent, false));
  }

  @Override
  public void onBindViewHolder(BluetoothDeviceViewHolder holder, int position) {
    holder.bindTo(getItem(position), listener);
  }

  @Override
  public void onCurrentListChanged(List<BluetoothDevice> previousList, List<BluetoothDevice> currentList) {

  }

  public void clear() {
    submitList(null);
  }

  public static class BluetoothDeviceViewHolder extends RecyclerView.ViewHolder {

    private final BluetoothItemBinding binding;

    public BluetoothDeviceViewHolder(BluetoothItemBinding binding) {
      super(binding.getRoot());
      this.binding = binding;
    }

    public void bindTo(BluetoothDevice device, @Nullable BluetoothDeviceClickListener listener) {
      binding.name.setText(device.getName());
      switch (device.getBondState()) {
        case BluetoothDevice.BOND_NONE:
          binding.status.setText(DeviceItem.STATUS_BOND_NONE);
          break;
        case BluetoothDevice.BOND_BONDING:
          binding.status.setText(DeviceItem.STATUS_BONDING);
          break;
        case BluetoothDevice.BOND_BONDED:
          binding.status.setText(DeviceItem.STATUS_BONDED);
          break;
        default:
          binding.status.setText(DeviceItem.STATUS_UNKNOWN);
          break;
      }

      binding.getRoot().setOnClickListener(v -> {
        if (listener != null) {
          listener.onBluetoothDeviceClickListener(device);
        }
      });

    }

  }

  public static final DiffUtil.ItemCallback<BluetoothDevice> DIFF_CALLBACK = new DiffUtil.ItemCallback<>() {

    @Override
    public boolean areItemsTheSame(BluetoothDevice oldItem, BluetoothDevice newItem) {
      return oldItem == newItem;
    }

    @Override
    public boolean areContentsTheSame(BluetoothDevice oldItem, BluetoothDevice newUser) {
      return oldItem.equals(newUser);
    }
  };

}
