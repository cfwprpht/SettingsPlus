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

#include <psp2/ctrl.h>
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

#define MAX_MODULES 128

#define BASE_DIRECTORY "ux0:app/FLOW10015/"

typedef struct {
	SceUInt size;
	SceChar8 version_string[28];
	SceUInt version_value;
	SceUInt unk;
} SceSystemSwVersionParam;

int sceKernelGetSystemSwVersion(SceSystemSwVersionParam *param);

void *(* sce_paf_private_malloc)(size_t size);
int (* ScePafMisc_19FE55A8)(int a1, void *xml_buf, int xml_size, int a4);

int sceKernelGetSystemSwVersionPatched(SceSystemSwVersionParam *param) {
	int res = sceKernelGetSystemSwVersion(param);

	char version[28];
	memset(version, 0, sizeof(version));
	if (ReadFile(BASE_DIRECTORY "version.txt", version, sizeof(version)) > 0) {
		memset(param->version_string, 0, sizeof(version));
		strcpy((char *)param->version_string, version);
	}

	return res;
}

int sce_paf_private_snprintf_patched(char *s, size_t n, const char *format, ...) {
	if (n == 18 && strcmp(format, "%02X:%02X:%02X:%02X:%02X:%02X") == 0) {
		char mac[18];
		memset(mac, 0, sizeof(mac));
		if (ReadFile(BASE_DIRECTORY "mac.txt", mac, sizeof(mac)) > 0) {
			memset(s, 0, n);
			strcpy(s, mac);
			return 0;
		}
	}

	va_list list;
	va_start(list, format);
	int res = vsnprintf(s, n, format, list);
	va_end(list);

	return res;
}

int ScePafMisc_19FE55A8_Patched(int a1, void *xml_buf, int xml_size, int a4) {
	if (strncmp(xml_buf + 0x4F, "console_info_plugin", strlen("console_info_plugin")) == 0) {
		WriteFile("ux0:console_info.xml", xml_buf, xml_size);

		void *new_buf = sce_paf_private_malloc(xml_size);
		int new_size = ReadFile(BASE_DIRECTORY "console_info.xml", new_buf, xml_size);
		if (new_size > 0) {
			xml_buf = new_buf;
			xml_size = new_size;
		}
	}

	if (strncmp(xml_buf + 0x52, "system_settings_plugin", strlen("system_settings_plugin")) == 0) {
		WriteFile("ux0:system_settings.xml", xml_buf, xml_size);

		void *new_buf = sce_paf_private_malloc(xml_size);
		int new_size = ReadFile(BASE_DIRECTORY "system_settings.xml", new_buf, xml_size);
		if (new_size > 0) {
			xml_buf = new_buf;
			xml_size = new_size;
		}
	}

	return ScePafMisc_19FE55A8(a1, xml_buf, xml_size, a4);
}

void PatchSceSettings(uint32_t *stub_list) {
	/*
		0xE148AF94: sce_paf_private_memset
		0x5CD08A47: sce_paf_private_strcmp
		0x2C5B6F9C: sce_paf_private_strncpy
		0xFC5CD359: sce_paf_private_malloc
		0xF5A2AA0C: sce_paf_private_strlen
	*/

	sce_paf_private_malloc = (void *)stub_list[926];					// 0xFC5CD359

	stub_list[176] = (uint32_t)sceKernelGetSystemSwVersionPatched;		// 0x5182E212
	stub_list[889] = (uint32_t)sce_paf_private_snprintf_patched;		// 0x4E0D907E

	ScePafMisc_19FE55A8 = (void *)stub_list[948];
	stub_list[948] = (uint32_t)ScePafMisc_19FE55A8_Patched;				// 0x19FE55A8
}

int _free_vita_newlib() {
	return 0;
}

int _start(SceSize argsize, uint32_t *arg) {
	int res;

	SceUID mod_list[MAX_MODULES];
	int mod_count = MAX_MODULES;

	res = sceKernelGetModuleList(0xFF, mod_list, &mod_count);
	if (res < 0)
		return res;

	SceKernelModuleInfo info;
	info.size = sizeof(SceKernelModuleInfo);
	res = sceKernelGetModuleInfo(mod_list[mod_count - 1], &info);
	if (res < 0)
		return res;

	uint32_t text_addr = (uint32_t)info.segments[0].vaddr;
	uint32_t text_size = (uint32_t)info.segments[0].memsz;

	uint32_t data_addr = (uint32_t)info.segments[1].vaddr;
	uint32_t data_size = (uint32_t)info.segments[1].memsz;

	SceModuleInfo *mod_info = findModuleInfo(info.module_name, text_addr, text_size);
	int count = countImports(mod_info, text_addr);

	uint32_t *stub_list = (uint32_t *)(data_addr + data_size - count * 0x4);

	if (strcmp(info.module_name, "SceSettings") == 0) {
		PatchSceSettings(stub_list);
	}

	return 0;
}