# Add project specific ProGuard rules here.
# You can control the set of applied configuration files using the
# proguardFiles setting in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

# Uncomment this to preserve the line number information for
# debugging stack traces.
#-keepattributes SourceFile,LineNumberTable

# If you keep the line number information, uncomment this to
# hide the original source file name.
#-renamesourcefileattribute SourceFile


#dontwarn	dontwarn是一个和keep可以说是形影不离,尤其是处理引入的library时.
#keep	保留类和类中的成员，防止被混淆或移除
#keepnames	保留类和类中的成员，防止被混淆，成员没有被引用会被移除
#keepclassmembers	只保留类中的成员，防止被混淆或移除
#keepclassmembernames	只保留类中的成员，防止被混淆，成员没有引用会被移除
#keepclasseswithmembers	保留类和类中的成员，防止被混淆或移除，保留指明的成员
#keepclasseswithmembernames	保留类和类中的成员，防止被混淆，保留指明的成员，成员没有引用会被移除


-optimizationpasses 5                                                           # 指定代码的压缩级别
-dontusemixedcaseclassnames                                                     # 是否使用大小写混合
#-dontskipnonpubliclibraryclasses                                                # 是否混淆第三方jar

-dontpreverify                                                                  # 混淆时是否做预校验
-verbose                                                                        # 混淆时是否记录日志
-optimizations !code/simplification/arithmetic,!field/*,!class/merging/*        # 混淆时所采用的算法

#-keep public class * extends android.app.Activity                               # 保持哪些类不被混淆
#-keep public class * extends android.app.Application                            # 保持哪些类不被混淆
#-keep public class * extends android.app.Service                                # 保持哪些类不被混淆
#-keep public class * extends android.content.BroadcastReceiver                  # 保持哪些类不被混淆
#-keep public class * extends android.content.ContentProvider                    # 保持哪些类不被混淆
#-keep public class * extends android.app.backup.BackupAgentHelper               # 保持哪些类不被混淆
#-keep public class * extends android.preference.Preference                      # 保持哪些类不被混淆
#-keep public class com.android.vending.licensing.ILicensingService              # 保持哪些类不被混淆

-keepclasseswithmembernames class * {                                           # 保持 native 方法不被混淆
    native <methods>;
}

#-keepclasseswithmembers class * {                                               # 保持自定义控件类不被混淆
#    public <init>(android.content.Context, android.util.AttributeSet);
#}
#
#-keepclasseswithmembers class * {
#    public <init>(android.content.Context, android.util.AttributeSet, int);     # 保持自定义控件类不被混淆
#}

#-keepclassmembers class * extends android.app.Activity {                        # 保持自定义控件类不被混淆
#   public void *(android.view.View);
#}

#-keepclassmembers enum * {                                                      # 保持枚举 enum 类不被混淆
#    public static **[] values();
#    public static ** valueOf(java.lang.String);
#}


#-keep class * implements android.os.Parcelable {                                # 保持 Parcelable 不被混淆
#  public static final android.os.Parcelable$Creator *;
#}
