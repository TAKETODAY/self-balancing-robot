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

import android.content.Context;
import android.content.DialogInterface;

import androidx.appcompat.app.AlertDialog;

import cn.taketoday.robot.util.RobotUtils;
import infra.util.StringUtils;

/**
 * DialogSupport
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public interface DialogSupport extends ContextCapable {

  default AlertDialog showErrorDialog(Throwable ex) {

    String message = ex.getMessage();
    if (StringUtils.isEmpty(message)) {
      message = getContext().getResources().getString(R.string.device_connect_error);
    }
    return showErrorDialog(message);
  }

  default AlertDialog showErrorDialog(String message) {

    final Context context = getContext();
    final String fatalError = context.getResources().getString(R.string.fatal_error);
    return RobotUtils.showDialog(context, fatalError, message);
  }

  default AlertDialog showChoiceDialog(final String title, final String[] items) {
    return showChoiceDialog(title, items, null, null);
  }

  default AlertDialog showChoiceDialog(final String title, //
          final String[] items, //
          final DialogInterface.OnClickListener listener, //
          final DialogInterface.OnClickListener clickListener) {
    return RobotUtils.showChoiceDialog(getContext(), title, items, listener, clickListener);
  }

  default void showProgressDialog(final String title, final RobotUtils.ProgressCallBack runnable) {
    showProgressDialog(title, getContext(), runnable);
  }

  default void showProgressDialog(final String title, //
          final Context context, //
          final RobotUtils.ProgressCallBack runnable) {
    RobotUtils.showProgressDialog(context, title, runnable);
  }

  default AlertDialog showDialog(final int titleId, final int resId) {
    return showDialog(getContext().getString(titleId), getContext().getString(resId), null);
  }

  default AlertDialog showDialog(final String title, int resId) {
    return showDialog(title, getContext().getString(resId), null);
  }

  default AlertDialog showDialog(final String title, final String message) {
    return showDialog(title, message, null);
  }

  /**
   * Show a dialog use input title and message
   *
   * @param title input title
   * @param message dialog message
   * @param clickListener click listener
   * @return Dialog instance
   */
  default AlertDialog showDialog(final String title,//
          final String message,//
          final DialogInterface.OnClickListener clickListener) {

    return showDialog(title, message, getContext(), clickListener);
  }

  /**
   * Show a dialog use input title and message
   *
   * @param title input title
   * @param message dialog message
   * @param clickListener click listener
   * @param context Context
   * @return Dialog instance
   */
  default AlertDialog showDialog(final String title,//
          final String message,//
          final Context context, //
          final DialogInterface.OnClickListener clickListener) {
    return RobotUtils.showDialog(context, title, message, clickListener);
  }

}
