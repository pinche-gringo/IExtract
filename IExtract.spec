# RPM spec file for IExtract
#
# The version is passed from outside (it's maintained in meson.build only):
#   rpmbuild -ba --define "iextract_version 0.9.41" IExtract.spec
# with IExtract-<version>.tar.xz (from "meson dist") in the SOURCES directory.

%{!?iextract_version: %{error:Define iextract_version (e.g. rpmbuild --define "iextract_version 0.9.41")}}

Name:           IExtract
Version:        %{iextract_version}
Release:        1%{?dist}
Summary:        Extracts meta-information (title, author, comment) from files
License:        GPL-2.0-or-later
URL:            https://github.com/pinche-gringo/IExtract
Source0:        %{name}-%{version}.tar.xz

BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  meson
BuildRequires:  ninja-build
BuildRequires:  gettext
BuildRequires:  perl
BuildRequires:  python3
BuildRequires:  boost-devel
BuildRequires:  zlib-devel
BuildRequires:  libYGP-devel

%description
IExtract prints the meta-information (like title, author and comment) stored
in files. Supported are HTML-, PDF-, RTF-, AbiWord-, OpenOffice.org- and
MS-Office-documents (including OOXML), JPEG-, PNG- and GIF-images as well as
MP3- and OGG-audio files.

%package        devel
Summary:        Header file to implement plug-ins for IExtract
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    devel
Header file declaring the properties of a file, needed to implement plug-ins
handling further types of files for IExtract.

%prep
%autosetup

%build
%meson -Dnls=enabled
%meson_build

%install
%meson_install
%find_lang %{name}

%check
%meson_test

%files -f %{name}.lang
%license COPYING
%doc AUTHORS NEWS README doc/IExtract.html.*
%{_bindir}/IExtract

%files devel
%{_includedir}/IExtract/

%changelog
