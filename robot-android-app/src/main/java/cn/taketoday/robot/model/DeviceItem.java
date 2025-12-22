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

import androidx.annotation.Nullable;

import cn.taketoday.robot.R;

/**
 * DeviceItem
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public class DeviceItem {

  public static final String STATUS_UNKNOWN = "未知状态 Unknown Status";//已配对
  public static final String STATUS_BONDED = "已配对";//已配对
  public static final String STATUS_BONDING = "配对中";
  public static final String STATUS_UNCONNECT = "未连接";//
  public static final String STATUS_CONNECTED = "已连接";//
  public static final String STATUS_BOND_NONE = "未配对";
  public static final String STATUS_CONNECTING = "正在连接";//
  public static final String STATUS_DISCONNECTED = "已断开";//(但还保存)
  public static final String STATUS_DISCONNECTING = "正在断开";//

  //
  public static final String STATUS_DEVICE_IS_BUSY = "设备繁忙";//已配对
  public static final String STATUS_CONNECT_ERROR = "连接错误";//已配对

  private int icon = R.mipmap.icon_bluetooth;

  private String name = "未知";
  private String status = STATUS_UNKNOWN; // status string
  private Object resource;

  public DeviceItem() {
  }

  public DeviceItem(Object resource) {
    this.resource = resource;
  }

  public DeviceItem(String status, Object resource) {
    this.status = status;
    this.resource = resource;
  }

  public String getName() {
    return name;
  }

  public void setName(final String name) {
    this.name = name;
  }

  public String getStatus() {
    return status;
  }

  public void setStatus(final String status) {
    this.status = status;
  }

  public int getIcon() {
    return icon;
  }

  public void setIcon(final int icon) {
    this.icon = icon;
  }

  public final <T> T getNativeResource() {
    return (T) resource;
  }

  public final <T> T getNativeResource(Class<T> targetClass) {
    return targetClass.cast(resource);
  }

  @Override
  public int hashCode() {
    return getNativeResource().hashCode();
  }

  @Override
  public boolean equals(@Nullable final Object obj) {
    if (obj == this) {
      return true;
    }
    if (obj instanceof DeviceItem) {
      final Object nativeResource = getNativeResource();
      final Object otherNativeResource = ((DeviceItem) obj).getNativeResource();
      return nativeResource.equals(otherNativeResource);
    }
    return false;
  }

  @Override
  public String toString() {
    final StringBuilder sb = new StringBuilder("DeviceItem{");
    sb.append("icon=").append(icon);
    sb.append(", name='").append(name).append('\'');
    sb.append(", status='").append(status).append('\'');
    sb.append(", resource=").append(resource);
    sb.append('}');
    return sb.toString();
  }
}
