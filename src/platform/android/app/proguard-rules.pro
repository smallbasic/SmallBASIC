#
# For more details, see https://developer.android.com/build/shrink-code
#

-keep public class net.sourceforge.smallbasic.** { public *; }
-keep public class ioio.** { *; }
-keepclasseswithmembernames class * { native <methods>; }
-printmapping build/outputs/mapping/release/mapping.txt
-keepattributes LineNumberTable,SourceFile
