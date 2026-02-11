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

package cn.taketoday.robot.model;

import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;
import androidx.lifecycle.ViewModelProvider;
import androidx.lifecycle.ViewModelStoreOwner;

import java.util.Arrays;

import cn.taketoday.robot.LoggingSupport;
import cn.taketoday.robot.protocol.RobotMessage;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2025/12/20 14:54
 */
public class RobotViewModel extends ViewModel implements DataHandler, LoggingSupport {

  public final MutableLiveData<Boolean> connected = new MutableLiveData<>(false);

  public final MutableLiveData<Integer> batteryLevel = new MutableLiveData<>(100);

  public final MutableLiveData<Integer> legHeightPercentage = new MutableLiveData<>(50);

  @SuppressWarnings("NullAway.Init")
  private WritableChannel writableChannel;

  public boolean isConnected() {
    return Boolean.TRUE.equals(connected.getValue());
  }

  @Override
  public void handleIncomingData(byte[] data) {
    debug("onDataReceived: %s", Arrays.toString(data));

  }

  @Override
  public void register(WritableChannel writableChannel) {
    this.writableChannel = writableChannel;
  }

  public void sendMessage(byte[] data) {
    writableChannel.write(data);
    debug("write: %s", Arrays.toString(data));
  }

  public void sendMessage(RobotMessage message) {
    sendMessage(message.toByteArray());
  }

  public void setHeightPercentage(int percentage) {
    RobotMessage robotMessage = RobotMessage.forControlLeg(percentage, percentage);
    sendMessage(robotMessage.toByteArray());
  }

  public void emergencyStop() {
    debug("stop");
    sendMessage(RobotMessage.forEmergencyStop());
  }

  public void emergencyRecover() {
    debug("recover");
    sendMessage(RobotMessage.forEmergencyRecover());
  }

  public static RobotViewModel getInstance(ViewModelStoreOwner store) {
    return new ViewModelProvider(store).get(RobotViewModel.class);
  }

}
