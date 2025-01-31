
dnl vala.m4
dnl
dnl Copyright 2010 Marc-Andre Lureau
dnl Copyright 2011 Rodney Dawes <dobey.pwns@gmail.com>
dnl
dnl This library is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU Lesser General Public
dnl License as published by the Free Software Foundation; either
dnl version 2.1 of the License, or (at your option) any later version.
dnl
dnl This library is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl Lesser General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public
dnl License along with this library; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

dnl dropped everything but VALA_PROG_VAPIGEN - Jens Georg <mail@jensge.org>

# Check whether the Vala API Generator exists in `PATH'. If it is found,
# the variable VAPIGEN is set. Optionally a minimum release number of the
# generator can be requested.
#
# VALA_PROG_VAPIGEN([MINIMUM-VERSION])
# ------------------------------------
AC_DEFUN([VALA_PROG_VAPIGEN],
[AC_PATH_PROG([VAPIGEN], [vapigen], [])
  AS_IF([test -z "$VAPIGEN"],
    [AC_MSG_WARN([No Vala API Generator found. You will not be able to generate .vapi files.])],
    [AS_IF([test -n "$1"],
        [AC_MSG_CHECKING([$VAPIGEN is at least version $1])
         am__vapigen_version=`$VAPIGEN --version | sed 's/Vala API Generator  *//'`
         AS_VERSION_COMPARE([$1], ["$am__vapigen_version"],
           [AC_MSG_RESULT([yes])],
           [AC_MSG_RESULT([yes])],
           [AC_MSG_RESULT([no])
            AC_MSG_ERROR([Vala API Generator $1 not found.])])])])
])
