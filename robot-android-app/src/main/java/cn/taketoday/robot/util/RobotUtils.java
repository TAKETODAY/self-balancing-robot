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

package cn.taketoday.robot.util;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.util.TypedValue;

import androidx.appcompat.app.AlertDialog;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public abstract class RobotUtils {

  public static int constrain(int amt, int low, int high) {
    return amt < low ? low : Math.min(amt, high);
  }

  public static int dp2px(Context context, float dpValue) {
    return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dpValue, context.getResources().getDisplayMetrics());
  }

  public static int sp2px(Context context, float spValue) {
    return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_SP, spValue, context.getResources().getDisplayMetrics());
  }

  /**
   * @param title Title
   * @param runnable Callback
   */
  public static void showProgressDialog(final Context context, //
          final String title, final ProgressCallBack runnable) {

    final ProgressDialog dialog = new ProgressDialog(context);

    dialog.setCancelable(false);

    dialog.setProgress(0);
    dialog.setTitle(title);
    dialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
    dialog.setMax(100);
    dialog.show();

    new Thread(() -> runnable.call(dialog)).start();
  }

  public static AlertDialog showChoiceDialog(final Context context, final String title, final String[] items) {
    return showChoiceDialog(context, title, items, null, null);
  }

  public static AlertDialog showChoiceDialog(final Context context, //
          final String title, //
          final String[] items, //
          final DialogInterface.OnClickListener listener, //
          final DialogInterface.OnClickListener clickListener) {

    final AlertDialog.Builder singleChoiceDialog = new AlertDialog.Builder(context);
    singleChoiceDialog.setTitle(title);

    singleChoiceDialog.setSingleChoiceItems(items, 0, clickListener);
    singleChoiceDialog.setPositiveButton("确定", listener);

    return singleChoiceDialog.show();
  }

  public static AlertDialog showDialog(final Context context, final String title, final String message) {
    return showDialog(context, title, message, null);
  }

  public static AlertDialog showDialog(final Context context, //
          final String title,//
          final String message,//
          final DialogInterface.OnClickListener clickListener) {

    final AlertDialog.Builder normalDialog = new AlertDialog.Builder(context);

    normalDialog.setTitle(title)//
            .setMessage(message)//
            .setCancelable(false)//
            .setPositiveButton("确定 Enter", clickListener)//
            .setNegativeButton("关闭 Close", clickListener);
    return normalDialog.show();
  }

  /**
   * @param context
   * @param title
   * @param message
   * @return @ProgressDialog
   */
  public static ProgressDialog showWaitingDialog(final Context context, final String title, final String message) {

    final ProgressDialog waitingDialog = new ProgressDialog(context);

    waitingDialog.setTitle(title);
    waitingDialog.setMessage(message);

    waitingDialog.setIndeterminate(true);

    waitingDialog.setCancelable(true);

    waitingDialog.show();
    return waitingDialog;
  }

  public interface ProgressCallBack {

    void call(ProgressDialog d);
  }

}
