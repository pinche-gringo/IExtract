# RPM spec file for IExtract
#
# The versions (of IExtract and the required libYGP) are passed from outside
# (they're maintained in meson.build only):
#   rpmbuild -ba --define "iextract_version 0.9.41" --define "ygp_version 0.9.81" IExtract.spec
# with IExtract-<version>.tar.xz (from "meson dist") in the SOURCES directory.

%{!?iextract_version: %{error:Define iextract_version (e.g. rpmbuild --define "iextract_version 0.9.41")}}
%{!?ygp_version: %{error:Define ygp_version (e.g. rpmbuild --define "ygp_version 0.9.81")}}

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
BuildRequires:  libYGP-devel >= %{ygp_version}

%description
IExtract prints the meta-information (like title, author and comment) stored
in files. Supported are HTML-, PDF-, RTF-, AbiWord-, OpenOffice.org-,
MS-Office- (including OOXML) and EPUB-documents, JPEG-, PNG-, GIF-, TIFF-,
WebP- and SVG-images, MP3-, OGG- (Vorbis and Opus), FLAC- and MPEG-4-audio
files (like M4A) as well as QuickTime-movies.

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
