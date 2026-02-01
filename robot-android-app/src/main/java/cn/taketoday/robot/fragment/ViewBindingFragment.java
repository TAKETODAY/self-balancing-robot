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

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.fragment.app.Fragment;
import androidx.viewbinding.ViewBinding;

import org.jspecify.annotations.Nullable;

/**
 * An abstract base class for {@link Fragment}s that use <a href="https://developer.android.com/topic/libraries/view-binding">View Binding</a>.
 * <p>
 * This class simplifies the process of inflating a layout and accessing its views in a type-safe manner.
 * Subclasses must implement the {@link #createBinding(LayoutInflater, ViewGroup)} method to provide the specific
 * binding instance for their layout. The binding is automatically created in {@code onCreateView} and cleared in
 * {@code onDestroyView} to prevent memory leaks.
 * <p>
 * Example Usage:
 * <pre>{@code
 * public class ProfileFragment extends ViewBindingFragment<FragmentProfileBinding> {
 *
 *     @Override
 *     protected FragmentProfileBinding createBinding(@NonNull LayoutInflater inflater, @Nullable ViewGroup container) {
 *         // Inflate and return the binding object for the fragment's layout.
 *         return FragmentProfileBinding.inflate(inflater, container, false);
 *     }
 *
 *     @Override
 *     public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
 *         super.onViewCreated(view, savedInstanceState);
 *
 *         // Access views through the 'binding' property, which is never null in onViewCreated.
 *         binding.nameTextView.setText("John Doe");
 *         binding.profileImage.setImageResource(R.drawable.ic_profile);
 *     }
 * }
 * }</pre>
 *
 * @param <BindingType> The type of the ViewBinding class generated for the fragment's layout.
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @see ViewBinding
 * @see Fragment
 */
public abstract class ViewBindingFragment<BindingType extends ViewBinding> extends Fragment {

  protected BindingType binding;

  @Override
  public final View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
    binding = createBinding(inflater, container);
    return binding.getRoot();
  }

  protected abstract BindingType createBinding(LayoutInflater inflater, @Nullable ViewGroup container);

  @Override
  public void onDestroyView() {
    super.onDestroyView();
    binding = null;
  }

}
