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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "disk_io_tools.h"

extern char *read_whole_file(const char *target){
	FILE *fp = fopen(target, "r");
	char *buffer, *new_str;
	int i;
	unsigned int read_bytes = 0;
	unsigned int each_size = 1024;
	
	if(fp == NULL)
		return NULL;
	
	buffer = (char *)malloc(sizeof(char)*each_size+read_bytes);
	if(buffer == NULL){
		csprintf("No memory \"buffer\".\n");
		fclose(fp);
		return NULL;
	}
	memset(buffer, 0, sizeof(char)*each_size+read_bytes);
	
	while((i = fread(buffer+read_bytes, sizeof(char), each_size, fp)) == each_size){
		read_bytes += each_size;
		new_str = (char *)malloc(sizeof(char)*each_size+read_bytes);
		if(new_str == NULL){
			csprintf("No memory \"new_str\".\n");
			free(buffer);
			fclose(fp);
			return NULL;
		}
		memset(new_str, 0, sizeof(char)*each_size+read_bytes);
		memcpy(new_str, buffer, read_bytes);
		
		free(buffer);
		buffer = new_str;
	}
	
	fclose(fp);
	return buffer;
}

extern int get_mount_path(const char *const pool, char **mount_path){
	int len = strlen(POOL_MOUNT_ROOT)+strlen("/")+strlen(pool);
	char *tmppath = (char *)malloc(sizeof(char)*(len+1));
	
	if(tmppath == NULL){
		csprintf("No memory \"tmppath\".\n");
		return -1;
	}
	sprintf(tmppath, "%s/%s", POOL_MOUNT_ROOT, pool);
	tmppath[len] = 0;
	
	if(!test_if_mount_point_of_pool(tmppath)){
		free(tmppath);
		return -1;
	}
		
	*mount_path = tmppath;

	return 0;
}

extern int mkdir_if_none(char *dir){
	DIR *dp = opendir(dir);
	if(dp != NULL){
		closedir(dp);
		return 0;
	}
	
	umask(0000);
	return (!mkdir(dir, 0777))?1:0;
}

extern int delete_file_or_dir(char *target){
	if(test_if_dir(target))
		rmdir(target);
	else
		unlink(target);
	
	return 0;
}

extern int test_if_file(const char *file){
	FILE *fp = fopen(file, "r");
	
	if(fp == NULL)
		return 0;
	
	fclose(fp);
	return 1;
}

extern int test_if_dir(const char *dir){
	DIR *dp = opendir(dir);
	
	if(dp == NULL)
		return 0;
	
	closedir(dp);
	return 1;
}

extern int test_if_mount_point_of_pool(const char *dirname){
/*	// for n12
	char *mount_dir, *follow_info;
	
	mount_dir = rindex(dirname, '/');
	if(mount_dir == NULL)
		return 0;
	
	++mount_dir;
	if(*mount_dir == 0 || isspace(*mount_dir))
		return 0;
	
	//if(!strncmp(mount_dir, ".", NAME_MAX) || !strncmp(mount_dir, "..", NAME_MAX))
	if(mount_dir[0] == '.')
		return 0;

	if(!test_if_dir(dirname))
		return 0;
	
	if(strncmp(mount_dir, "Harddisk_part", 13) && strncmp(mount_dir, "USBdisks_part", 13))
		return 0;
	else{
		for(follow_info = (char *)mount_dir+13; *follow_info != 0 && !isspace(*follow_info); ++follow_info)
			if(!isdigit(*follow_info))
				return 0;
	}
*/	
	return 1;
}

extern int test_if_System_folder(const char *const dirname){
	char *MS_System_folder[] = {"SYSTEM VOLUME INFORMATION", "RECYCLER", "RECYCLED", NULL};
	char *Linux_System_folder[] = {"lost+found", NULL};
	int i;
	
	for(i = 0; MS_System_folder[i] != NULL; ++i){
		if(!upper_strcmp(dirname, MS_System_folder[i]))
			return 1;
	}
	
	for(i = 0; Linux_System_folder[i] != NULL; ++i){
		if(!upper_strcmp(dirname, Linux_System_folder[i]))
			return 1;
	}
	
	return 0;
}

extern int test_mounted_disk_size_status(char *diskpath){
	struct statfs fsbuf;
	unsigned long long block_size;

	if(statfs(diskpath, &fsbuf)){
		perror("*** statfs fail! - in test_mounted_disk_size_status()");
		return 0;
	}
	
	block_size = fsbuf.f_bsize;
	if(block_size*fsbuf.f_bfree/(1<<20) < (unsigned long long)33)
		return 1;
	else if(block_size*fsbuf.f_blocks/(1<<20) > (unsigned long long)256)
		return 2;
	else
		return 3;
}

extern char *get_upper_str(const char *const str){
	int len, i;
	char *target;
	
	len = strlen(str);
	target = (char *)malloc(sizeof(char)*(len+1));
	if(target == NULL){
		csprintf("No memory \"target\".\n");
		return NULL;
	}
	for(i = 0; i < len; ++i)
		target[i] = toupper(str[i]);
	target[len] = 0;
	
	return target;
}

extern int upper_strcmp(const char *const str1, const char *const str2){
	int len1, len2, i;
	
	len1 = strlen(str1);
	len2 = strlen(str2);
	if(len1 != len2)
		return len1-len2;
	
	for(i = 0; i < len1; ++i){
		if(toupper(str1[i]) != toupper(str2[i]))
			return i+1;
	}
	
	return 0;
}

extern int upper_strncmp(const char *const str1, const char *const str2, int max_len){
	int i;
	
	for(i = 0; i < max_len; ++i){
		if(toupper(str1[i]) != toupper(str2[i]))
			return i+1;
	}
	
	return 0;
}

extern char *upper_strstr(const char *const str1, const char *const str2){
	char *line, *line_end, *line_str;
	int len, ret;
	
	line = (char *)str1;
	while(line != NULL){
		line_end = strchr(line, '\n');
		if(line_end != NULL)
			len = line_end-line;
		else
			len = strlen(line);
		
		line_str = (char *)malloc(sizeof(char)*(len+1));
		if(line_str == NULL){
			csprintf("upper_strstr: Can't malloc memory!\n");
			return 0;
		}
		strncpy(line_str, line, len);
		line_str[len] = 0;
		
		len = strlen(str2);
		ret = upper_strncmp(line_str, str2, len);
		free(line_str);
		if(ret == 0)
			return line;
		
		line = strchr(line_end, '*');
	}
	
	return NULL;
}

extern void strntrim(char *str, size_t len){
	register char *start = str;
	register char *end = start+len-1;

	while(start <  end && isspace(*start))
		++start;
	while(start <= end && isspace(*end))
		--end;

	end++;

	if((int)(end - start) < len){
		memcpy(str, start, (end-start));
		str[end-start] = 0;
	}

	return;
}

extern void write_escaped_value(FILE *fp, const char *value){
	const char *follow_value;

	follow_value = value;
	while(*follow_value != 0){
		if(*follow_value == '\''){
			fputc('\\', fp);
			fputc('\'', fp);
		}
		else if(*follow_value == '\"'){
			fputc('\\', fp);
			fputc('\"', fp);
		}
		else if(*follow_value == '\?'){
			fputc('\\', fp);
			fputc('\?', fp);
		}
		else if(*follow_value == '\\'){
			fputc('\\', fp);
			fputc('\\', fp);
		}
		else if(*follow_value == '\a'){
			fputc('\\', fp);
			fputc('a', fp);
		}
		else if(*follow_value == '\b'){
			fputc('\\', fp);
			fputc('b', fp);
		}
		else if(*follow_value == '\f'){
			fputc('\\', fp);
			fputc('f', fp);
		}
		else if(*follow_value == '\n'){
			fputc('\\', fp);
			fputc('n', fp);
		}
		else if(*follow_value == '\r'){
			fputc('\\', fp);
			fputc('r', fp);
		}
		else if(*follow_value == '\t'){
			fputc('\\', fp);
			fputc('\t', fp);
		}
		else if(*follow_value == '\v'){
			fputc('\\', fp);
			fputc('\v', fp);
		}
		else if(isprint(*follow_value)){
			fputc(*follow_value, fp);
		}
		else if(((unsigned char)(*follow_value) && 0x80) != 0){
			fputc(*follow_value, fp);
		}
		else{
			fprintf(fp, "\\%03o", (unsigned)(unsigned char)(*follow_value));
		}
		++follow_value;
	}
}
