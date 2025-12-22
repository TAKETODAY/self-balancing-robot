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

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.provider.Settings;
import android.util.Log;
import android.widget.Toast;

import androidx.core.app.ActivityCompat;

import java.util.ArrayList;
import java.util.List;

import cn.taketoday.robot.R;

/**
 * PermissionUtils
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public class PermissionUtils {

  private static final String TAG = PermissionUtils.class.getSimpleName();

  public static final int CODE_MULTI_PERMISSION = 100;

  private static final String[] requestPermissions = {
          Manifest.permission.INTERNET,
          Manifest.permission.BLUETOOTH,
          Manifest.permission.BLUETOOTH_CONNECT,
          Manifest.permission.BLUETOOTH_SCAN,
          Manifest.permission.BLUETOOTH_ADMIN,
          Manifest.permission.READ_EXTERNAL_STORAGE,
          Manifest.permission.MODIFY_AUDIO_SETTINGS,
          Manifest.permission.WRITE_EXTERNAL_STORAGE,
  };

  public interface PermissionGrant {

    void onPermissionGranted(int requestCode);
  }

  public static void requestPermissions(final Activity activity) {
    requestPermissions(activity, null);
  }

  /**
   * 一次申请多个权限
   */
  public static void requestPermissions(final Activity activity, PermissionGrant grant) {
    final List<String> permissionsList = getNoGrantedPermission(activity, false);
    final List<String> shouldRationalePermissionsList = getNoGrantedPermission(activity, true);
    if (permissionsList == null || shouldRationalePermissionsList == null) {
      return;
    }
    if (Log.isLoggable(TAG, Log.DEBUG)) {
      Log.d(TAG, "requestMultiPermissions permissionsList:" + permissionsList
              .size() + ",shouldRationalePermissionsList:" + shouldRationalePermissionsList.size());
    }
    if (!permissionsList.isEmpty()) {
      ActivityCompat.requestPermissions(activity, permissionsList.toArray(new String[permissionsList.size()]), CODE_MULTI_PERMISSION);
      Log.d(TAG, "showMessageOKCancel requestPermissions");
    }
    else if (!shouldRationalePermissionsList.isEmpty()) {
      showMessageOKCancel(activity, activity.getResources().getString(R.string.request_permissions),
              (dialog, which) -> {
                final String[] permissions = shouldRationalePermissionsList
                        .toArray(new String[shouldRationalePermissionsList.size()]);

                ActivityCompat.requestPermissions(activity, permissions, CODE_MULTI_PERMISSION);
                //                            Log.d(TAG, "showMessageOKCancel requestPermissions");
              });
    }
    else if (grant != null) {
      grant.onPermissionGranted(CODE_MULTI_PERMISSION);
    }
  }

  private static void showMessageOKCancel(final Activity context, String message, DialogInterface.OnClickListener okListener) {
    new AlertDialog.Builder(context)
            .setMessage(message)
            .setPositiveButton("OK", okListener)
            .setNegativeButton("Cancel", null)
            .create()
            .show();
  }

  private static void openSettingActivity(final Activity activity, String message) {
    showMessageOKCancel(activity, message, (dialog, which) -> {
      Intent intent = new Intent();
      intent.setAction(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
      Log.d(TAG, "getPackageName(): " + activity.getPackageName());
      Uri uri = Uri.fromParts("package", activity.getPackageName(), null);
      intent.setData(uri);
      activity.startActivity(intent);
    });
  }

  /**
   * @param isShouldRationale true: return no granted and shouldShowRequestPermissionRationale permissions, false:return no granted and !shouldShowRequestPermissionRationale
   */
  public static ArrayList<String> getNoGrantedPermission(Activity activity, boolean isShouldRationale) {

    ArrayList<String> permissions = new ArrayList<>();

    for (String requestPermission : requestPermissions) {
      int checkSelfPermission;
      try {
        checkSelfPermission = ActivityCompat.checkSelfPermission(activity, requestPermission);
      }
      catch (RuntimeException e) {
        Toast.makeText(activity, activity.getResources().getString(R.string.request_permissions), Toast.LENGTH_SHORT).show();
        Log.e(TAG, "RuntimeException:" + e.getMessage());
        return null;
      }

      if (checkSelfPermission != PackageManager.PERMISSION_GRANTED) {
        Log.i(TAG, "getNoGrantedPermission ActivityCompat.checkSelfPermission != PackageManager.PERMISSION_GRANTED:" + requestPermission);

        if (ActivityCompat.shouldShowRequestPermissionRationale(activity, requestPermission)) {
          Log.d(TAG, "shouldShowRequestPermissionRationale if");
          if (isShouldRationale) {
            permissions.add(requestPermission);
          }
        }
        else {
          if (!isShouldRationale) {
            permissions.add(requestPermission);
          }
          Log.d(TAG, "shouldShowRequestPermissionRationale else");
        }

      }
    }

    return permissions;
  }

}
