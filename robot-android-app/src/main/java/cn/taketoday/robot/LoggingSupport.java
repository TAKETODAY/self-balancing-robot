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
package cn.taketoday.robot;

import android.util.Log;

/**
 * ApplicationSupport
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 */
public interface LoggingSupport {

  default boolean isDebugEnabled() {
    return RobotApplication.isDebug();
  }

  default void logger(Object msg, Object... args) {
    if (msg == null) {
      logger("Message can't be null");
    }
    else if (args == null || args.length == 0) {
      logger(msg.toString());
    }
    else {
      logger(String.format(msg.toString(), args));
    }
  }

  default void logger(Object msg) {
    if (isDebugEnabled()) {
      if (msg == null) {
        Log.d(getTAG(), "Message can't be null");
      }
      else {
        Log.d(getTAG(), msg.toString());
      }
    }
  }

  default String getTAG() {
    return getClass().getName();
  }

}
