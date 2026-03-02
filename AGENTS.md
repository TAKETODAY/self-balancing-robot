# AGENTS.md - 平衡小车机器人项目

## 项目概述

这是一个自平衡机器人项目，包含两个主要组件：
1. **Android 应用** (`robot-android-app/`) - 用于机器人控制的 Kotlin/Java Android 应用
2. **嵌入式固件** (`robot-embedded-firmware/`) - ESP-IDF C/C++ 固件

## 构建命令

### Gradle (Android 应用)

```bash
# 构建整个项目（所有模块）
./gradlew build

# 构建调试版 APK
./gradlew assembleDebug

# 构建发布版 APK
./gradlew assembleRelease

# 运行 lint 分析
./gradlew lint

# 运行单元测试
./gradlew test

# 运行单个测试类
./gradlew test --tests "cn.taketoday.robot.MyTestClass"

# 运行单个测试方法
./gradlew test --tests "cn.taketoday.robot.MyTestClass.myTestMethod"

# 构建指定模块
./gradlew :robot-android-app:build
./gradlew :robot-android-app:common:build

# 清理构建产物
./gradlew clean
```

### ESP-IDF (嵌入式固件)

```bash
# 配置并构建固件（在 robot-embedded-firmware 目录下执行）
idf.py build

# 烧录固件
idf.py flash

# 监控串口输出
idf.py monitor
```

## 代码风格指南

### 通用原则

- **目标 Java 17** - 设置源文件和目标兼容性为 17
- **使用 `@Nullable`** - 来自 `org.jspecify.annotations` 的可空类型注解
- **使用接口** - 用于协议定义（如 `Message`、`Readable`、`Writable`）
- **避免原始类型** - 始终正确使用泛型
- **保持类专注** - 单一职责原则

### 命名规范

| 元素 | 规范 | 示例 |
|------|------|------|
| 类名 | PascalCase | `BluetoothLeService`、`RobotApplication` |
| 方法名 | camelCase | `connect()`、`readCharacteristic()` |
| 常量 | UPPER_SNAKE_CASE | `STATE_CONNECTED`、`UUID_PROTOCOL_SERVICE` |
| 包名 | 小写、点分隔 | `cn.taketoday.robot.bluetooth` |
| 接口 | PascalCase，通常以后缀结尾 | `Message`、`Readable`、`Factory` |

### 导入顺序

按以下顺序组织导入（组之间不空行）：
1. Android 框架导入 (`android.*`)
2. Java 标准库 (`java.*`, `javax.*`)
3. 第三方库 (`org.*`, `com.*`)
4. 项目导入 (`cn.taketoday.*`, `infra.*`)

### 类结构

Java 文件按以下顺序排列：
1. 版权头（必需）
2. 包声明
3. 导入语句
4. 类 Javadoc
5. 类声明及实现的接口
6. 常量 (static final)
7. 字段
8. 构造方法
9. 公共方法
10. 包级私有/私有方法
11. 内部类

### 日志记录

- 使用 `LoggingSupport` 接口，提供 `debug()`、`info()`、`warn()`、`error()` 方法
- 优先使用延迟字符串格式化（使用 `debug("message %s", arg)` 而非 `debug("message " + arg)`）
- 使用 `BuildConfig.DEBUG` 检查调试专用代码

### 错误处理

- 协议错误使用继承自 `RuntimeException` 的自定义异常类
- 内部不变量使用 `infra.lang.Assert`
- 可选结果返回 `null` 或使用 `@Nullable`，而非抛出异常
- 重新抛出前使用适当的日志级别记录错误

### Android 特定模式

- 使用 `RobotApplication` 作为 Application 子类
- 实现 `LoggingSupport` 和 `Constant` 接口
- 使用 ViewBinding（在 build.gradle 中启用）
- 仅使用 AndroidX 库
- 新 UI 组件使用 Jetpack Compose
- 遵循 Android 生命周期 - 不要在 `onCreate()` 中执行重操作

### 文档要求

- 公共 API（类、方法）需要 Javadoc
- 类 Javadoc 包含 `@author` 标签：`@author <a href="https://github.com/TAKETODAY">海子 Yang</a>`
- 包含 `@since` 标签和日期：`@since 1.0 2025/8/16` (日期是当前日期)
- 在 `@throws` 标签中记录未检查异常

### 测试

- 测试类命名为 `*Test`，放在 `src/test/java/`
- 使用 JUnit 4 风格（AndroidJUnitRunner）
- 测试资源放在 `src/test/resources/`
- 测试应该是确定性的且快速的

## 项目结构

```
self-balancing-robot/
├── build.gradle                 # 根构建配置（仓库）
├── gradle.properties├── settings.gradle
            # Gradle 配置
├── .codacy.yaml                # Codacy lint 配置
├── robot-android-app/          # 主 Android 应用
│   ├── build.gradle
│   ├── src/main/java/.../
│   ├── src/main/res/.../
│   └── src/test/java/.../
├── robot-android-app/common/   # 共享库模块
│   ├── build.gradle
│   └── src/main/java/infra/...
└── robot-embedded-firmware/     # ESP-IDF 固件
    ├── CMakeLists.txt
    ├── src/
    └── include/
```

## 模块依赖

- `robot-android-app` 依赖 `robot-android-app:common`
- Common 模块 (`infra.*`) 提供工具类（集合、并发、日志）

## 关键依赖

- AndroidX (AppCompat, Lifecycle, Navigation, Compose)
- Netty (buffer/netty-buffer:4.2.7.Final)
- EventBus (greenrobot:3.3.1)
- JSpecify (jspecify:1.0.0)
- SLF4J (slf4j-android:1.7.36)

## 额外说明

- 项目部分代码使用中文注释（特别是 UI 相关代码）
- 使用 Codacy 进行代码质量跟踪（配置在 `.codacy.yaml`）
- Codacy 排除：`*.js`、`*.css`、`*.html`、`*.xml`、`*.json`、`*.properties`、`*.md`、`*.gradle`、测试文件、`buildSrc`、`foc`
- 构建使用并行模式和配置缓存
