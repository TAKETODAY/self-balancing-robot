package cn.taketoday.robot.bluetooth;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.os.Build;

import org.jspecify.annotations.Nullable;

import java.util.List;
import java.util.UUID;

import cn.taketoday.robot.LoggingSupport;
import cn.taketoday.robot.protocol.Frame;

public class BluetoothLeService implements LoggingSupport {

  // 服务UUID
  public static final UUID UUID_PROTOCOL_SERVICE = UUID.fromString("0000ABF0-0000-1000-8000-00805f9b34fb");

  public static final UUID UUID_PROTOCOL_FRAME = UUID.fromString("0000ABF1-0000-1000-8000-00805f9b34fb");

  // 客户端特征配置描述符UUID
  public static final UUID UUID_CLIENT_CHARACTERISTIC_CONFIG = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

  // 连接状态常量
  public static final int STATE_DISCONNECTED = 0;
  public static final int STATE_CONNECTING = 1;
  public static final int STATE_CONNECTED = 2;
  public static final int STATE_DISCONNECTING = 3;

  private final Context context;

  private final ConnectionListener connectionListener;

  private @Nullable BluetoothDevice device;

  private @Nullable BluetoothGatt bluetoothGatt;

  private int connectionState = STATE_DISCONNECTED;

  BluetoothLeService(Context context, ConnectionListener connectionListener) {
    this.context = context;
    this.connectionListener = connectionListener;
  }

  /**
   * 连接到GATT服务器
   */
  public boolean connect(BluetoothDevice device) {
    // 检查是否已经连接到该设备
    if (device.equals(this.device) && bluetoothGatt != null) {
      debug("Trying to use an existing BluetoothGatt for connection");
      if (bluetoothGatt.connect()) {
        connectionState = STATE_CONNECTING;
        return true;
      }
      else {
        return false;
      }
    }

    bluetoothGatt = device.connectGatt(context, false, gattCallback);
    debug("Trying to create a new connection");
    this.device = device;
    connectionState = STATE_CONNECTING;

    return true;
  }

  /**
   * 使用自动重连模式连接
   */
  public boolean connectWithAutoReconnect(BluetoothDevice device) {
    if (connectionState != STATE_CONNECTED) {
      bluetoothGatt = device.connectGatt(context, true, gattCallback);
      this.device = device;
      connectionState = STATE_CONNECTING;
      return true;
    }
    return false;
  }

  /**
   * 断开连接
   */
  public void disconnect() {
    if (bluetoothGatt == null) {
      debug("bluetoothGatt not initialized");
      return;
    }

    bluetoothGatt.disconnect();
    connectionState = STATE_DISCONNECTING;
  }

  /**
   * 关闭GATT客户端
   */
  public void close() {
    debug("close");
    if (bluetoothGatt == null) {
      return;
    }

    bluetoothGatt.close();
    bluetoothGatt = null;
    connectionState = STATE_DISCONNECTED;
  }

  /**
   * 读取指定特征的值
   */
  public void readCharacteristic(BluetoothGattCharacteristic characteristic) {
    if (bluetoothGatt == null) {
      debug("BluetoothAdapter not initialized");
      return;
    }

    bluetoothGatt.readCharacteristic(characteristic);
  }

  public void write(Frame frame) {
    write(frame.toBytes());
  }

  public void write(byte[] data) {
    if (bluetoothGatt == null || connectionState != STATE_CONNECTED) {
      debug("not connected");
      return;
    }

    BluetoothGattCharacteristic characteristic = getCharacteristic(UUID_PROTOCOL_SERVICE, UUID_PROTOCOL_FRAME);
    if (characteristic != null) {
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
        bluetoothGatt.writeCharacteristic(characteristic, data, BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
      }
      else {
        characteristic.setValue(data);
        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
        bluetoothGatt.writeCharacteristic(characteristic);
      }
    }
    else {
      error("write failed, characteristic not found");
    }
  }

  /**
   * 写入特征值（带响应）
   */
  public void writeCharacteristic(BluetoothGattCharacteristic characteristic) {
    if (bluetoothGatt == null) {
      debug("BluetoothAdapter not initialized");
      return;
    }

    // 设置写入类型
    characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
    bluetoothGatt.writeCharacteristic(characteristic);
  }

  /**
   * 写入特征值（无响应）
   */
  public void writeCharacteristicNoResponse(BluetoothGattCharacteristic characteristic) {
    if (bluetoothGatt == null) {
      debug("BluetoothAdapter not initialized");
      return;
    }

    characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
    bluetoothGatt.writeCharacteristic(characteristic);
  }

  public void setCharacteristicNotification(boolean enabled) {
    BluetoothGattCharacteristic characteristic = getCharacteristic(UUID_PROTOCOL_SERVICE, UUID_PROTOCOL_FRAME);
    if (characteristic != null) {
      setCharacteristicNotification(characteristic, enabled);
    }
    else {
      error("characteristic not found");
    }

  }

  /**
   * 设置特征值通知
   */
  public void setCharacteristicNotification(BluetoothGattCharacteristic characteristic, boolean enabled) {
    if (bluetoothGatt == null) {
      debug("BluetoothAdapter not initialized");
      return;
    }

    // 启用或禁用通知
    bluetoothGatt.setCharacteristicNotification(characteristic, enabled);

    // 对于需要客户端特征配置描述符的特征，需要写入描述符
    BluetoothGattDescriptor descriptor = characteristic.getDescriptor(UUID_CLIENT_CHARACTERISTIC_CONFIG);
    if (descriptor != null) {
      if (enabled) {
        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
      }
      else {
        descriptor.setValue(BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE);
      }
      bluetoothGatt.writeDescriptor(descriptor);
    }
  }

  public BluetoothGattCharacteristic getCharacteristic() {
    return getCharacteristic(UUID_PROTOCOL_SERVICE, UUID_PROTOCOL_FRAME);
  }

  public void setCharacteristicIndication(boolean enabled) {
    BluetoothGattCharacteristic characteristic = getCharacteristic(UUID_PROTOCOL_SERVICE, UUID_PROTOCOL_FRAME);
    if (characteristic != null) {
      setCharacteristicIndication(characteristic, enabled);
    }
    else {
      error("characteristic not found");
    }
  }

  /**
   * 设置特征值指示（需要设备响应）
   */
  public void setCharacteristicIndication(BluetoothGattCharacteristic characteristic, boolean enabled) {
    if (bluetoothGatt == null) {
      debug("bluetoothGatt not initialized");
      return;
    }

    bluetoothGatt.setCharacteristicNotification(characteristic, enabled);

    BluetoothGattDescriptor descriptor = characteristic.getDescriptor(UUID_CLIENT_CHARACTERISTIC_CONFIG);
    if (descriptor != null) {
      if (enabled) {
        descriptor.setValue(BluetoothGattDescriptor.ENABLE_INDICATION_VALUE);
      }
      else {
        descriptor.setValue(BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE);
      }
      bluetoothGatt.writeDescriptor(descriptor);
    }
    else {
      warn("descriptor not found");
    }
  }

  /**
   * 读取远程设备的RSSI
   */
  public void readRemoteRssi() {
    if (bluetoothGatt == null) {
      debug("BluetoothAdapter not initialized");
      return;
    }

    bluetoothGatt.readRemoteRssi();
  }

  /**
   * 请求更新连接参数
   */
  public void requestConnectionPriority(int connectionPriority) {
    if (bluetoothGatt == null) {
      debug("BluetoothAdapter not initialized");
      return;
    }

    bluetoothGatt.requestConnectionPriority(connectionPriority);
  }

  /**
   * 请求MTU（最大传输单元）大小
   */
  public void requestMtu(int mtu) {
    if (bluetoothGatt == null) {
      debug("BluetoothAdapter not initialized");
      return;
    }

    bluetoothGatt.requestMtu(mtu);
  }

  /**
   * 获取已发现的服务列表
   */
  public @Nullable List<BluetoothGattService> getSupportedGattServices() {
    if (bluetoothGatt == null) {
      return null;
    }
    return bluetoothGatt.getServices();
  }

  /**
   * 获取连接状态
   */
  public int getConnectionState() {
    return connectionState;
  }

  /**
   * 获取设备
   */
  public @Nullable BluetoothDevice getDevice() {
    return device;
  }

  /**
   * 根据UUID查找服务
   */
  public @Nullable BluetoothGattService getService(UUID serviceUuid) {
    if (bluetoothGatt == null) {
      return null;
    }
    return bluetoothGatt.getService(serviceUuid);
  }

  /**
   * 根据服务UUID和特征UUID查找特征
   */
  public @Nullable BluetoothGattCharacteristic getCharacteristic(UUID serviceUuid, UUID characteristicUuid) {
    BluetoothGattService service = getService(serviceUuid);
    if (service == null) {
      return null;
    }
    return service.getCharacteristic(characteristicUuid);
  }

  private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {

    @Override
    public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
      debug("onConnectionStateChange : %s => %s", gatt, newState);
      if (newState == BluetoothProfile.STATE_CONNECTED) {
        connectionState = STATE_CONNECTED;
        info("Connected to GATT server");
        info("Attempting to start service discovery: %s", gatt.discoverServices());

        connectionListener.onConnected(gatt.getDevice());
      }
      else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
        info("Disconnected from GATT server");
        close();
        connectionState = STATE_DISCONNECTED;
        connectionListener.onDisconnected(gatt.getDevice());
      }
      else if (newState == BluetoothProfile.STATE_CONNECTING) {
        connectionState = STATE_CONNECTING;
        connectionListener.onConnecting(gatt.getDevice());
      }
      else if (newState == BluetoothProfile.STATE_DISCONNECTING) {
        connectionState = STATE_DISCONNECTING;
        connectionListener.onDisconnecting(gatt.getDevice());
      }
      else {
        warn("Unknown GATT connection state: %s", newState);
      }
    }

    @Override
    public void onServicesDiscovered(BluetoothGatt gatt, int status) {
      if (status == BluetoothGatt.GATT_SUCCESS) {
        info("Services discovered");
        connectionListener.onServicesDiscovered(gatt, gatt.getDevice());
      }
      else {
        debug("onServicesDiscovered received: %s", status);
      }
    }

    @Override
    public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
      if (status == BluetoothGatt.GATT_SUCCESS) {
        dataRead(gatt, characteristic);
      }
    }

    @Override
    public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
      if (status == BluetoothGatt.GATT_SUCCESS) {
        debug("Characteristic write successful: %s", characteristic.getUuid().toString());
      }
    }

    @Override
    public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
      dataRead(gatt, characteristic);
    }

    @Override
    public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
      if (status == BluetoothGatt.GATT_SUCCESS) {
        debug("Descriptor read: %s", descriptor.getUuid().toString());
      }
    }

    @Override
    public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
      if (status == BluetoothGatt.GATT_SUCCESS) {
        debug("Descriptor write successful: %s", descriptor.getUuid().toString());
      }
      // 133
      else {
        error("Descriptor write failed: %s, status: %s", descriptor.getUuid().toString(), status);
      }
    }

    @Override
    public void onReadRemoteRssi(BluetoothGatt gatt, int rssi, int status) {
      if (status == BluetoothGatt.GATT_SUCCESS) {
        debug("Remote RSSI: %s dBm", rssi);
        connectionListener.onRssiUpdated(gatt, rssi);
      }
    }

    @Override
    public void onMtuChanged(BluetoothGatt gatt, int mtu, int status) {
      if (status == BluetoothGatt.GATT_SUCCESS) {
        debug("MTU changed to: " + mtu);
      }
    }

    @Override
    public void onPhyUpdate(BluetoothGatt gatt, int txPhy, int rxPhy, int status) {
      if (status == BluetoothGatt.GATT_SUCCESS) {
        debug("PHY updated - TX: " + txPhy + ", RX: " + rxPhy);
      }
    }

    @Override
    public void onPhyRead(BluetoothGatt gatt, int txPhy, int rxPhy, int status) {
      if (status == BluetoothGatt.GATT_SUCCESS) {
        debug("PHY read - TX: " + txPhy + ", RX: " + rxPhy);
      }
    }
  };

  /**
   * 发送带数据的广播更新
   */
  private void dataRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
    if (UUID_PROTOCOL_FRAME.equals(characteristic.getUuid())) {
      final byte[] data = characteristic.getValue();
      if (data != null && data.length > 0) {
        connectionListener.onDataReceived(gatt, data);
      }
    }
    else {
      warn("unsupported characteristic %s", characteristic);
    }
  }
}