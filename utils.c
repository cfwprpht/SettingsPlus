/*
	Settings+ Flow Plugin
	Copyright (C) 2016, TheFloW

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <psp2/moduleinfo.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int debugPrintf(char *text, ...) {
	va_list list;
	char string[512];

	va_start(list, text);
	vsprintf(string, text, list);
	va_end(list);

	SceUID fd = sceIoOpen("ux0:flow_log.txt", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
	if (fd >= 0) {
		sceIoWrite(fd, string, strlen(string));
		sceIoClose(fd);
	}

	return 0;
}

int ReadFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	int read = sceIoRead(fd, buf, size);

	sceIoClose(fd);
	return read;
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

static void convertToImportsTable3xx(SceImportsTable2xx *import_2xx, SceImportsTable3xx *import_3xx)
{
	memset(import_3xx, 0, sizeof(SceImportsTable3xx));

	if (import_2xx->size == sizeof(SceImportsTable2xx)) {
		import_3xx->size = import_2xx->size;
		import_3xx->lib_version = import_2xx->lib_version;
		import_3xx->attribute = import_2xx->attribute;
		import_3xx->num_functions = import_2xx->num_functions;
		import_3xx->num_vars = import_2xx->num_vars;
		import_3xx->module_nid = import_2xx->module_nid;
		import_3xx->lib_name = import_2xx->lib_name;
		import_3xx->func_nid_table = import_2xx->func_nid_table;
		import_3xx->func_entry_table = import_2xx->func_entry_table;
		import_3xx->var_nid_table = import_2xx->var_nid_table;
		import_3xx->var_entry_table = import_2xx->var_entry_table;
	} else if (import_2xx->size == sizeof(SceImportsTable3xx)) {
		memcpy(import_3xx, import_2xx, sizeof(SceImportsTable3xx));
	}
}

uint32_t findModuleImportNumber(SceModuleInfo *mod_info, uint32_t text_addr, char *libname, uint32_t nid) {
	if (!mod_info)
		return 0;

	int count = 0;

	uint32_t i = mod_info->impTop;
	while (i < mod_info->impBtm) {
		SceImportsTable3xx import;
		convertToImportsTable3xx((void *)text_addr + i, &import);

		if (import.lib_name && strcmp(import.lib_name, libname) == 0) {
			int j;
			for (j = 0; j < import.num_functions; j++) {
				if (import.func_nid_table[j] == nid) {
					return count + j;
				}
			}
		}

		count += import.num_functions;
		i += import.size;
	}

	return 0;
}

uint32_t countImports(SceModuleInfo *mod_info, uint32_t text_addr) {
	if (!mod_info)
		return 0;

	int count = 0;

	uint32_t i = mod_info->impTop;
	while (i < mod_info->impBtm) {
		SceImportsTable3xx import;
		convertToImportsTable3xx((void *)text_addr + i, &import);

		count += import.num_functions;

		i += import.size;
	}

	return count;
}

SceModuleInfo *findModuleInfo(char *modname, uint32_t text_addr, uint32_t text_size) {
	SceModuleInfo *mod_info = NULL;

	uint32_t i;
	for (i = 0; i < text_size; i += 4) {
		if (strcmp((char *)(text_addr + i), modname) == 0) {
			mod_info = (SceModuleInfo *)(text_addr + i - 0x4);
			break;
		}
	}

	return mod_info;
}