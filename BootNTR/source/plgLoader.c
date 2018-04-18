#include "main.h"
#include "plgLoader_pm_N3DS.h"
#include "plgLoader_pm_O3DS.h"
#include "plgLoader.h"


#define PM_PAYLOAD (isN3DS ? plgLoader_pm_N3DS_bin : plgLoader_pm_O3DS_bin)
#define PM_PAYLOAD_SIZE (isN3DS ? plgLoader_pm_N3DS_bin_size : plgLoader_pm_O3DS_bin_size)
#define PM_INJECT_ADDR (isN3DS ? 0x610BA00 : 0x610AC00)
#define PM_INJECT_SIZE 0x2000
#define PM_SVCRUN_ADDR (isN3DS ? 0x06103150 : 0x06103154)

bool isN3DS = false;

Result OpenProcessByName(const char *name, Handle *h)
{
	u32 pidList[0x40];
	s32 processCount;
	svcGetProcessList(&processCount, pidList, 0x40);
	Handle dstProcessHandle = 0;

	for (s32 i = 0; i < processCount; i++)
	{
		Handle processHandle;
		Result res = svcOpenProcess(&processHandle, pidList[i]);
		if (R_FAILED(res))
			continue;

		char procName[8] = { 0 };
		svcGetProcessInfo((s64 *)procName, processHandle, 0x10000);
		if (strncmp(procName, name, 8) == 0)
			dstProcessHandle = processHandle;
		else
			svcCloseHandle(processHandle);
	}

	if (dstProcessHandle == 0)
		return -1;

	*h = dstProcessHandle;
	return 0;
}

static u32 *translateAddress(u32 addr)
{
	if (addr < 0x1ff00000)
	{
		return (u32 *)(addr - 0x1f3f8000 + 0xfffdc000);
	}
	return (u32*)(addr - 0x1ff00000 + 0xdff00000);

}

void flash(u32 cl) ///< Hardcoded N3DS !!!
{
	u32 i;

	if (isN3DS)
	{
		for (i = 0; i < 64; i++) {
			*(vu32*)(0xFFFC4000 + 0x204) = cl;
			svcSleepThread(5000000);
		}
		*(vu32*)(0xFFFC4000 + 0x204) = 0;
	}
	else
	{
		for (i = 0; i < 64; i++) {
			*(vu32*)(0xfffc8000 + 0x204) = cl;
			svcSleepThread(5000000);
		}
		*(vu32*)(0xfffc8000 + 0x204) = 0;
	}
}

void set_kmmu_rw(int cpu, u32 addr, u32 size)
{
	int i, j;
	u32 mmu_p;
	u32 p1, p2;
	u32 v1, v2;
	u32 end;

	if (cpu == 0) {
		mmu_p = 0x1fff8000;
	}
	if (cpu == 1) {
		mmu_p = 0x1fffc000;
	}
	if (cpu == 2) {
		mmu_p = 0x1f3f8000;
	}

	end = addr + size;

	v1 = 0x20000000;

	for (i = 512; i<4096; i++)
	{
		p1 = *translateAddress(mmu_p + i * 4);
		if ((p1 & 3) == 2)
		{
			if (v1 >= addr && v1<end)
			{
				p1 &= 0xffff73ff;
				p1 |= 0x00000c00;
				*translateAddress(mmu_p + i * 4) = p1;
			}
		}
		else if ((p1 & 3) == 1) {
			p1 &= 0xfffffc00;
			for (j = 0; j<256; j++) {
				v2 = v1 + j * 0x1000;
				if ((v2 >= addr) && (v2<end)) {
					p2 = *translateAddress(p1 + j * 4);
					if ((p2 & 3) == 1) {
						p2 &= 0xffff7dcf;
						p2 |= 0x00000030;
						*translateAddress(p1 + j * 4) = p2;
					}
					else if ((p2 & 3)>1) {
						p2 &= 0xfffffdce;
						p2 |= 0x00000030;
						*translateAddress(p1 + j * 4) = p2;
					}
				}
			}

		}
		v1 += 0x00100000;
	}
}

s32 K_DoKernelHax() ///< This is hardcoded for N3DS for the test !
{
	__asm__ __volatile__("cpsid aif");

	u32 kmmuAddr = isN3DS ? 0xFFFBA000 : 0xFFFBE000;
	u32 kmmusize = 0x10000;
	// set mmu    
	for (int i = 0; i < 2 + isN3DS; i++)
		set_kmmu_rw(i, kmmuAddr, kmmusize);

	if (isN3DS)
	{
		*(u32*)0xDFF8862C = 0;
		*(u32*)0xDFF88630 = 0;
	}
	else
	{
		*(u32*)0xDFF88514 = 0;
		*(u32*)0xDFF88518 = 0;
	}

	return (0);
}

static void installPMPatch(void)
{
	static bool isInstalled = 0;

	if (isInstalled)
		return;

	svcBackdoor(K_DoKernelHax);
	svcFlushEntireDataCache();
	svcInvalidateEntireInstructionCache();

	u32   *dst = (u32 *)PM_INJECT_ADDR;
	u32   *payload = (u32 *)PM_PAYLOAD;

	for (u32 i = 0; i < PM_PAYLOAD_SIZE / 4; i++)
		dst[i] = payload[i];

	isInstalled = 1;
	return;
}

void    launchPluginLoader(void)
{
	APT_CheckNew3DS(&isN3DS);

	s64 startAddress, textTotalRoundedSize, rodataTotalRoundedSize, dataTotalRoundedSize;
	u32 totalSize;
	Handle processHandle = 0;
	Result res;

	if (osGetKernelVersion() < SYSTEM_VERSION(2, 54, 0))
		return;

	res = OpenProcessByName("pm", &processHandle);

	if (R_SUCCEEDED(res))
	{
		svcControlProcessMemory(processHandle, (PM_INJECT_ADDR - 0x6000000) & ~0xFFF, (PM_INJECT_ADDR - 0x6000000) & ~0xFFF, PM_INJECT_SIZE, 6, 7);
		svcGetProcessInfo(&textTotalRoundedSize, processHandle, 0x10002); // only patch .text + .data
		svcGetProcessInfo(&rodataTotalRoundedSize, processHandle, 0x10003);
		svcGetProcessInfo(&dataTotalRoundedSize, processHandle, 0x10004);

		totalSize = (u32)(textTotalRoundedSize + rodataTotalRoundedSize + dataTotalRoundedSize);

		svcGetProcessInfo(&startAddress, processHandle, 0x10005);
		res = svcMapProcessMemoryEx(processHandle, 0x06100000, (u32)startAddress, totalSize);

		u32   *code = (u32 *)PM_SVCRUN_ADDR; ///< addr >= 11.0, TODO: search for pattern

		if (R_SUCCEEDED(res))
		{
			
				installPMPatch();

				// Install hook
				code[0] = 0xE51FF004; // ldr pc, [pc, #-4]
				code[1] = PM_INJECT_ADDR - 0x6000000; // Payload
		}

		svcUnmapProcessMemoryEx(processHandle, 0x06100000, totalSize);
	}
	svcCloseHandle(processHandle);
}
