AM_CFLAGS = \
	$(WARN_CFLAGS) \
	$(VISIBILITY_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/nexell \
	-I$(top_srcdir)/include/drm

libdrm_nexell_la_LTLIBRARIES = libdrm_nexell.la
libdrm_nexell_ladir = $(libdir)
libdrm_nexell_la_LDFLAGS = -version-number 1:0:0 -no-undefined
libdrm_nexell_la_LIBADD = ../libdrm.la

libdrm_nexell_la_SOURCES = \
	nexell_drm.c

libdrm_nexellcommonincludedir = ${includedir}/nexell
libdrm_nexellcommoninclude_HEADERS = nexell_drm.h
libdrm_nexellincludedir = ${includedir}/libdrm
libdrm_nexellinclude_HEADERS = nexell_drmif.h

pkgconfigdir = @pkgconfigdir@
pkgconfig_DATA = libdrm_nexell.pc
