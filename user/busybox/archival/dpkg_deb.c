/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/* vi: set sw=4 ts=4: */
/*
 * dpkg-deb packs, unpacks and provides information about Debian archives.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */
#include "libbb.h"
#include "unarchive.h"

#define DPKG_DEB_OPT_CONTENTS	1
#define DPKG_DEB_OPT_CONTROL	2
#define DPKG_DEB_OPT_FIELD	4
#define DPKG_DEB_OPT_EXTRACT	8
#define DPKG_DEB_OPT_EXTRACT_VERBOSE	16

int dpkg_deb_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int dpkg_deb_main(int argc, char **argv)
{
	archive_handle_t *ar_archive;
	archive_handle_t *tar_archive;
	llist_t *control_tar_llist = NULL;
	unsigned opt;
	const char *extract_dir;
	int need_args;

	/* Setup the tar archive handle */
	tar_archive = init_handle();

	/* Setup an ar archive handle that refers to the gzip sub archive */
	ar_archive = init_handle();
	ar_archive->sub_archive = tar_archive;
	ar_archive->filter = filter_accept_list_reassign;

#if ENABLE_FEATURE_SEAMLESS_GZ
	llist_add_to(&(ar_archive->accept), (char*)"data.tar.gz");
	llist_add_to(&control_tar_llist, (char*)"control.tar.gz");
#endif

#if ENABLE_FEATURE_SEAMLESS_BZ2
	llist_add_to(&(ar_archive->accept), (char*)"data.tar.bz2");
	llist_add_to(&control_tar_llist, (char*)"control.tar.bz2");
#endif

	opt_complementary = "c--efXx:e--cfXx:f--ceXx:X--cefx:x--cefX";
	opt = getopt32(argv, "cefXx");
	argv += optind;
	argc -= optind;

	if (opt & DPKG_DEB_OPT_CONTENTS) {
		tar_archive->action_header = header_verbose_list;
	}
	extract_dir = NULL;
	need_args = 1;
	if (opt & DPKG_DEB_OPT_CONTROL) {
		ar_archive->accept = control_tar_llist;
		tar_archive->action_data = data_extract_all;
		if (1 == argc) {
			extract_dir = "./DEBIAN";
		} else {
			need_args++;
		}
	}
	if (opt & DPKG_DEB_OPT_FIELD) {
		/* Print the entire control file
		 * it should accept a second argument which specifies a
		 * specific field to print */
		ar_archive->accept = control_tar_llist;
		llist_add_to(&(tar_archive->accept), (char*)"./control");
		tar_archive->filter = filter_accept_list;
		tar_archive->action_data = data_extract_to_stdout;
	}
	if (opt & DPKG_DEB_OPT_EXTRACT) {
		tar_archive->action_header = header_list;
	}
	if (opt & (DPKG_DEB_OPT_EXTRACT_VERBOSE | DPKG_DEB_OPT_EXTRACT)) {
		tar_archive->action_data = data_extract_all;
		need_args = 2;
	}

	if (need_args != argc) {
		bb_show_usage();
	}

	tar_archive->src_fd = ar_archive->src_fd = xopen(argv[0], O_RDONLY);

	/* Work out where to extract the files */
	/* 2nd argument is a dir name */
	if (argv[1]) {
		extract_dir = argv[1];
	}
	if (extract_dir) {
		mkdir(extract_dir, 0777); /* bb_make_directory(extract_dir, 0777, 0) */
		xchdir(extract_dir);
	}

	/* Do it */
	unpack_ar_archive(ar_archive);

	/* Cleanup */
	if (ENABLE_FEATURE_CLEAN_UP)
		close(ar_archive->src_fd);

	return EXIT_SUCCESS;
}
