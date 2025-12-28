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

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;

import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import org.jspecify.annotations.Nullable;

import cn.taketoday.robot.ApplicationSupport;

/**
 * This is the base Activity
 *
 * <p>
 * It provide some useful tools
 * </p>
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public abstract class BasicActivity extends AppCompatActivity implements ApplicationSupport {

  public BasicActivity() {
    debug("Activity:[%s] Are Created", this);
  }

  @Override
  protected void onCreate(@Nullable final Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    debug("onCreate");

    setContentView(getContentViewId());

    // setting tool bar
    // --------------------

    final TextView title = getToolBarTextView(); // if exist tool bar

    if (title != null) {
      Log.d(getTAG() + "===Toolbar Title===", "Setting title");
      final Bundle inputData = getIntent().getExtras();
      if (inputData != null) {
        title.setText(inputData.getString("toolbar_title"));
      }
    }

    final Toolbar toolbar = getToolbar();
    if (toolbar != null) {
      Log.d(getTAG() + "===Toolbar===", "Setting ToolBar");
      setSupportActionBar(toolbar);
      // disable Action bar
      final ActionBar actionBar = getSupportActionBar();
      if (actionBar != null) {
        actionBar.setDisplayHomeAsUpEnabled(true); // display return button
        actionBar.setDisplayShowTitleEnabled(false);
      }
    }

    postCreate(savedInstanceState);

    //        setFullScreen();
  }

  protected void postCreate(@Nullable Bundle savedInstanceState) {
  }

  public void setFullScreen() {
    View localView = getWindow().getDecorView();
    if (localView != null) {
      localView.setSystemUiVisibility(5894);
    }
  }

  //---------------
  @Override
  public boolean onOptionsItemSelected(MenuItem item) {
    if (item.getItemId() == android.R.id.home) {
      final Class<?> parentActivity = getParentActivity();
      if (parentActivity != null) {

        final Intent upIntent = new Intent(this, parentActivity)//
                .putExtra("toolbar_title", getIntent().getStringExtra("pre_toolbar_title"));

        navigateUpTo(upIntent);
      }
      else {
        super.onBackPressed();
      }
      return true;
    }
    return super.onOptionsItemSelected(item);
  }

  protected Class<?> getParentActivity() {
    return null;
  }

  @Override
  protected void onSaveInstanceState(final Bundle outState) {
    super.onSaveInstanceState(outState);
  }

  @Override
  protected void onRestoreInstanceState(final Bundle savedInstanceState) {
    super.onRestoreInstanceState(savedInstanceState);
    debug("Activity On onRestoreInstanceState");
  }

  @Override
  protected void onStart() {
    super.onStart();
    debug("Activity On Start");
  }

  @Override
  protected void onRestart() {
    super.onRestart();
    debug("Activity onRestart");
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
    debug("Activity onDestroy");
  }

  @Override
  protected void onPostCreate(@Nullable final Bundle savedInstanceState) {
    super.onPostCreate(savedInstanceState);
    debug("Activity onPostCreate");
  }

  @Override
  protected void onPause() {
    super.onPause();
    debug("Activity onPause");
  }

  @Override
  protected void onResume() {
    super.onResume();
    debug("Activity onResume");
  }

  @Override
  protected void onStop() {
    super.onStop();
    debug("Activity onStop");
  }

  @Override
  protected void onPostResume() {
    super.onPostResume();
    debug("Activity onPostResume");
  }

  protected abstract int getContentViewId();

  protected Toolbar getToolbar() {
    return null;
  }

  protected TextView getToolBarTextView() {
    return null;
  }

  @Override
  public Context getContext() {
    return this;
  }

}
