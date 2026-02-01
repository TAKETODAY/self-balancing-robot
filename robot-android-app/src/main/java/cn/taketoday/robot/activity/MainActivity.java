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
import android.view.View;
import android.view.ViewGroup;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.navigation.NavController;
import androidx.navigation.fragment.NavHostFragment;
import androidx.navigation.ui.AppBarConfiguration;
import androidx.navigation.ui.NavigationUI;

import org.jspecify.annotations.Nullable;

import cn.taketoday.robot.LoggingSupport;
import cn.taketoday.robot.R;
import cn.taketoday.robot.databinding.ActivityMainBinding;
import cn.taketoday.robot.fragment.DeviceConnectionFragment;
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

    binding.fab.setOnClickListener(view -> {
      showDeviceConnectionInDialog();
    });

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

  private AlertDialog bluetoothDialog; // 将对话框作为成员变量，方便管理

  // ...

  // 创建一个方法来显示包含Fragment的对话框
  private void showDeviceConnectionInDialog() {
    // 如果对话框已显示，则不重复创建
    if (bluetoothDialog != null && bluetoothDialog.isShowing()) {
      return;
    }

    // 1. 创建 Fragment 实例
    DeviceConnectionFragment fragment = new DeviceConnectionFragment();

    // 2. 使用 FragmentManager 将 Fragment 添加到 Activity 的管理中
    // 注意：我们没有提供容器ID (第二个参数)，所以Fragment的视图不会被自动添加到任何地方
    // 它只是被“附加”到了Activity的生命周期上
    getSupportFragmentManager().beginTransaction()
            .add(fragment, "DeviceConnectionFragment") // 使用Tag来唯一标识Fragment
            .commit();

    // 我们需要立即执行事务，以确保Fragment的视图能够被创建并获取
    getSupportFragmentManager().executePendingTransactions();

    // 3. 获取 Fragment 的视图
    View fragmentView = fragment.getView();

    // 重要：如果视图的父布局不为空，需要先将其移除，否则会抛出异常
    if (fragmentView != null && fragmentView.getParent() != null) {
      ((ViewGroup) fragmentView.getParent()).removeView(fragmentView);
    }

    // 4. 创建 AlertDialog 并将 Fragment 的视图设置进去
    AlertDialog.Builder builder = new AlertDialog.Builder(this);

    // 如果你的布局已经很完善，可以不设置标题
    // builder.setTitle("连接蓝牙设备");

    // 将Fragment的视图设置为对话框的内容
    builder.setView(fragmentView);

    // 5. 设置对话框关闭时的监听器，这是关键的清理步骤
    builder.setOnDismissListener(dialog -> {
      // 当对话框关闭时，从 FragmentManager 中移除 Fragment
      DeviceConnectionFragment fragmentToRemove = (DeviceConnectionFragment) getSupportFragmentManager()
              .findFragmentByTag("DeviceConnectionFragment");
      if (fragmentToRemove != null) {
        getSupportFragmentManager().beginTransaction()
                .remove(fragmentToRemove)
                .commit();
      }
    });

    // 6. 创建并显示对话框
    bluetoothDialog = builder.create();
    bluetoothDialog.show();
  }

}
