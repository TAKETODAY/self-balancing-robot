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

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;

import androidx.appcompat.app.AppCompatActivity;
import androidx.navigation.NavController;
import androidx.navigation.fragment.NavHostFragment;
import androidx.navigation.ui.AppBarConfiguration;
import androidx.navigation.ui.NavigationUI;

import com.google.android.material.snackbar.Snackbar;

import org.jspecify.annotations.Nullable;

import cn.taketoday.robot.LoggingSupport;
import cn.taketoday.robot.R;
import cn.taketoday.robot.databinding.ActivityMainBinding;
import cn.taketoday.robot.util.PermissionUtils;

/**
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public class MainActivity extends AppCompatActivity implements LoggingSupport {

  private ActivityMainBinding binding;

  private AppBarConfiguration appBarConfiguration;

  private NavController navController;

  @Override
  protected void onCreate(@Nullable Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    PermissionUtils.requestPermissions(this);
    binding = ActivityMainBinding.inflate(getLayoutInflater());
    setContentView(binding.getRoot());
    setSupportActionBar(binding.toolbar);

    binding.fab.setOnClickListener(view -> Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
            .setAction("Action", null).setAnchorView(R.id.fab).show());

    NavHostFragment navHostFragment = (NavHostFragment) getSupportFragmentManager().findFragmentById(R.id.mainFragmentContainer);
    assert navHostFragment != null;
    navController = navHostFragment.getNavController();
    appBarConfiguration = new AppBarConfiguration.Builder(R.id.nav_device_connection)
            .setOpenableLayout(binding.drawerLayout)
            .build();

    NavigationUI.setupActionBarWithNavController(this, navController, appBarConfiguration);
    NavigationUI.setupWithNavController(binding.navView, navController);
  }

  @Override
  public boolean onCreateOptionsMenu(Menu menu) {
    boolean result = super.onCreateOptionsMenu(menu);
    getMenuInflater().inflate(R.menu.menu, menu);
    return result;
  }

  @Override
  public boolean onOptionsItemSelected(MenuItem item) {
    switch (item.getItemId()) {
      case R.id.menu_connect:
        navController.navigate(R.id.nav_device_connection);
        return true;
      case R.id.menu_settings, R.id.nav_settings:
        navController.navigate(R.id.nav_settings);
        return true;
      case R.id.action_about:
        Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://taketoday.cn"));
        startActivity(browserIntent);
        return true;
      default:
        return super.onOptionsItemSelected(item);
    }
  }

  @Override
  public boolean onSupportNavigateUp() {
    return (navController != null && NavigationUI.navigateUp(navController, appBarConfiguration))
            || super.onSupportNavigateUp();
  }

}
