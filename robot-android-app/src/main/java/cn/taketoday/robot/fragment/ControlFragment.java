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

package cn.taketoday.robot.fragment;

import android.annotation.SuppressLint;
import android.content.res.Configuration;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SeekBar;

import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.navigation.fragment.NavHostFragment;

import org.jspecify.annotations.Nullable;

import cn.taketoday.robot.LoggingSupport;
import cn.taketoday.robot.R;
import cn.taketoday.robot.databinding.FragmentControlBinding;
import cn.taketoday.robot.model.RobotViewModel;

/**
 * A fragment for displaying and managing the robot's control interface.
 * <p>
 * This fragment provides the user interface for sending commands to control the robot's
 * movements and actions. It uses {@link FragmentControlBinding} for its view,
 * which is inflated in the {@link #createBinding(LayoutInflater, ViewGroup)} method.
 * All view-related setup and event listeners should be implemented in
 * {@link #onViewCreated(View, Bundle)}.
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @see ViewBindingFragment
 * @see FragmentControlBinding
 * @since 1.0 2026/2/1 17:49
 */
public class ControlFragment extends ViewBindingFragment<FragmentControlBinding> implements LoggingSupport {

  @Override
  protected FragmentControlBinding createBinding(LayoutInflater inflater, @Nullable ViewGroup container) {
    return FragmentControlBinding.inflate(inflater, container, false);
  }

  @SuppressLint({ "SetTextI18n", "DefaultLocale" })
  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    RobotViewModel robotModel = RobotViewModel.getInstance(requireActivity());

    int orientation = getResources().getConfiguration().orientation;
    setFullScreen(orientation == Configuration.ORIENTATION_LANDSCAPE);

    setConnectionStatus(robotModel.isConnected());
    robotModel.connected.observe(getViewLifecycleOwner(), this::setConnectionStatus);
    robotModel.robotHeightPercentage.observe(getViewLifecycleOwner(), binding.robotHeight::setProgress);

    NavHostFragment navHostFragment = (NavHostFragment) requireParentFragment();
    var navController = navHostFragment.getNavController();

    binding.connectionStatus.setOnClickListener(e -> {
      navController.navigate(R.id.nav_device_connection);
    });

    if (binding.joystick != null) {
      binding.joystick.setOnMoveListener((xPercentage, yPercentage, angle, strength) -> {
        binding.angleX.setText(String.valueOf(xPercentage));
        binding.angleY.setText(String.valueOf(yPercentage));
        robotModel.joystickMove(xPercentage, yPercentage);
      });
    }
    else if (binding.joystickLeft != null && binding.joystickRight != null) {
      binding.joystickLeft.setOnMoveListener((z, leftPercentage, angle, strength) -> {
        binding.angleX.setText(String.valueOf(leftPercentage));
        int rightPercentage = binding.joystickRight.getAxisYPercentage();
        robotModel.control(leftPercentage, rightPercentage);
      });

      binding.joystickRight.setOnMoveListener((z, rightPercentage, angle, strength) -> {
        binding.angleY.setText(String.valueOf(rightPercentage));
        int leftPercentage = binding.joystickLeft.getAxisYPercentage();
        robotModel.control(leftPercentage, rightPercentage);
      });
    }

    robotModel.batteryStatus.observe(getViewLifecycleOwner(), status -> {
      binding.battery.setText(String.format("%.2fV", status.getVoltage()));
    });

    binding.addHeightBtn.setOnClickListener(v -> {
      robotModel.setRobotHeightPercentage(robotModel.getRobotHeightPercentage() + 10);
    });

    binding.minusHeightBtn.setOnClickListener(v -> {
      robotModel.setRobotHeightPercentage(robotModel.getRobotHeightPercentage() - 10);
    });

    binding.emergencyStop.setOnCheckedChangeListener((buttonView, checked) -> {
      if (checked) {
        robotModel.emergencyStop();
      }
      else {
        robotModel.emergencyRecover();
      }
    });

    binding.robotHeight.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

      @Override
      public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        robotModel.setRobotHeightPercentage(progress);
      }

      @Override
      public void onStartTrackingTouch(SeekBar seekBar) {

      }

      @Override
      public void onStopTrackingTouch(SeekBar seekBar) {

      }
    });

  }

  private void setConnectionStatus(boolean connected) {
    binding.connectionStatus.setText(connected ? R.string.device_connected : R.string.device_not_connected);
    binding.connectionStatus.setTextColor(getResources().getColor(connected ? R.color.color_success : R.color.color_warning, requireActivity().getTheme()));
  }

  @Override
  public void onConfigurationChanged(Configuration newConfig) {
    super.onConfigurationChanged(newConfig);
    setFullScreen(newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE);

  }

  /**
   * 根据是否横屏设置全屏状态
   *
   * @param isLandscape true 表示横屏，进入全屏；false 表示竖屏，退出全屏
   */
  private void setFullScreen(boolean isLandscape) {
    AppCompatActivity activity = (AppCompatActivity) getActivity();
    if (activity == null) {
      return;
    }

    var insetsController = WindowCompat.getInsetsController(
            activity.getWindow(), activity.getWindow().getDecorView());

    ActionBar supportActionBar = activity.getSupportActionBar();
    if (isLandscape) {
      insetsController.hide(WindowInsetsCompat.Type.statusBars());
      insetsController.hide(WindowInsetsCompat.Type.systemBars());
      insetsController.hide(WindowInsetsCompat.Type.navigationBars());
      if (supportActionBar != null) {
        supportActionBar.hide();
      }
    }
    else {
      insetsController.show(WindowInsetsCompat.Type.statusBars());
      if (supportActionBar != null) {
        supportActionBar.show();
      }
    }
  }

}
