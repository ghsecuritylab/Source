OBJS = disk_io_tools.o disk_initial.o disk_share.o disk_swap.o

TOP = $(ROOTDIR)/user


CFLAGS += -DASUS_NVRAM
CFLAGS += -I. -I$(TOP)/shared -I$(TOP)/shared/include -I$(ROOTDIR)/linux-2.6.21.x/include -Wall
#LDFLAGS = -L. -ldisk -L$(TOP)/nvram -lnvram -L$(TOP)/shared -lshared
LDFLAGS = -L. -ldisk -L$(TOP)/shared -lshared

all: libdisk.so initial_var_file write_smb_conf test_of_var_files_in_mount_path create_if_no_var_files initial_folder_list_in_mount_path

libdisk.so: $(OBJS) $(TOP)/shared/shutils.o
	$(LD) -shared -o $@ $^

initial_var_file: initial_var_file.o libdisk.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

write_smb_conf: write_smb_conf.o libdisk.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

test_of_var_files_in_mount_path: test_of_var_files_in_mount_path.o libdisk.so# $(TOP)/shared/shutils.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

create_if_no_var_files: create_if_no_var_files.o libdisk.so# $(TOP)/shared/shutils.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

initial_folder_list_in_mount_path: initial_folder_list_in_mount_path.o libdisk.so# $(TOP)/shared/shutils.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

romfs: all
	$(STRIP) -s libdisk.so
	$(ROMFSINST) /lib/libdisk.so
	$(ROMFSINST) /sbin/initial_var_file	# initial permission
	$(ROMFSINST) /sbin/write_smb_conf
	$(ROMFSINST) /sbin/test_of_var_files_in_mount_path	# test if permission file exist

clean:
	rm -f *.o libdisk.so initial_var_file write_smb_conf test_of_var_files_in_mount_path create_if_no_var_files initial_folder_list_in_mount_path
