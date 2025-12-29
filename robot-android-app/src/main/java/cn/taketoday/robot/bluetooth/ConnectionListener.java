package cn.taketoday.robot.bluetooth;

import android.bluetooth.BluetoothDevice;

/**
 * A listener for receiving notifications about the state of a Bluetooth connection.
 * Implement this interface to handle events such as connection, disconnection, data reception, and RSSI updates.
 *
 * @author <a href="https://github.com/TAKETODAY">海子 Yang</a>
 * @since 1.0 2025/12/28 15:19
 */
public interface ConnectionListener {

  void onConnected(BluetoothDevice device);

  void onDisconnected(BluetoothDevice device);

  default void onConnecting(BluetoothDevice device) {
  }

  default void onDisconnecting(BluetoothDevice device) {
  }

  default void onServicesDiscovered(BluetoothDevice device) {
  }

  void onDataReceived(BluetoothDevice device, byte[] data);

  void onRssiUpdated(BluetoothDevice device, int rssi);

}