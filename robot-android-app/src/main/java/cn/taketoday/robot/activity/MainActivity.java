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

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.ViewModel;
import androidx.lifecycle.ViewModelProvider;
import androidx.navigation.NavController;
import androidx.navigation.fragment.NavHostFragment;
import androidx.navigation.ui.AppBarConfiguration;
import androidx.navigation.ui.NavigationUI;

import org.jspecify.annotations.Nullable;

import cn.taketoday.robot.LoggingSupport;
import cn.taketoday.robot.R;
import cn.taketoday.robot.bluetooth.BluetoothViewModel;
import cn.taketoday.robot.databinding.ActivityMainBinding;
import cn.taketoday.robot.model.DataHandler;
import cn.taketoday.robot.model.RobotViewModel;
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
    connectModel();

    PermissionUtils.requestPermissions(this);
    binding = ActivityMainBinding.inflate(getLayoutInflater());
    setContentView(binding.getRoot());
    setSupportActionBar(binding.toolbar);

    NavHostFragment navHostFragment = (NavHostFragment) getSupportFragmentManager().findFragmentById(R.id.mainFragmentContainer);
    assert navHostFragment != null;
    navController = navHostFragment.getNavController();
    appBarConfiguration = new AppBarConfiguration.Builder(R.id.nav_main)
            .setOpenableLayout(binding.drawerLayout)
            .build();

    NavigationUI.setupActionBarWithNavController(this, navController, appBarConfiguration);
    NavigationUI.setupWithNavController(binding.navView, navController);
  }

  private void connectModel() {
    RobotViewModel robotModel = RobotViewModel.getInstance(this);
    new ViewModelProvider(this, new BluetoothViewModelFactory(robotModel)).get(BluetoothViewModel.class);

    BluetoothViewModel bluetoothModel = BluetoothViewModel.getInstance(this);
    bluetoothModel.robotConnected.observe(this, robotModel.connected::setValue);
  }

  @Override
  public boolean onSupportNavigateUp() {
    return (navController != null && NavigationUI.navigateUp(navController, appBarConfiguration))
            || super.onSupportNavigateUp();
  }

  private class BluetoothViewModelFactory implements ViewModelProvider.Factory {

    private final DataHandler handler;

    BluetoothViewModelFactory(DataHandler handler) {
      this.handler = handler;
    }

    @Override
    @SuppressWarnings("unchecked")
    public <T extends ViewModel> T create(Class<T> modelClass) {
      return (T) new BluetoothViewModel(getApplication(), handler);
    }
  }

}
