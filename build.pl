#!/usr/bin/env perl -w
use PrepareAndroidSDK;
use File::Path;
use strict;
use warnings;

my $api = "android-26";

my @classes = (
	'::android::Manifest_permission',
	'::android::R_attr',
	'::android::app::Activity',
	'::android::app::AlertDialog_Builder',
	'::android::app::NotificationManager',
	'::android::app::Presentation',
	'::android::content::Context',
	'::android::graphics::Color',
	'::android::graphics::ImageFormat',
	'::android::graphics::drawable::ColorDrawable',
	'::android::hardware::display::DisplayManager',
	'::android::hardware::Camera',
	'::android::hardware::input::InputManager',
	'::android::hardware::GeomagneticField',
	'::android::location::LocationManager',
	'::android::media::AudioAttributes_Builder',
	'::android::media::AudioFocusRequest_Builder',
	'::android::media::AudioManager',
	'::android::media::MediaCodec',
	'::android::media::MediaCodec::BufferInfo',
	'::android::media::MediaExtractor',
	'::android::media::MediaFormat',
	'::android::media::MediaRouter',
	'::android::net::ConnectivityManager',
	'::android::os::Build',
	'::android::os::Build_VERSION',
	'::android::os::HandlerThread',
	'::android::os::Environment',
	'::android::os::PowerManager',
	'::android::os::Process',
	'::android::os::Vibrator',
	'::android::provider::Settings_Secure',
	'::android::provider::Settings_System',
	'::android::telephony::TelephonyManager',
	'::android::telephony::SubscriptionManager',
	'::android::telephony::SubscriptionInfo',
	'::android::view::Choreographer',
	'::android::view::Display',
	'::android::view::Gravity',
	'::android::view::SurfaceView',
	'::android::view::WindowManager',
	'::android::webkit::MimeTypeMap',
	'::android::widget::CheckBox',
	'::android::widget::CompoundButton_OnCheckedChangeListener',
	'::android::widget::ProgressBar',
	'::java::lang::Character',
	'::java::lang::System',
	'::java::lang::SecurityException',
	'::java::lang::NoSuchMethodError',
	'::java::lang::ClassCastException',
	'::java::lang::UnsatisfiedLinkError',
	'::java::io::FileNotFoundException',
	'::java::net::HttpURLConnection',
	'::java::nio::channels::Channels',
	'::java::util::HashSet',
	'::java::util::Map_Entry',
	'::java::util::NoSuchElementException',
	'::java::util::Scanner',
	'::javax::net::ssl::X509TrustManager',
	'::javax::net::ssl::TrustManagerFactory',
	'::java::security::KeyStore',

	'::com::google::android::gms::ads::identifier::AdvertisingIdClient',
	'::com::google::android::gms::common::GooglePlayServicesAvailabilityException',
	'::com::google::android::gms::common::GooglePlayServicesNotAvailableException',
);

sub BuildAndroid
{
	my $class_names = join(' ', @classes);
	my $threads = 8;

    PrepareAndroidSDK::GetAndroidSDK("$api", "21", "r16b", "24");

    system("make clean") && die("Clean failed");
    system("make api-source PLATFORM=android APINAME=\"$api\" APICLASSES=\"$class_names\"") && die("Failed to make API source");
    system("make api-module PLATFORM=android APINAME=\"$api\" APICLASSES=\"$class_names\"") && die("Failed to make API module");
    system("make compile-static-apilib -j$threads PLATFORM=android ABI=armeabi-v7a APINAME=\"$api\" APICLASSES=\"$class_names\"") && die("Failed to make android armv7 library");
    system("make compile-static-apilib -j$threads PLATFORM=android ABI=arm64-v8a   APINAME=\"$api\" APICLASSES=\"$class_names\"") && die("Failed to make android arm64 library");
    system("make compile-static-apilib -j$threads PLATFORM=android ABI=x86         APINAME=\"$api\" APICLASSES=\"$class_names\"") && die("Failed to make android x86 library");
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
