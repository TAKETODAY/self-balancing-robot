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

package cn.taketoday.robot.widget;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

public class AttitudeIndicatorView extends View {
  private Paint mPaint;
  private float mPitchAngle = 0.0f; // 俯仰角（前后倾斜）
  private float mRollAngle = 0.0f;  // 横滚角（左右倾斜）

  public AttitudeIndicatorView(Context context) {
    this(context, null);
  }

  public AttitudeIndicatorView(Context context, AttributeSet attrs) {
    super(context, attrs);
    init();
  }

  private void init() {
    mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    mPaint.setStyle(Paint.Style.STROKE);
    mPaint.setStrokeWidth(4);
    mPaint.setColor(Color.WHITE);
  }

  // 用于从 ViewModel 更新数据的方法
  public void setAttitude(float pitch, float roll) {
    this.mPitchAngle = pitch;
    this.mRollAngle = roll;
    invalidate(); // 请求重绘视图
  }

  @Override
  protected void onDraw(Canvas canvas) {
    super.onDraw(canvas);

    int centerX = getWidth() / 2;
    int centerY = getHeight() / 2;
    int radius = Math.min(centerX, centerY) - 20; // 留出边距

    // 1. 绘制背景（一个圆）
    mPaint.setStyle(Paint.Style.FILL);
    mPaint.setColor(Color.parseColor("#303030")); // 深灰色背景
    canvas.drawCircle(centerX, centerY, radius, mPaint);

    // 2. 绘制十字线（代表水平）
    mPaint.setStyle(Paint.Style.STROKE);
    mPaint.setColor(Color.WHITE);
    canvas.drawLine(centerX - radius, centerY, centerX + radius, centerY, mPaint); // 水平线
    canvas.drawLine(centerX, centerY - radius, centerX, centerY + radius, mPaint); // 垂直线

    // 3. 根据横滚角旋转画布，模拟机身倾斜
    canvas.save();
    canvas.rotate(-mRollAngle, centerX, centerY);

    // 4. 根据俯仰角，平移水平线的位置（简化模拟）
    float pitchOffset = (mPitchAngle / 90) * radius; // 一个简单的映射
    canvas.drawLine(centerX - radius, centerY + pitchOffset,
            centerX + radius, centerY + pitchOffset,
            mPaint);

    canvas.restore(); // 恢复画布旋转

    // 5. 绘制外圈
    mPaint.setStyle(Paint.Style.STROKE);
    mPaint.setColor(Color.LTGRAY);
    canvas.drawCircle(centerX, centerY, radius, mPaint);
  }
}