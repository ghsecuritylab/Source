%define majorminor  0.10
%define gstreamer   gstreamer

%define gst_minver  0.10.0

Name: 		%{gstreamer}-plugins-base
Version: 	0.10.36
Release: 	1.gst
Summary: 	GStreamer streaming media framework plug-ins

Group: 		Applications/Multimedia
License: 	LGPL
URL:		http://gstreamer.freedesktop.org/
Vendor:         GStreamer Backpackers Team <package@gstreamer.freedesktop.org>
Source:         http://gstreamer.freedesktop.org/src/gst-plugins-base/gst-plugins-base-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires: 	  %{gstreamer} >= %{gst_minver}
BuildRequires: 	  %{gstreamer}-devel >= %{gst_minver}

BuildRequires:  gcc-c++
BuildRequires:  gtk-doc >= 1.3
BuildRequires:  orc-devel
Requires:       orc

#Requires:      gnome-vfs2 > 1.9.4.00
#BuildRequires: gnome-vfs2-devel > 1.9.4.00
#Requires:      libogg >= 1.0
#Requires:      libvorbis >= 1.0
#BuildRequires: libogg-devel >= 1.0
#BuildRequires: libvorbis-devel >= 1.0
#BuildRequires:  libXv-devel
#Requires:       libXv-devel
BuildRequires: alsa-lib-devel
Requires:      alsa-lib
#BuildRequires: cdparanoia-devel
#Requires:      cdparanoia
#BuildRequires: libvisual-devel
#Requires:      libvisual
#BuildRequires: pango-devel
#Requires: pango
#BuildRequires: libtheora-devel >= 1.0
#Requires:      libtheora >= 1.0

%description
GStreamer is a streaming media framework, based on graphs of filters which
operate on media data. Applications using this library can do anything
from real-time sound processing to playing videos, and just about anything
else media-related.  Its plugin-based architecture means that new data
types or processing capabilities can be added simply by installing new
plug-ins.

%prep
%setup -q -n gst-plugins-base-%{version}
export DOCS_ARE_INCOMPLETE_PLEASE_FIXME=0
%build
%configure \
  --enable-gtk-doc --enable-introspection=yes

make %{?_smp_mflags}
                                                                                
%install
rm -rf $RPM_BUILD_ROOT

%makeinstall

# Clean out files that should not be part of the rpm.
rm -f $RPM_BUILD_ROOT%{_libdir}/gstreamer-%{majorminor}/*.la
rm -f $RPM_BUILD_ROOT%{_libdir}/gstreamer-%{majorminor}/*.a
rm -f $RPM_BUILD_ROOT%{_libdir}/*.a
rm -f $RPM_BUILD_ROOT%{_libdir}/*.la

%find_lang gst-plugins-base-%{majorminor}

%clean
rm -rf $RPM_BUILD_ROOT

%files -f gst-plugins-base-%{majorminor}.lang
%defattr(-, root, root)
%doc AUTHORS COPYING README REQUIREMENTS gst-plugins-base.doap

# helper programs
%{_bindir}/gst-visualise-%{majorminor}
%{_bindir}/gst-discoverer-%{majorminor}
%{_mandir}/man1/gst-visualise-%{majorminor}*

# libraries
%{_libdir}/libgstaudio-%{majorminor}.so.*
%{_libdir}/libgstcdda-%{majorminor}.so*
%{_libdir}/libgstinterfaces-%{majorminor}.so.*
%{_libdir}/libgstnetbuffer-%{majorminor}.so*
%{_libdir}/libgstpbutils-%{majorminor}.so*
%{_libdir}/libgstriff-%{majorminor}.so.*
%{_libdir}/libgstrtp-%{majorminor}.so*
%{_libdir}/libgsttag-%{majorminor}.so.*
%{_libdir}/libgstvideo-%{majorminor}.so.*
%{_libdir}/libgstfft-%{majorminor}.so.*
%{_libdir}/libgstrtsp-%{majorminor}.so.*
%{_libdir}/libgstsdp-%{majorminor}.so.*
%{_libdir}/libgstapp-%{majorminor}.so.*

# base plugins without external dependencies
%{_libdir}/gstreamer-%{majorminor}/libgstadder.so
%{_libdir}/gstreamer-%{majorminor}/libgstaudioconvert.so
%{_libdir}/gstreamer-%{majorminor}/libgstffmpegcolorspace.so
%{_libdir}/gstreamer-%{majorminor}/libgstdecodebin.so
%{_libdir}/gstreamer-%{majorminor}/libgstdecodebin2.so
%{_libdir}/gstreamer-%{majorminor}/libgstplaybin.so
%{_libdir}/gstreamer-%{majorminor}/libgsttypefindfunctions.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideotestsrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstaudiorate.so
%{_libdir}/gstreamer-%{majorminor}/libgstsubparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstvolume.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideorate.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideoscale.so
%{_libdir}/gstreamer-%{majorminor}/libgsttcp.so
%{_libdir}/gstreamer-%{majorminor}/libgstaudioresample.so
%{_libdir}/gstreamer-%{majorminor}/libgstaudiotestsrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstgdp.so
%{_libdir}/gstreamer-%{majorminor}/libgstapp.so
%{_libdir}/gstreamer-%{majorminor}/libgstencodebin.so

# Here are packages not in the base plugins package but not dependant
# on an external lib

# @USE_GST_V4L2_TRUE@%{_libdir}/gstreamer-%{majorminor}/libgstvideo4linux2.so

# base plugins with dependencies
%{_libdir}/gstreamer-%{majorminor}/libgstalsa.so
#%{_libdir}/gstreamer-%{majorminor}/libgsttheora.so
#%{_libdir}/gstreamer-%{majorminor}/libgstgnomevfs.so
#%{_libdir}/gstreamer-%{majorminor}/libgstvorbis.so
#%{_libdir}/gstreamer-%{majorminor}/libgstogg.so
#%{_libdir}/gstreamer-%{majorminor}/libgstximage*.so
#%{_libdir}/gstreamer-%{majorminor}/libgstxvimagesink.so
#%{_libdir}/gstreamer-%{majorminor}/libgstlibvisual.so
#%{_libdir}/gstreamer-%{majorminor}/libgstpango.so
#%{_libdir}/gstreamer-%{majorminor}/libgstcdparanoia.so
%{_libdir}/gstreamer-%{majorminor}/libgstgio.so
# @USE_SCHRO_TRUE@%{_libdir}/gstreamer-%{majorminor}/libgstschro.so                                                                     
%package devel
Summary: 	GStreamer Plugin Library Headers
Group: 		Development/Libraries
Requires: 	%{gstreamer}-plugins-base = %{version}

%description devel
GStreamer Plugins Base library development and header files.

%files devel
%defattr(-, root, root)
# plugin helper library headers
%{_includedir}/gstreamer-%{majorminor}/gst/audio/audio.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/gstaudiofilter.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/multichannel.h
%{_includedir}/gstreamer-%{majorminor}/gst/floatcast/floatcast.h
%{_includedir}/gstreamer-%{majorminor}/gst/riff/riff-ids.h
%{_includedir}/gstreamer-%{majorminor}/gst/riff/riff-media.h
%{_includedir}/gstreamer-%{majorminor}/gst/riff/riff-read.h
%{_includedir}/gstreamer-%{majorminor}/gst/video/video.h
%{_includedir}/gstreamer-%{majorminor}/gst/interfaces/colorbalance.h
%{_includedir}/gstreamer-%{majorminor}/gst/interfaces/colorbalancechannel.h
%{_includedir}/gstreamer-%{majorminor}/gst/interfaces/interfaces-enumtypes.h
%{_includedir}/gstreamer-%{majorminor}/gst/interfaces/mixer.h
%{_includedir}/gstreamer-%{majorminor}/gst/interfaces/mixeroptions.h
%{_includedir}/gstreamer-%{majorminor}/gst/interfaces/mixertrack.h
%{_includedir}/gstreamer-%{majorminor}/gst/interfaces/navigation.h
%{_includedir}/gstreamer-%{majorminor}/gst/interfaces/propertyprobe.h
%{_includedir}/gstreamer-%{majorminor}/gst/interfaces/tuner.h
%{_includedir}/gstreamer-%{majorminor}/gst/interfaces/tunerchannel.h
%{_includedir}/gstreamer-%{majorminor}/gst/interfaces/tunernorm.h
%{_includedir}/gstreamer-%{majorminor}/gst/interfaces/xoverlay.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/gstaudiosrc.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/gstbaseaudiosrc.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtp/gstbasertpaudiopayload.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtp/gstbasertpdepayload.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtp/gstrtpbuffer.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/gstaudioclock.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/gstaudiosink.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/gstbaseaudiosink.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/gstringbuffer.h
%{_includedir}/gstreamer-%{majorminor}/gst/tag/tag.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtp/gstbasertppayload.h
%{_includedir}/gstreamer-%{majorminor}/gst/video/gstvideofilter.h
%{_includedir}/gstreamer-%{majorminor}/gst/video/gstvideosink.h
%{_includedir}/gstreamer-%{majorminor}/gst/netbuffer/gstnetbuffer.h
%{_includedir}/gstreamer-%{majorminor}/gst/cdda/gstcddabasesrc.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/mixerutils.h
%{_includedir}/gstreamer-%{majorminor}/gst/interfaces/videoorientation.h
%{_includedir}/gstreamer-%{majorminor}/gst/pbutils/descriptions.h
%{_includedir}/gstreamer-%{majorminor}/gst/pbutils/install-plugins.h
%{_includedir}/gstreamer-%{majorminor}/gst/pbutils/missing-plugins.h
%{_includedir}/gstreamer-%{majorminor}/gst/pbutils/pbutils.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtp/gstrtcpbuffer.h
%{_includedir}/gstreamer-%{majorminor}/gst/fft/gstfft.h
%{_includedir}/gstreamer-%{majorminor}/gst/fft/gstfftf32.h
%{_includedir}/gstreamer-%{majorminor}/gst/fft/gstfftf64.h
%{_includedir}/gstreamer-%{majorminor}/gst/fft/gstffts16.h
%{_includedir}/gstreamer-%{majorminor}/gst/fft/gstffts32.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtsp/gstrtsp-enumtypes.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtsp/gstrtspbase64.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtsp/gstrtspconnection.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtsp/gstrtspdefs.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtsp/gstrtspextension.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtsp/gstrtspmessage.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtsp/gstrtsprange.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtsp/gstrtsptransport.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtsp/gstrtspurl.h
%{_includedir}/gstreamer-%{majorminor}/gst/sdp/gstsdp.h
%{_includedir}/gstreamer-%{majorminor}/gst/sdp/gstsdpmessage.h
%{_includedir}/gstreamer-%{majorminor}/gst/rtp/gstrtppayloads.h
%{_includedir}/gstreamer-%{majorminor}/gst/tag/gsttagdemux.h
%{_includedir}/gstreamer-%{majorminor}/gst/pbutils/pbutils-enumtypes.h
%{_includedir}/gstreamer-%{majorminor}/gst/app/gstappbuffer.h
%{_includedir}/gstreamer-%{majorminor}/gst/app/gstappsink.h
%{_includedir}/gstreamer-%{majorminor}/gst/app/gstappsrc.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/audio-enumtypes.h
%{_includedir}/gstreamer-%{majorminor}/gst/video/video-enumtypes.h
%{_includedir}/gstreamer-%{majorminor}/gst/interfaces/streamvolume.h
%{_includedir}/gstreamer-%{majorminor}/gst/pbutils/codec-utils.h
%{_includedir}/gstreamer-%{majorminor}/gst/pbutils/encoding-profile.h
%{_includedir}/gstreamer-%{majorminor}/gst/pbutils/encoding-target.h
%{_includedir}/gstreamer-%{majorminor}/gst/pbutils/gstdiscoverer.h
%{_includedir}/gstreamer-%{majorminor}/gst/pbutils/gstpluginsbaseversion.h
%{_includedir}/gstreamer-%{majorminor}/gst/tag/xmpwriter.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/gstaudioiec61937.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/gstaudiodecoder.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/gstaudioencoder.h
%{_includedir}/gstreamer-%{majorminor}/gst/tag/gsttagmux.h
%{_includedir}/gstreamer-%{majorminor}/gst/video/video-overlay-composition.h

%{_libdir}/libgstfft-%{majorminor}.so
%{_libdir}/libgstrtsp-%{majorminor}.so
%{_libdir}/libgstsdp-%{majorminor}.so
%{_libdir}/libgstaudio-%{majorminor}.so
%{_libdir}/libgstriff-%{majorminor}.so
%{_libdir}/libgsttag-%{majorminor}.so
%{_libdir}/libgstvideo-%{majorminor}.so
%{_libdir}/libgstrtp-%{majorminor}.so
%{_libdir}/libgstinterfaces-%{majorminor}.so
%{_libdir}/libgstnetbuffer-%{majorminor}.so
%{_libdir}/libgstpbutils-%{majorminor}.so
%{_libdir}/libgstcdda-%{majorminor}.so
%{_libdir}/libgstapp-%{majorminor}.so

# pkg-config files
%{_libdir}/pkgconfig/gstreamer-plugins-base-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-audio-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-cdda-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-fft-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-floatcast-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-interfaces-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-netbuffer-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-pbutils-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-riff-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-rtp-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-rtsp-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-sdp-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-tag-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-video-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-app-%{majorminor}.pc

%{_libdir}/girepository-1.0/GstApp-0.10.typelib
%{_libdir}/girepository-1.0/GstAudio-0.10.typelib
%{_libdir}/girepository-1.0/GstFft-0.10.typelib
%{_libdir}/girepository-1.0/GstInterfaces-0.10.typelib
%{_libdir}/girepository-1.0/GstNetbuffer-0.10.typelib
%{_libdir}/girepository-1.0/GstPbutils-0.10.typelib
%{_libdir}/girepository-1.0/GstRiff-0.10.typelib
%{_libdir}/girepository-1.0/GstRtp-0.10.typelib
%{_libdir}/girepository-1.0/GstRtsp-0.10.typelib
%{_libdir}/girepository-1.0/GstSdp-0.10.typelib
%{_libdir}/girepository-1.0/GstTag-0.10.typelib
%{_libdir}/girepository-1.0/GstVideo-0.10.typelib
%{_datadir}/gir-1.0/GstApp-0.10.gir
%{_datadir}/gir-1.0/GstAudio-0.10.gir
%{_datadir}/gir-1.0/GstFft-0.10.gir
%{_datadir}/gir-1.0/GstInterfaces-0.10.gir
%{_datadir}/gir-1.0/GstNetbuffer-0.10.gir
%{_datadir}/gir-1.0/GstPbutils-0.10.gir
%{_datadir}/gir-1.0/GstRiff-0.10.gir
%{_datadir}/gir-1.0/GstRtp-0.10.gir
%{_datadir}/gir-1.0/GstRtsp-0.10.gir
%{_datadir}/gir-1.0/GstSdp-0.10.gir
%{_datadir}/gir-1.0/GstTag-0.10.gir
%{_datadir}/gir-1.0/GstVideo-0.10.gir

# gtk-doc documentation
%doc %{_datadir}/gtk-doc/html/gst-plugins-base-libs-%{majorminor}
%doc %{_datadir}/gtk-doc/html/gst-plugins-base-plugins-%{majorminor}
%doc %{_datadir}/gst-plugins-base/license-translations.dict

%changelog
* Fri Dec 15 2006 Thomas Vander Stichele <thomas at apestaart dot org>
- add doap file
- cleanups

* Fri Sep 02 2005 Thomas Vander Stichele <thomas at apestaart dot org>
- clean up a little

* Fri May 6 2005 Christian Schaller <christian at fluendo dot com>
- Added libgstaudiorate and libgstsubparse to spec file

* Thu May 5 2005 Christian Schaller <christian at fluendo dot com>
- first attempt at spec file for gst-plugins-base
