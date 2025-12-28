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
 * An interface that provides convenient default methods for logging.
 * <p>
 * Classes implementing this interface can easily log messages at different levels
 * (DEBUG, INFO, WARN, ERROR) without needing to instantiate a logger. The default
 * implementation uses {@code android.util.Log} for output.
 * <p>
 * The logging tag is automatically determined from the implementing class's name
 * via the {@link #getTAG()} method.
 *
 * <p><b>Example Usage:</b>
 * <pre>{@code
 * public class MyService implements LoggingSupport {
 *
 *     public void doSomething() {
 *         // The `debug`, `info`, etc. methods are available directly
 *         info("Starting the service operation for user: {}", "JohnDoe");
 *
 *         try {
 *             // ... some logic that might fail
 *         } catch (Exception e) {
 *             // Log an error with an implicit tag (e.g., "com.example.MyService")
 *             error("Operation failed: {}", e.getMessage());
 *         }
 *     }
 * }
 * }</pre>
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @see Log
 */
public interface LoggingSupport {

  default boolean isDebugEnabled() {
    return RobotApplication.isDebug();
  }

  default void debug(Object msg, Object... args) {
    if (msg == null) {
      debug("Message can't be null");
    }
    else if (args == null || args.length == 0) {
      debug(msg.toString());
    }
    else {
      debug(String.format(msg.toString(), args));
    }
  }

  default void debug(Object msg) {
    if (isDebugEnabled()) {
      if (msg == null) {
        Log.d(getTAG(), "Message can't be null");
      }
      else {
        Log.d(getTAG(), msg.toString());
      }
    }
  }

  default void info(Object msg, Object... args) {
    log(Log.INFO, msg, args);
  }

  default void warn(Object msg, Object... args) {
    log(Log.WARN, msg, args);
  }

  default void error(Object msg, Object... args) {
    log(Log.ERROR, msg, args);
  }

  default void log(int level, Object msg, Object... args) {
    if (Log.isLoggable(getTAG(), level)) {
      if (msg == null) {
        Log.e(getTAG(), "Message can't be null");
      }
      else if (args == null || args.length == 0) {
        debug(msg.toString());
      }
      else {
        debug(String.format(msg.toString(), args));
      }
    }
  }

  default String getTAG() {
    return getClass().getName();
  }

}
