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
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import cn.taketoday.robot.LoggingSupport;
import cn.taketoday.robot.R;

/**
 * RecyclerViewAdapter
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public class RecyclerViewAdapter extends RecyclerView.Adapter<RecyclerViewAdapter.MyViewHolder> implements LoggingSupport {

  private final List<DeviceItem> datas;

  private ItemClickListener mItemClickListener;

  public RecyclerViewAdapter() {
    datas = new ArrayList<>();
  }

  public void add(DeviceItem deviceItem) {
    datas.add(deviceItem);
    notifyDataSetChanged();
  }

  public void merge(DeviceItem deviceItem) {
    if (shouldAdd(datas, deviceItem)) {
      datas.add(deviceItem);
      notifyDataSetChanged();
    }
  }

  public boolean shouldAdd(List<DeviceItem> deviceItems, final DeviceItem deviceItem) {

    for (final DeviceItem device : deviceItems) {
      if (device.equals(deviceItem)) {
        device.setStatus(deviceItem.getStatus());
        notifyDataSetChanged();
        return false;
      }
    }
    return true;
  }

  public void add(int index, DeviceItem deviceItem) {

    datas.add(index, deviceItem);
    notifyDataSetChanged();
  }

  public void addAll(List<DeviceItem> bluetoothItems) {

    clear();

    Set<DeviceItem> deviceItemSet = new HashSet<>(bluetoothItems);
    deviceItemSet.addAll(datas);
    datas.addAll(deviceItemSet);

    notifyDataSetChanged();
  }

  public void clear() {
    datas.clear();
  }

  public List<DeviceItem> getData() {
    return datas;
  }

  public boolean isEmpty() {
    return datas.isEmpty();
  }

  public void remove(Object o) {
    datas.remove(o);
  }

  public int getItemResource() {
    return itemResource;
  }

  public void setItemResource(final int itemResource) {
    this.itemResource = itemResource;
  }

  private int itemResource;

  @Override
  public MyViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
    logger("============创建视图列表暂存==============");

    final View view = LayoutInflater.from(parent.getContext())//
            .inflate(getItemResource(), parent, false);

    return new MyViewHolder(view);
  }

  @Override
  public void onBindViewHolder(MyViewHolder holder, int position) {

    logger("刷新列表 %s", position);

    final DeviceItem deviceItem = datas.get(position);
    if (deviceItem != null) {
      applyName(holder, deviceItem);
      applyStatus(holder, deviceItem);
      applyStyle(holder, deviceItem);
      applyListener(holder, deviceItem);
    }
    else {
      logger("发生错误刷新列表时不能为空");
    }
  }

  protected void applyListener(final MyViewHolder holder, final DeviceItem deviceItem) {
    holder.layout.setOnClickListener(v -> {
      if (mItemClickListener != null) {
        mItemClickListener.onItemClickListener(deviceItem);
      }
    });
  }

  protected void applyName(final MyViewHolder holder, final DeviceItem bluetoothItem) {
    holder.deviceName.setText(bluetoothItem.getName());
  }

  protected void applyStyle(final MyViewHolder holder, final DeviceItem bluetoothItem) {
    holder.image.setImageResource(bluetoothItem.getIcon());
  }

  protected void applyStatus(final MyViewHolder holder, final DeviceItem deviceItem) {
    holder.deviceStatus.setText(deviceItem.getStatus());
  }

  @Override
  public int getItemCount() {
    return datas == null ? 0 : datas.size();
  }

  static class MyViewHolder extends RecyclerView.ViewHolder {

    private final View layout;

    private final ImageView image;

    private final TextView deviceName;

    private final TextView deviceStatus;

    public MyViewHolder(View itemView) {
      super(itemView);
      this.layout = itemView.findViewById(R.id.layout);
      this.image = itemView.findViewById(R.id.img_signal);
      this.deviceName = itemView.findViewById(R.id.txt_device_name);
      this.deviceStatus = itemView.findViewById(R.id.txt_device_status);
    }
  }

  public void setItemClickListener(ItemClickListener listener) {
    mItemClickListener = listener;
  }

  public interface ItemClickListener {
    void onItemClickListener(DeviceItem bluetoothItem);
  }
}
