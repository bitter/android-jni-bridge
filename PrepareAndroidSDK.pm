#!/usr/bin/perl

package PrepareAndroidSDK;

use strict;
use warnings;

use File::Basename;
use File::Spec;
use lib File::Spec->rel2abs(dirname(__FILE__)) . '/perl_lib';

use Getopt::Long;
use Carp qw(croak carp);
use File::Path qw(mkpath rmtree);
use File::Spec::Functions;
use File::Copy;
use File::Copy::Recursive;

require Exporter;
our @ISA       = qw(Exporter);
our @EXPORT_OK = qw(GetAndroidSDK);

our $SDK_ROOT_ENV = "ANDROID_SDK_ROOT";
our $NDK_ROOT_ENV = "ANDROID_NDK_ROOT";

# based on https://dl.google.com/android/repository/repository-12.xml
# and sdk tools on https://developer.android.com/studio/#downloads

our $BASE_URL_SDK = "http://dl.google.com/android/repository/";

our $sdks =
    {
    "android-7"  => "android-2.1_r03-linux.zip",
    "android-8"  => "android-2.2_r03-linux.zip",
    "android-9"  => "android-2.3.1_r02-linux.zip",
    "android-10" => "android-2.3.3_r02-linux.zip",
    "android-11" => "android-3.0_r02-linux.zip",
    "android-12" => "android-3.1_r03-linux.zip",
    "android-13" => "android-3.2_r01-linux.zip",
    "android-14" => "android-14_r03.zip",
    "android-15" => "android-15_r03.zip",
    "android-16" => "android-16_r02.zip",
    "android-17" => "android-17_r01.zip",
    "android-18" => "android-18_r02.zip",
    "android-19" => "android-19_r03.zip",
    "android-21" => "android-21_r02.zip",
    "android-23" => "android-23_r02.zip",
    "android-26" => "platform-26_r02.zip",
    "android-28" => "platform-28_r06.zip",
    "android-31" => "platform-31_r01.zip",
    };

our $sdk_tools =
    {
    "version" => "26.1.1",
    "windows" => "sdk-tools-windows-4333796.zip",
    "linux"   => "sdk-tools-linux-4333796.zip",
    "macosx"  => "sdk-tools-darwin-4333796.zip",
    };

our $platform_tools =
    {
    "version" => "31.0.3",
    "windows" => "platform-tools_r31.0.3-windows.zip",
    "linux"   => "platform-tools_r31.0.3-linux.zip",
    "macosx"  => "platform-tools_r31.0.3-darwin.zip",
    };

our $build_tools =
    {
    "version" => "29.0.2",
    "windows" => "build-tools_r29.0.2-windows.zip",
    "linux"   => "build-tools_r29.0.2-linux.zip",
    "macosx"  => "build-tools_r29.0.2-macosx.zip",
    };

our $gpss =
    {
    "froyo" =>
        {
        "version" => "12.0.0",
        "url"     => "google_play_services_3265130_r12.zip",
        "path"    => "google_play_services_froyo",
        },
    };

our $ndks =
    {
    "r13b" =>
        {
        "windows" => "android-ndk-r13b-windows-x86_64.zip",
        "macosx"  => "android-ndk-r13b-darwin-x86_64.zip",
        "linux"   => "android-ndk-r13b-linux-x86_64.zip",
        },
    "r16b" =>
        {
        "windows" => "android-ndk-r16b-windows-x86_64.zip",
        "macosx"  => "android-ndk-r16b-darwin-x86_64.zip",
        "linux"   => "android-ndk-r16b-linux-x86_64.zip",
        },
    "r17" =>
        {
        "windows" => "android-ndk-r17-windows-x86_64.zip",
        "macosx"  => "android-ndk-r17-darwin-x86_64.zip",
        "linux"   => "android-ndk-r17-linux-x86_64.zip",
        },
    "r19" =>
        {
        "windows" => "android-ndk-r19-windows-x86_64.zip",
        "macosx"  => "android-ndk-r19-darwin-x86_64.zip",
        "linux"   => "android-ndk-r19-linux-x86_64.zip",
        },
    "r21" =>
        {
        "windows" => "android-ndk-r21d-windows-x86_64.zip",
        "macosx"  => "android-ndk-r21d-darwin-x86_64.zip",
        "linux"   => "android-ndk-r21d-linux-x86_64.zip",
        },
    };

our $sourcePropVersions =
    {
    "13.1.3345770" => "r13b (64-bit)",
    "16.1.4479499" => "r16b (64-bit)",
    "17.0.4754217" => "r17 (64-bit)",
    "19.0.5232133" => "r19 (64-bit)",
    "21.3.6528147" => "r21 (64-bit)",
    };

our ($HOST_ENV, $TMP, $HOME, $WINZIP);

sub GetAndroidSDK
{
    if (lc $^O eq 'darwin')
    {
        $HOST_ENV = "macosx";
        $TMP      = $ENV{"TMPDIR"};
        $HOME     = $ENV{"HOME"};
    }
    elsif (lc $^O eq 'linux')
    {
        $HOST_ENV = "linux";
        $TMP      = "/tmp";
        $HOME     = $ENV{"HOME"};
    }
    elsif (lc $^O eq 'mswin32')
    {
        $HOST_ENV = "windows";
        $TMP      = $ENV{"TMP"};
        $HOME     = $ENV{"USERPROFILE"};
        if (-e "External/7z/win32/7za.exe")
        {
            $WINZIP = "External/7z/win32/7za.exe";
        }
    }
    elsif (lc $^O eq 'cygwin')
    {
        $HOST_ENV = "windows";
        $TMP      = $ENV{"TMP"};
        $HOME     = $ENV{"HOME"};
    }
    else
    {
        die "UNKNOWN " . $^O;
    }

    print "Environment:\n";
    print "\tHost      = $HOST_ENV\n";
    print "\tTemporary = $TMP\n";
    print "\tHome      = $HOME\n";
    print "\n";
    print "\t\$$SDK_ROOT_ENV = $ENV{$SDK_ROOT_ENV}\n" if ($ENV{$SDK_ROOT_ENV});
    print "\t\$$NDK_ROOT_ENV = $ENV{$NDK_ROOT_ENV}\n" if ($ENV{$NDK_ROOT_ENV});
    print "\n";

    my ($sdk, $tools, $gps, $ndk, $setenv) = @_;

#   Getopt::Long::GetOptions("sdk=s"=>\$sdk, "ndk=s"=>\$ndk) or die ("Illegal cmdline options");

    if ($sdk or $tools or $gps)
    {
        if ($sdk)
        {
            print "Installing SDK '$sdk':\n";
        }
        elsif ($tools)
        {
            print "Installing SDK Tools '$tools':\n";
        }
        elsif ($gps)
        {
            print "Installing Google Play Services '$gps':\n";
        }

        if (!$ENV{$SDK_ROOT_ENV})
        {
            $ENV{$SDK_ROOT_ENV} = catfile($HOME, "android-sdk_auto");
            print "\t\$$SDK_ROOT_ENV not set; using $ENV{$SDK_ROOT_ENV} instead\n";
        }

        if (not $tools and $sdk)
        {
            my @split = split('-', $sdk);
            $tools = $split[1];
        }
        if ($tools)
        {
            PrepareSDKTools($tools);
        }
        if ($sdk)
        {
            PrepareSDK($sdk);
        }
        if ($gps)
        {
            PrepareGPS($gps);
        }
        print "\n";
    }

    if ($ndk)
    {
        print "Installing NDK '$ndk':\n";
        if (!$ENV{$NDK_ROOT_ENV})
        {
            $ENV{$NDK_ROOT_ENV} = catfile($HOME, "android-ndk_auto-" . $ndk);
            print "\t\$$NDK_ROOT_ENV not set; using $ENV{$NDK_ROOT_ENV} instead\n";
        }
        PrepareNDK($ndk);
        print "\n";
    }

    my $export = "export";
    if (lc $^O eq 'mswin32')
    {
        $export = "set";
    }

    if ($setenv and ($ENV{$SDK_ROOT_ENV} or $ENV{$SDK_ROOT_ENV}))
    {
        print "Outputing updated environment:\n";
        print "\t'$setenv'\n";
        open(SETENV, '>' . $setenv);
        print SETENV "$export $SDK_ROOT_ENV=$ENV{$SDK_ROOT_ENV}\n" if ($ENV{$SDK_ROOT_ENV});
        print SETENV "$export $NDK_ROOT_ENV=$ENV{$NDK_ROOT_ENV}\n" if ($ENV{$NDK_ROOT_ENV});
        close(SETENV);
        print "\n";
    }

    print "Environment:\n" if ($ENV{$SDK_ROOT_ENV} or $ENV{$SDK_ROOT_ENV});
    print "\t\$$SDK_ROOT_ENV = $ENV{$SDK_ROOT_ENV}\n" if ($ENV{$SDK_ROOT_ENV});
    print "\t\$$NDK_ROOT_ENV = $ENV{$NDK_ROOT_ENV}\n" if ($ENV{$NDK_ROOT_ENV});
    print "\n";
}

sub PrepareSDKTools
{
    my $sdk_root               = $ENV{$SDK_ROOT_ENV};
    my $sdk_tool_path          = catfile($sdk_root, "tools");
    my $sdk_platform_tool_path = catfile($sdk_root, "platform-tools");
    my $sdk_build_tool_path    = catfile($sdk_root, "build-tools", $build_tools->{'version'});

    my $sdk_tool_version          = GetToolsRevisionMajor("$sdk_tool_path");
    my $sdk_platform_tool_version = GetToolsRevisionMajor("$sdk_platform_tool_path");
    my $sdk_build_tool_version    = GetToolsRevisionMajor("$sdk_build_tool_path");

    my $sdk_tool      = $sdk_tools->{$HOST_ENV};
    my $platform_tool = $platform_tools->{$HOST_ENV};
    my $build_tool    = $build_tools->{$HOST_ENV};
    die("Unknown host environment '$HOST_ENV'") if (!$sdk_tool or !$platform_tool or !$build_tool);

    if (ParseMajor($sdk_tools->{'version'}) != $sdk_tool_version)
    {
        print "\tInstalling Tools\n";
        DownloadAndUnpackArchive($BASE_URL_SDK . $sdk_tool, "$sdk_tool_path");
    }

    if (ParseMajor($platform_tools->{'version'}) != $sdk_platform_tool_version)
    {
        print "\tInstalling Platform Tools\n";
        DownloadAndUnpackArchive($BASE_URL_SDK . $platform_tool, "$sdk_platform_tool_path");
    }

    if (ParseMajor($build_tools->{'version'}) != $sdk_build_tool_version)
    {
        print "\tInstalling Build Tools\n";
        DownloadAndUnpackArchive($BASE_URL_SDK . $build_tool, "$sdk_build_tool_path");
    }
}

sub ParseMajor
{
    my ($version_str) = @_;
    my @version_numbers = split('\.', $version_str);
    return int($version_numbers[0]);
}

sub GetToolsRevisionMajor
{
    my ($tools_dir) = @_;
    if (open PROPS, "<", catfile("$tools_dir", "source.properties"))
    {
        my @content = <PROPS>;
        close PROPS;
        chomp(@content);
        foreach (@content)
        {
            if (index($_, "Pkg.Revision") != -1)
            {
                my @tokens = split('=', $_);
                return ParseMajor($tokens[1]);
            }
        }
    }
    return 0;
}

sub PrepareSDK
{
    my $sdk_root = $ENV{$SDK_ROOT_ENV};

    my ($sdk) = @_;

    if (IsPlatformInstalled($sdk))
    {
        print "\tPlatform '$sdk' is already installed\n";
        return;
    }

    my $platform = $sdks->{$sdk};
    die("Unknown platform API '$sdk'") if (!$platform);

    my $output = catfile($sdk_root, "platforms", $sdk);
    print "\tDownloading '$platform' to '$output'\n";
    DownloadAndUnpackArchive($BASE_URL_SDK . $platform, $output);
}

sub PrepareGPS
{
    my $sdk_root = $ENV{$SDK_ROOT_ENV};
    my ($gps) = @_;

    my $gps_version = $gpss->{$gps}->{'version'};
    my $gps_url     = $gpss->{$gps}->{'url'};
    my $gps_path    = catfile($sdk_root, 'extras', 'google', $gpss->{$gps}->{'path'});

    if (-e catfile($gps_path, 'libproject', 'google-play-services_lib', 'libs', 'google-play-services.jar'))
    {
        print("\tGoogle Play Services '$gps_version' is already installed.\n");
        return;
    }

    print("\tInstalling Google Play Services '$gps_version'...\n");
    DownloadAndUnpackArchive($BASE_URL_SDK . $gps_url, $gps_path);
}

sub IsPlatformInstalled
{
    my $sdk_root = $ENV{$SDK_ROOT_ENV};
    my ($sdk) = @_;
    if (!$sdk_root)
    {
        return 0;
    }
    unless (grep { $_ eq $sdk } GetCurrentSDKPlatforms($sdk_root))
    {
        return 0;
    }
    return 1;
}

sub GetCurrentSDKPlatforms
{
    my ($sdk_root) = @_;
    my $platform_root = $sdk_root . "/platforms";
    opendir(my $dh, $platform_root) || return;
    my @platforms = grep { !/^\.\.?$/ && -e catfile($platform_root, $_, "android.jar") } readdir($dh);
    closedir $dh;

    return @platforms;
}

sub DownloadAndUnpackArchive
{
    my ($url, $output) = @_;
    my ($base, $base_url, $suffix) = fileparse($url, qr/\.[^.]*/);
    my ($dest_name, $dest_path) = fileparse($output);

    my $temporary_download_path = catfile($TMP, $base . $suffix);
    my $temporary_unpack_path   = catfile($TMP, $base . "_unpack");

    print "\t\tURL: " . $url . "\n";
    print "\t\tOutput: " . $output . "\n";
    print "\t\tBase: " . $base . "\n";
    print "\t\tURL base: " . $base_url . "\n";
    print "\t\tSuffix: " . $suffix . "\n";
    print "\t\tTmp DL: " . $temporary_download_path . "\n";
    print "\t\tTmp unpack: " . $temporary_unpack_path . "\n";
    print "\t\tDest path: " . $dest_path . "\n";
    print "\t\tDest name: " . $dest_name . "\n";

    # remove old output
    rmtree($output);
    mkpath($dest_path);

    # create temporary locations
    unlink($temporary_download_path);
    rmtree($temporary_unpack_path);
    mkpath($temporary_unpack_path);

    system("lwp-download", $url, $temporary_download_path);

    if ($WINZIP)
    {
        system($WINZIP, "x", $temporary_download_path, "-o" . $temporary_unpack_path);
    }
    else
    {
        if (lc $suffix eq '.zip')
        {
            system("unzip", "-q", $temporary_download_path, "-d", $temporary_unpack_path);
        }
        elsif (lc $suffix eq '.bz2')
        {
            system("tar", "-xf", $temporary_download_path, "-C", $temporary_unpack_path);
        }
        elsif (lc $suffix eq '.bin')
        { chmod(0755, $temporary_download_path);
            system($temporary_download_path, "-o" . $temporary_unpack_path);
        }
        elsif (lc $suffix eq '.exe')
        { chmod(0755, $temporary_download_path);
            system($temporary_download_path, "-o" . $temporary_unpack_path);
        }
        else
        {
            die "Unknown file extension '" . $suffix . "'\n";
        }
    }

    opendir(my $dh, $temporary_unpack_path);
    my @dirs = grep { !/^\.\.?$/ && -d catfile($temporary_unpack_path, $_) } readdir($dh);
    closedir $dh;
    my $unpacked_subdir = catfile($temporary_unpack_path, $dirs[0]);

    if (move($unpacked_subdir, $output) == 0)
    {
        # move failed. Try to do a recursive copy instead
        if (File::Copy::Recursive::dircopy($unpacked_subdir, $output) == 0)
        {
            print "\t\tMove/Copy Error: " . $! . "\n";
        }
    }

    # clean up
    unlink($temporary_download_path);
    rmtree($temporary_unpack_path);
}

sub ReadConfig
{
    my ($fileName) = @_;
    my %prefs;
    open CONFIG, "<", $fileName;
    while (<CONFIG>) {
        chomp;       # no newline
        s/#.*//;     # no comments
        s/^\s+//;    # no leading white
        s/\s+$//;    # no trailing white
        next unless length;    # anything left?
        my ($var, $value) = split(/\s*=\s*/, $_, 2);
        $prefs{$var} = $value;
    }
    return %prefs;
}

sub PrepareNDK
{
    my ($ndk) = @_;
    my $ndk_root = $ENV{$NDK_ROOT_ENV};
    $ndk_root = $1 if ($ndk_root =~ /(.*)\/$/);

    if (-e $ndk_root)
    {
        my $propFile   = catfile("$ndk_root", "source.properties");
        my $releaseTxt = catfile("$ndk_root", "RELEASE.TXT");

        my $full_version = "Unrecognized";

        if (-e $releaseTxt and open RELEASE, "<", $releaseTxt)
        {
            my @content = <RELEASE>;
            close RELEASE;
            chomp(@content);
            $full_version = $content[0];
        }
        elsif (-e $propFile)
        {
            my %prefs = ReadConfig($propFile);
            if (exists $prefs{'Pkg.Revision'})
            {
                my $revision = $prefs{'Pkg.Revision'};
                if (exists $sourcePropVersions->{$revision})
                {
                    $full_version = $sourcePropVersions->{$revision};
                }
            }
        }
        else
        {
            printf "Your current NDK is not recognized!";
        }

        print "\tCurrently installed = " . $full_version . "\n";

        # remove the possible '-rcX' or ' (64-bit)' from the end
        my @curr_arr = split(/ |-/, $full_version);
        my $major_version = $curr_arr[0];
        print "\tShort name = " . $major_version . "\n";
        my $isSdk64bit = index($full_version, "(64-bit)") != -1;

        if ($ndk eq $major_version and $isSdk64bit)
        {
            print "\tNDK '$ndk' is already installed\n";
            return;
        }
        else
        {
            if (!$isSdk64bit)
            {
                print "\tThe NDK version found is not 64-bit!\n";
                print "\t64-bit NDK is required for building Android player on all supported platforms\n";
            }
            my ($current_name, $path) = fileparse($ndk_root);

            $ENV{$NDK_ROOT_ENV} = catfile($path, "android-ndk-" . $ndk);
            if ($ndk_root eq $ENV{$NDK_ROOT_ENV})
            {
                print "**Error: NDK Preparation failed!";
                return;
            }
            print "\t\$$NDK_ROOT_ENV is pointing to a mismatching NDK; using $ENV{$NDK_ROOT_ENV} instead\n";
            PrepareNDK($ndk);
            return;
        }
    }

    rmtree($ndk_root);

    my $archive = $ndks->{$ndk}->{$HOST_ENV};
    die("Unknown NDK release '$ndk' (for $HOST_ENV)") if (!$archive);

    print "\tDownloading '$ndk' to '$ndk_root'\n";
    DownloadAndUnpackArchive($BASE_URL_SDK . $archive, $ndk_root);
}

1;
