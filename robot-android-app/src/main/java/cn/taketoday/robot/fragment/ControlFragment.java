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

package cn.taketoday.robot.fragment;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.lifecycle.ViewModelProvider;

import org.jspecify.annotations.Nullable;

import cn.taketoday.robot.bluetooth.BluetoothViewModel;
import cn.taketoday.robot.databinding.FragmentControlBinding;

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
public class ControlFragment extends ViewBindingFragment<FragmentControlBinding> {

  private BluetoothViewModel viewModel;

  @Override
  protected FragmentControlBinding createBinding(LayoutInflater inflater, @Nullable ViewGroup container) {
    return FragmentControlBinding.inflate(inflater, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    viewModel = new ViewModelProvider(this).get(BluetoothViewModel.class);

    binding.joystick.setOnMoveListener((angle, strength) -> {
      binding.textViewAngleLeft.setText(angle + "°");
      binding.textViewStrengthLeft.setText(strength + "%");
    });

  }

}
