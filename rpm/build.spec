%define name uAuditAnalyzer2
%define version 2.1.1
%define build_timestamp %{lua: print(os.date("%Y%m%d"))}

Name:           %{name}
Version:        %{version}
Release:        %{build_timestamp}.git%{?dist}
Summary:        Unmanarc's Auditd Analyzer
License:        GPLv3
URL:            https://github.com/unmanarc/uAuditAnalyzer2
Source0:        https://github.com/unmanarc/uAuditAnalyzer2/archive/main.tar.gz#/%{name}-%{version}-%{build_timestamp}.tar.gz
Group:          Applications/Internet

%define cmake cmake

%if 0%{?rhel} == 6
%define cmake cmake3
%endif

%if 0%{?rhel} == 7
%define cmake cmake3
%endif

%if 0%{?rhel} == 8
%define debug_package %{nil}
%endif

%if 0%{?rhel} == 9
%define debug_package %{nil}
%endif

%if 0%{?fedora} >= 33
%define debug_package %{nil}
%endif

BuildRequires: %{cmake} systemd libMantids-devel libMantids-sqlite openssl-devel zlib-devel boost-devel gcc-c++ jsoncpp-devel sqlite-devel
Requires: libMantids libMantids-sqlite zlib openssl boost-regex boost-system jsoncpp sqlite

%description
This package contains uAuditAnalyzer2 is intended to be a highly-efficient application for processing auditd logs received via rsyslog. It is being actively used in many enterprise-level environments for detecting potential threats and providing instant alerts using messaging applications, bots or other means.

%prep
%autosetup -n %{name}-main

%build
%{cmake} -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=MinSizeRel
%{cmake} -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=MinSizeRel
make %{?_smp_mflags}

%clean
rm -rf $RPM_BUILD_ROOT

%install
rm -rf $RPM_BUILD_ROOT

%if 0%{?fedora} >= 33
ln -s . %{_host}
%endif

%if 0%{?rhel} >= 9
ln -s . %{_host}
%endif

%if 0%{?fedora} == 35
ln -s . redhat-linux-build
%endif

%if "%{_host}" == "powerpc64le-redhat-linux-gnu"
ln -s . ppc64le-redhat-linux-gnu
%endif

%if "%{_host}" == "s390x-ibm-linux-gnu"
ln -s . s390x-redhat-linux-gnu
%endif

%if "%{cmake}" == "cmake3"
%cmake3_install
%else
%cmake_install
%endif

mkdir -vp $RPM_BUILD_ROOT/var/www
cp -a uanlz_web/var/www/uauditweb $RPM_BUILD_ROOT/var/www

mkdir -vp $RPM_BUILD_ROOT/etc/uauditanalyzer

cp -a uanlz_web/etc/uauditanalyzer/uanlz_web $RPM_BUILD_ROOT/etc/uauditanalyzer/
cp -a uanlz_log2json/etc/uauditanalyzer/uanlz_log2json $RPM_BUILD_ROOT/etc/uauditanalyzer/
cp -a uanlz_alert/etc/uauditanalyzer/uanlz_alert $RPM_BUILD_ROOT/etc/uauditanalyzer/

chmod 600 $RPM_BUILD_ROOT/etc/uauditanalyzer/uanlz_web/snakeoil.key
chmod 600 $RPM_BUILD_ROOT/etc/uauditanalyzer/uanlz_alert/*.json
chmod 600 $RPM_BUILD_ROOT/etc/uauditanalyzer/*/*.ini

mkdir -vp $RPM_BUILD_ROOT/usr/lib/systemd/system

cp etc/systemd/system/uanlz_web.service $RPM_BUILD_ROOT/usr/lib/systemd/system/
cp etc/systemd/system/uanlz_log2json.service $RPM_BUILD_ROOT/usr/lib/systemd/system/
cp etc/systemd/system/uanlz_alert.service $RPM_BUILD_ROOT/usr/lib/systemd/system/

chmod 644 $RPM_BUILD_ROOT/usr/lib/systemd/system/*.service

%files
%doc
%{_bindir}/uanlz_alert
%{_bindir}/uanlz_log2json
%{_bindir}/uanlz_web
%config(noreplace) /etc/uauditanalyzer/*
/var/www/uauditweb/*
/usr/lib/systemd/system/uanlz_alert.service
/usr/lib/systemd/system/uanlz_web.service
/usr/lib/systemd/system/uanlz_log2json.service

%post
%systemd_post uanlz_alert.service
%systemd_post uanlz_web.service
%systemd_post uanlz_log2json.service

%preun
%systemd_preun uanlz_alert.service
%systemd_preun uanlz_web.service
%systemd_preun uanlz_log2json.service

%postun
%systemd_postun_with_restart uanlz_alert.service
%systemd_postun_with_restart uanlz_web.service
%systemd_postun_with_restart uanlz_log2json.service

%changelog
