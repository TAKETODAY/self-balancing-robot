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

import org.jspecify.annotations.Nullable;

import cn.taketoday.robot.databinding.FragmentAboutBinding;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2025/12/21 18:33
 */
public class AboutFragment extends ViewBindingFragment<FragmentAboutBinding> {

  @Override
  protected FragmentAboutBinding createBinding(LayoutInflater inflater, @Nullable ViewGroup container) {
    return FragmentAboutBinding.inflate(inflater, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
//    Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://taketoday.cn"));
//    startActivity(browserIntent);
  }
}
