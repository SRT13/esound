# Note that this is NOT a relocatable package
%define ver      0.2.1
%define rel      SNAP
%define prefix   /usr

Summary:   The Enlightened Sound Daemon
Name:      esound
Version:   %ver
Release:   %rel
Copyright: GPL
Group:     Daemons
Source0:   esound-%{PACKAGE_VERSION}.tar.gz
URL:       http://pw1.netcom.com/~ericmit/EsounD.html
BuildRoot: /tmp/esound-%{PACKAGE_VERSION}-root
Packager: Michael Fulbright <msf@redhat.com>

Docdir: %{prefix}/doc

%description
The Enlightened Sound Daemon is a server process that allows multiple
applications to share a single sound card.

%package devel
Summary: Libraries, includes, etc to develop EsounD applications
Group: Libraries
Requires: esound

%description devel
Libraries, include files, etc you can use to develop EsounD applications.

 
%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" LDFLAGS="-s" ./configure --prefix=%{prefix} 

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
  make
else
  make
fi

%install
rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{prefix} install

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%clean
#rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc AUTHORS COPYING ChangeLog EsounD.html INSTALL NEWS README TIPS TODO
%{prefix}/bin/*
%{prefix}/lib/lib*.so.*

%files devel
%defattr(-, root, root)

%{prefix}/lib/lib*.so
%{prefix}/lib/*a
%{prefix}/include/*


%changelog
* Wed May 13 1998 Michael Fulbright <msf@redhat.com>

- First try at an RPM




