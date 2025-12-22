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
package cn.taketoday.robot;

import android.app.Application;
import android.util.Log;

import cn.taketoday.robot.bluetooth.BluetoothController;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public class RobotApplication extends Application implements Constant, LoggingSupport {

  public static boolean isDebug() {
    return BuildConfig.DEBUG;
  }

  private static RobotApplication instance;

  public static RobotApplication getInstance() {
    return instance;
  }

  @Override
  public void onCreate() {
    super.onCreate();
    Log.i("RobotApplication", "RobotApplication startup");

//    Iconify.with(new FontAwesomeModule())
//            .with(new EntypoModule())
//            .with(new TypiconsModule())//
//            .with(new MaterialModule())//
//            .with(new MaterialCommunityModule())//
//            .with(new MeteoconsModule())//
//            .with(new WeathericonsModule())//
//            .with(new SimpleLineIconsModule())//
//            .with(new IoniconsModule());

    instance = this;

  }

  @Override
  public void onTerminate() {
    super.onTerminate();
    final BluetoothController bluetoothController = RobotController.getBluetoothController();
    if (bluetoothController != null) {
      bluetoothController.destroy();
    }
  }

}
