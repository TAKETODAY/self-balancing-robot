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

import android.view.LayoutInflater;
import android.view.ViewGroup;

import androidx.recyclerview.widget.DiffUtil;
import androidx.recyclerview.widget.ListAdapter;
import androidx.recyclerview.widget.RecyclerView;

import org.jspecify.annotations.Nullable;

import java.util.List;

import cn.taketoday.robot.LoggingSupport;
import cn.taketoday.robot.R;
import cn.taketoday.robot.bluetooth.BluetoothItem;
import cn.taketoday.robot.databinding.BluetoothItemBinding;

/**
 * An adapter for displaying a list of Bluetooth devices in a {@link RecyclerView}.
 * <p>
 * This class uses {@link ListAdapter} to efficiently update the list of devices
 * by leveraging {@link DiffUtil}. It binds {@link BluetoothItem} data to the
 * corresponding views in the list and handles item click events.
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @see BluetoothItem
 * @see BluetoothDeviceViewHolder
 * @since 1.0 2025/12/22 22:01
 */
public class BluetoothDeviceListAdapter extends ListAdapter<BluetoothItem, BluetoothDeviceListAdapter.BluetoothDeviceViewHolder> implements LoggingSupport {

  @Nullable
  private final BluetoothItemClickListener listener;

  public BluetoothDeviceListAdapter(@Nullable BluetoothItemClickListener listener) {
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
  public void onCurrentListChanged(List<BluetoothItem> previousList, List<BluetoothItem> currentList) {
    debug("previousList: %s, currentList: %s", previousList, currentList);
  }

  public void clear() {
    submitList(null);
  }

  public static class BluetoothDeviceViewHolder extends RecyclerView.ViewHolder {

    private final BluetoothItemBinding binding;

    private final int connectedColor;

    private final int notConnectedColor;

    public BluetoothDeviceViewHolder(BluetoothItemBinding binding) {
      super(binding.getRoot());
      this.binding = binding;
      this.connectedColor = binding.layout.getContext().getColor(R.color.colorPrimary);
      this.notConnectedColor = binding.layout.getContext().getColor(android.R.color.darker_gray);
    }

    public void bindTo(BluetoothItem item, @Nullable BluetoothItemClickListener listener) {
      binding.name.setText(item.getName());
      binding.status.setText(item.getStatusText());
      binding.status.setTextColor(item.isConnected() ? connectedColor : notConnectedColor);
      binding.icon.setImageResource(R.mipmap.icon_bluetooth);

      binding.getRoot().setOnClickListener(v -> {
        if (listener != null) {
          listener.onBluetoothItemClickListener(v, item);
        }
      });

      binding.deviceAddress.setText(item.getAddress());
      binding.rssiValue.setText(String.format("RSSI: %s dBm", item.getRssi()));
    }

  }

  public static final DiffUtil.ItemCallback<BluetoothItem> DIFF_CALLBACK = new DiffUtil.ItemCallback<>() {

    @Override
    public boolean areItemsTheSame(BluetoothItem oldItem, BluetoothItem newItem) {
      return oldItem == newItem;
    }

    @Override
    public boolean areContentsTheSame(BluetoothItem oldItem, BluetoothItem newUser) {
      return oldItem.equals(newUser);
    }
  };

}
