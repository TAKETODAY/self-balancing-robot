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

import android.view.View;
import android.widget.Toast;

import com.google.android.material.snackbar.Snackbar;

import cn.taketoday.robot.util.ToastUtils;

/**
 * ToastSupport
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public interface ToastSupport extends ContextCapable {

  default void makeToast(final String msg, final int type) {
    ToastUtils.makeToast(getContext(), msg, type);
  }

  default void makeShortToast(final String msg) {
    makeToast(msg, Toast.LENGTH_SHORT);
  }

  default void makeLongToast(final String msg) {
    makeToast(msg, Toast.LENGTH_LONG);
  }

  default void makeToast(final int id, final int type) {
    ToastUtils.makeToast(getContext(), getContext().getString(id), type);
  }

  default void makeShortToast(final int id) {
    makeShortToast(getContext().getString(id));
  }

  default void makeLongToast(final int id) {
    makeLongToast(getContext().getString(id));
  }

  // Snackbar
  // --------------------------------------------

  default void makeSnackBar(View view, int id, int time) {
    makeSnackBar(view, getContext().getString(id), time);
  }

  default void makeSnackBar(View view, CharSequence message, int time) {
    Snackbar.make(view, message, time).show();
  }

  default void makeShortSnackBar(View view, CharSequence message) {
    Snackbar.make(view, message, Snackbar.LENGTH_SHORT).show();
  }

  default void makeLongSnackBar(View view, CharSequence message) {
    Snackbar.make(view, message, Snackbar.LENGTH_LONG).show();
  }

  default void makeShortSnackBar(View view, int id) {
    makeShortSnackBar(view, getContext().getString(id));
  }

  default void makeLongSnackBar(View view, int id) {
    makeLongSnackBar(view, getContext().getString(id));
  }

}
