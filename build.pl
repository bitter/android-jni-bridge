#!/usr/bin/env perl -w

use PrepareAndroidSDK;
use File::Path;
use strict;
use warnings;

my $api = "android-17";

my @classes = (
	'::android::Manifest_permission',
	'::android::app::Activity',
	'::android::app::AlertDialog_Builder',
	'::android::app::NotificationManager',
	'::android::app::Presentation',
	'::android::content::Context',
	'::android::hardware::display::DisplayManager',
	'::android::hardware::GeomagneticField',
	'::android::location::LocationManager',
	'::android::media::AudioManager',
	'::android::media::MediaRouter',
	'::android::net::ConnectivityManager',
	'::android::os::Build',
	'::android::os::Build_VERSION',
	'::android::os::Environment',
	'::android::os::PowerManager',
	'::android::os::Vibrator',
	'::android::provider::Settings_Secure',
	'::android::provider::Settings_System',
	'::android::telephony::TelephonyManager',
	'::android::view::Choreographer',
	'::android::view::Display',
	'::android::view::WindowManager',
	'::android::webkit::MimeTypeMap',
	'::java::lang::Character',
	'::java::lang::System',
	'::java::lang::SecurityException',
	'::java::lang::NoSuchMethodError',
	'::java::lang::ClassCastException',
	'::java::lang::UnsatisfiedLinkError',
	'::java::util::HashSet',
	'::java::util::Map_Entry',

	'::com::google::android::gms::ads::identifier::AdvertisingIdClient',
	'::com::google::android::gms::common::GooglePlayServicesAvailabilityException',
	'::com::google::android::gms::common::GooglePlayServicesNotAvailableException'
);

sub BuildAndroid
{
	my $class_names = join(' ', @classes);
	my $threads = 8;

    PrepareAndroidSDK::GetAndroidSDK("$api", "21", "r10c", "24");

    system("make clean") && die("Clean failed");
    system("make -j$threads PLATFORM=android ABI=mips        APINAME=\"$api\" APICLASSES=\"$class_names\"") && die("Failed to make android mips library");
    system("make -j$threads PLATFORM=android ABI=armeabi-v7a APINAME=\"$api\" APICLASSES=\"$class_names\"") && die("Failed to make android armv7 library");
    system("make -j$threads PLATFORM=android ABI=armeabi     APINAME=\"$api\" APICLASSES=\"$class_names\"") && die("Failed to make android armv5 library");
    system("make -j$threads PLATFORM=android ABI=x86         APINAME=\"$api\" APICLASSES=\"$class_names\"") && die("Failed to make android x86 library");
}

sub ZipIt
{
	system("mkdir -p build/temp/include") && die("Failed to create temp directory.");

	# write build info
	my $git_info = qx(git symbolic-ref -q HEAD && git rev-parse HEAD);
	open(BUILD_INFO_FILE, '>', "build/temp/build.txt") or die("Unable to write build information to build/temp/build.txt");
	print BUILD_INFO_FILE "$git_info";
	close(BUILD_INFO_FILE);

	# create zip
	system("cp build/$api/source/*.h build/temp/include") && die("Failed to copy headers.");
	system("cd build && jar cf temp/jnibridge.jar bitter") && die("Failed to create java class archive.");
	system("cd build/$api && zip ../builds.zip -r android/*/*.a") && die("Failed to package libraries into zip file.");
	system("cd build/temp && zip ../builds.zip -r build.txt jnibridge.jar include") && die("Failed to package zip file.");
	system("rm -r build/temp") && die("Unable to remove temp directory.");
}

BuildAndroid();
ZipIt();