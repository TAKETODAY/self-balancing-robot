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

package cn.taketoday.robot.activity;

import android.annotation.SuppressLint;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;

import org.jspecify.annotations.Nullable;

import cn.taketoday.robot.R;
import cn.taketoday.robot.widget.JoystickView;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2025/12/18 17:49
 */
public final class ControlActivity extends BasicActivity {

  private TextView mTextViewAngleLeft;

  private TextView mTextViewStrengthLeft;

  private TextView mTextViewAngleRight;

  private TextView mTextViewStrengthRight;

  private TextView mTextViewCoordinateRight;

  @Override
  protected void postCreate(@Nullable Bundle savedInstanceState) {
//    setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
//    setTheme(R.style.FullscreenTheme);
    mTextViewAngleLeft = findViewById(R.id.textView_angle_left); mTextViewStrengthLeft = findViewById(R.id.textView_strength_left);

    JoystickView joystickLeft = findViewById(R.id.joystickView_left); joystickLeft.setOnMoveListener(new JoystickView.OnMoveListener() {
      @Override
      public void onMove(int angle, int strength) {
        mTextViewAngleLeft.setText(angle + "°");
        mTextViewStrengthLeft.setText(strength + "%");
      }
    });

    mTextViewAngleRight = findViewById(R.id.textView_angle_right); mTextViewStrengthRight = findViewById(R.id.textView_strength_right);
    mTextViewCoordinateRight = findViewById(R.id.textView_coordinate_right);

    final JoystickView joystickRight = findViewById(R.id.joystickView_right); joystickRight.setOnMoveListener(new JoystickView.OnMoveListener() {

      @SuppressLint("DefaultLocale")
      @Override
      public void onMove(int angle, int strength) {
        mTextViewAngleRight.setText(angle + "°"); mTextViewStrengthRight.setText(strength + "%");
        mTextViewCoordinateRight.setText(String.format("x%03d:y%03d", joystickRight.getNormalizedX(), joystickRight.getNormalizedY()));
      }
    });
  }

  @Override
  protected int getContentViewId() {
    return R.layout.activity_control;
  }

}
