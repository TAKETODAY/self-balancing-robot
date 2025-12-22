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
package cn.taketoday.robot.util;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public class IntentUtils {

  public static void goToVolumeSettings(Activity activity) {
    activity.startActivity(new Intent(android.provider.Settings.ACTION_SOUND_SETTINGS));
  }

  public static void startActivity(Context context, Class<?> activityClass) {
    context.startActivity(new Intent(context, activityClass));
  }

}