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
import cn.taketoday.robot.protocol.message.BatteryStatus;
import cn.taketoday.robot.protocol.message.PercentageValue;
import cn.taketoday.robot.protocol.message.ReportType;
import cn.taketoday.robot.protocol.message.StatusReport;

import static cn.taketoday.robot.util.RobotUtils.constrain;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2025/12/20 14:54
 */
public class RobotViewModel extends ViewModel implements DataHandler, LoggingSupport {

  public final MutableLiveData<Boolean> connected = new MutableLiveData<>(false);

  public final MutableLiveData<Integer> batteryPercentage = new MutableLiveData<>(100);

  public final MutableLiveData<Integer> robotHeightPercentage = new MutableLiveData<>(50);

  @SuppressWarnings("NullAway.Init")
  private WritableChannel writableChannel;

  public boolean isConnected() {
    return Boolean.TRUE.equals(connected.getValue());
  }

  @Override
  public void handleIncomingData(byte[] data) {
    RobotMessage message = RobotMessage.parse(data);
    debug("onDataReceived: %s", message);

    switch (message.type) {
      case STATUS_REPORT -> {
        StatusReport statusReport = message.read(StatusReport.factory);
        handleStatusReport(statusReport);
      }
      default -> {
        debug("unknown message type: %s", message.type);
      }
    }

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

  public void setRobotHeightPercentage(int percentage) {
    percentage = constrain(percentage, 0, 100);
    robotHeightPercentage.postValue(percentage);
    RobotMessage robotMessage = RobotMessage.forControlHeight(percentage);
    sendMessage(robotMessage.toByteArray());
  }

  public int getRobotHeightPercentage() {
    Integer value = robotHeightPercentage.getValue();
    return value != null ? value : 0;
  }

  public void emergencyStop() {
    debug("stop");
    sendMessage(RobotMessage.forEmergencyStop());
  }

  public void emergencyRecover() {
    debug("recover");
    sendMessage(RobotMessage.forEmergencyRecover());
  }

  private void handleStatusReport(StatusReport statusReport) {
    ReportType type = statusReport.getType();
    switch (type) {
      case battery -> {
        BatteryStatus batteryStatus = statusReport.read(BatteryStatus.class);
        batteryPercentage.postValue(batteryStatus.getPercentage());
      }
      case robot_height -> {
        PercentageValue percentage = PercentageValue.parse(statusReport.createBodyReadable());
        robotHeightPercentage.postValue(percentage.value);
      }
    }
  }

  public static RobotViewModel getInstance(ViewModelStoreOwner store) {
    return new ViewModelProvider(store).get(RobotViewModel.class);
  }

}
