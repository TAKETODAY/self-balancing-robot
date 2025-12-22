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

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2025/12/20 14:54
 */
public class RobotViewModel extends ViewModel {

  public final MutableLiveData<String> connectionStatus = new MutableLiveData<>("未连接");

  public final MutableLiveData<Integer> batteryLevel = new MutableLiveData<>(100);

  public final MutableLiveData<String> latestCommand = new MutableLiveData<>("");

  public LiveData<String> getConnectionStatus() {
    return connectionStatus;
  }

  public LiveData<Integer> getBatteryLevel() {
    return batteryLevel;
  }

  public LiveData<String> getLatestCommand() {
    return latestCommand;
  }

  public void updateConnectionStatus(String status) {
    connectionStatus.postValue(status);
  }

  public void updateBatteryLevel(int level) {
    batteryLevel.postValue(level);
  }

  public void sendControlCommand(String command) {
    latestCommand.postValue(command);
  }

  public void sendEmergencyStop() {
  }

}