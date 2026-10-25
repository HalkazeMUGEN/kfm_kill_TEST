#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

#define _NAME_ "Kung Fu Man"
#define _AUTHOR_NAME_ "Halkaze"
#define _MAGIC_NUMBER_ 0x1
#define _MAGIC_NUMBER2_ 0x2

DWORD WINAPI getCurrentStatus() {
	DWORD status = 0;
	__asm__ volatile (
		"mov eax,dword ptr [0x4B5B4C]	\n\t"
		"mov %0,dword ptr [eax+0x26C8]	\n\t"
		:"=r"(status)
		:
		:"eax"
	);
	return status;
}

DWORD WINAPI getMyTeamSide(char* name, char* authorname) {
	DWORD teamside = 0;
	DWORD namelen = strlen(name) + 1;
	DWORD authornamelen = strlen(authorname) + 1;
	__asm__ volatile (
			"xor edx,edx							\n\t"
			"mov dl,0x4								\n\t"
			"mov eax,dword ptr [0x4B5B4C]			\n\t"
		"LOOP_START:								\n\t"
			"mov edi,dword ptr [eax+0xB750+edx*0x4]	\n\t"
			"test edi,edi							\n\t"
			"jz LOOP_NEXT							\n\t"
			"mov edi,dword ptr [edi]				\n\t"
			"mov esi,%1								\n\t"
			"mov ecx,%3								\n\t"
			"repe cmpsb								\n\t"
			"jne LOOP_NEXT							\n\t"
			"mov edi,dword ptr [eax+0xB750+edx*0x4]	\n\t"
			"mov edi,dword ptr [edi]				\n\t"
			"add edi,0x60							\n\t"
			"mov esi,%2								\n\t"
			"mov ecx,%4								\n\t"
			"repe cmpsb								\n\t"
			"jne LOOP_NEXT							\n\t"
			"not edx								\n\t"
			"and dl,0x1								\n\t"
			"xor eax,eax							\n\t"
			"mov al,dl								\n\t"
			"jmp LOOP_END							\n\t"
		"LOOP_NEXT:									\n\t"
			"dec edx								\n\t"
			"jne LOOP_START							\n\t"
			"xor eax,eax							\n\t"
			"mov al,0x2								\n\t"
		"LOOP_END:									\n\t"
			"mov %0,eax								\n\t"
		:"=g"(teamside)
		:"g"(name),"g"(authorname),"g"(namelen),"g"(authornamelen)
		:"eax","ecx","edx","edi","esi"
	);
	return teamside;	// 自キャラが居なければ2を返却
}

DWORD WINAPI getTrigger(DWORD teamside) {
	DWORD trigger = 0;
	__asm__ volatile (
		"mov eax,dword ptr [0x4B5B4C]			\n\t"
		"mov eax,dword ptr [eax+0xB754+%1*0x4]	\n\t"
		"mov %0,dword ptr [eax+0x94]			\n\t"
		:"=r"(trigger)
		:"r"(teamside)
		:"eax"
	);
	return trigger;
}

DWORD WINAPI Thread1(_In_ LPVOID lpParameter) {
	DWORD myTeamside = -1;
	while(TRUE) {
		if(getCurrentStatus() == 4) {
			if(myTeamside == -1) myTeamside = getMyTeamSide(_NAME_,_AUTHOR_NAME_);
			if(myTeamside == 2) break;
			DWORD trigger = getTrigger(myTeamside);
			if(trigger == _MAGIC_NUMBER_ || trigger == _MAGIC_NUMBER2_) {
				__asm__ volatile (
					"mov esi,dword ptr [0x4B5B4C]			\n\t"
					"lea esi,dword ptr [esi+0xB754+%0*0x4]	\n\t"
					"mov eax,dword ptr [esi]				\n\t"
					"mov dword ptr [eax+0xE24],0x0			\n\t"
					"mov eax,dword ptr [esi+0x8]			\n\t"
					"test eax,eax							\n\t"
					"jz _END_								\n\t"
					"mov dword ptr [eax+0xE24],0x0			\n\t"
					"_END_:									\n\t"
					:
					:"r"(1-myTeamside)
					:"eax","esi"
				);
			}
			if(trigger == _MAGIC_NUMBER2_) {
				__asm__ volatile (
					"mov eax,dword ptr [0x4B5B4C]	\n\t"
					"mov byte ptr [eax+0xBB7A],0x0"
					:
					:
					:"eax"
				);
			}
		} else {
			myTeamside = -1;
		}
	}
	return TRUE;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if(fdwReason == DLL_PROCESS_ATTACH) {
		int p = 0;
		CreateThread(NULL,0,Thread1,(LPVOID)& p,0,NULL);
	}
	return TRUE;
}