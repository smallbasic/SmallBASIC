#
# For more details, see https://developer.android.com/build/shrink-code
#

-keep public class * { public *; }
-keepclasseswithmembernames class * { native <methods>; }
-printmapping build/outputs/mapping/release/mapping.txt
-keepattributes LineNumberTable,SourceFile
