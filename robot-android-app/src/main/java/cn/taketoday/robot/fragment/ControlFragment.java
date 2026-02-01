package cn.taketoday.robot.fragment;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import org.jspecify.annotations.Nullable;

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

  @Override
  protected FragmentControlBinding createBinding(LayoutInflater inflater, @Nullable ViewGroup container) {
    return FragmentControlBinding.inflate(inflater, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    binding.joystick.setOnMoveListener((angle, strength) -> {
      binding.textViewAngleLeft.setText(angle + "°");
      binding.textViewStrengthLeft.setText(strength + "%");
    });

  }

}
