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

import android.os.Bundle;

import androidx.appcompat.widget.Toolbar;
import androidx.preference.PreferenceFragmentCompat;

import org.jspecify.annotations.Nullable;

import cn.taketoday.robot.R;
import cn.taketoday.robot.fragment.SettingsFragment;

/**
 * Settings UI
 */
public class SettingsActivity extends BasicActivity {

  @Override
  protected int getContentViewId() {
    return R.layout.activity_settings;
  }

  @Override
  protected Toolbar getToolbar() {
    return findViewById(R.id.settings_toolbar);
  }

  @Override
  protected void postCreate(@Nullable Bundle savedInstanceState) {
    getSupportFragmentManager().beginTransaction().replace(R.id.settings, new SettingsFragment()).commit();
  }

}