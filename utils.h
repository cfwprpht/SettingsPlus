/*
	Settings+ Flow Plugin
	Copyright (C) 2015-2016, TheFloW

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

#ifndef __UTILS_H__
#define __UTILS_H__

typedef struct {
	uint16_t size;
	uint16_t lib_version;
	uint16_t attribute;
	uint16_t num_functions;
	uint16_t num_vars;
	uint16_t num_tls_vars;
	uint32_t reserved1;
	uint32_t module_nid;
	char *lib_name;
	uint32_t reserved2;
	uint32_t *func_nid_table;
	void **func_entry_table;
	uint32_t *var_nid_table;
	void **var_entry_table;
	uint32_t *tls_nid_table;
	void **tls_entry_table;
} SceImportsTable2xx;

typedef struct {
	uint16_t size;
	uint16_t lib_version;
	uint16_t attribute;
	uint16_t num_functions;
	uint16_t num_vars;
	uint16_t unknown1;
	uint32_t module_nid;
	char *lib_name;
	uint32_t *func_nid_table;
	void **func_entry_table;
	uint32_t *var_nid_table;
	void **var_entry_table;
} SceImportsTable3xx;

int debugPrintf(char *text, ...);

int ReadFile(char *file, void *buf, int size);
int WriteFile(char *file, void *buf, int size);

uint32_t findModuleImportNumber(SceModuleInfo *mod_info, uint32_t text_addr, char *libname, uint32_t nid);

uint32_t countImports(SceModuleInfo *mod_info, uint32_t text_addr);

SceModuleInfo *findModuleInfo(char *modname, uint32_t text_addr, uint32_t text_size);

#endif