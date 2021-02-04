#include <common.h>

struct S_Hook
{
	SceUID         id;
	tai_hook_ref_t ref;
};
typedef struct S_Hook Hook;

// List of hooks
#define HOOK_COUNT 14
static Hook hooks[HOOK_COUNT] = {0};

// Hook functions
static int hook_DisplaySetFrameBufInternalForDriver(int fb_id1, int fb_id2, const SceDisplayFrameBuf *pParam, int sync);

static int hook_PowerSetArmClockFrequency(int freq);
static int hook_PowerSetBusClockFrequency(int freq);
static int hook_PowerSetGpuClockFrequency(int freq);
static int hook_PowerSetGpuXbarClockFrequency(int freq);

static int hook_CtrlPeekBufferPositive(int port, SceCtrlData *ctrl, int count);
static int hook_CtrlReadBufferPositive(int port, SceCtrlData *ctrl, int count);
static int hook_CtrlPeekBufferPositive2(int port, SceCtrlData *ctrl, int count);
static int hook_CtrlReadBufferPositive2(int port, SceCtrlData *ctrl, int count);
static int hook_CtrlPeekBufferPositiveExt(int port, SceCtrlData *ctrl, int count);
static int hook_CtrlReadBufferPositiveExt(int port, SceCtrlData *ctrl, int count);
static int hook_CtrlPeekBufferPositiveExt2(int port, SceCtrlData *ctrl, int count);
static int hook_CtrlReadBufferPositiveExt2(int port, SceCtrlData *ctrl, int count);

// Install kernel hooks
void hooks_install(void)
{
	tai_module_info_t SceCtrl_info;

	// Hook "sceDisplaySetFrameBufInternalForDriver"
	hooks[0].id = taiHookFunctionExportForKernel(KERNEL_PID, &hooks[0].ref, "SceDisplay",0x9FED47AC,0x16466675, hook_DisplaySetFrameBufInternalForDriver);

	// Hook "scePowerSetArmClockFrequency"
	hooks[1].id = taiHookFunctionExportForKernel(KERNEL_PID, &hooks[1].ref, "ScePower", 0x1590166F, 0x74DB5AE5, hook_PowerSetArmClockFrequency);
	// Hook "scePowerSetBusClockFrequency"
	hooks[2].id = taiHookFunctionExportForKernel(KERNEL_PID, &hooks[2].ref, "ScePower", 0x1590166F, 0xB8D7B3FB, hook_PowerSetBusClockFrequency);
	// Hook "scePowerSetGpuClockFrequency"
	hooks[3].id = taiHookFunctionExportForKernel(KERNEL_PID, &hooks[3].ref, "ScePower", 0x1590166F, 0x264C24FC, hook_PowerSetGpuClockFrequency);
	// Hook "scePowerSetGpuXbarClockFrequency"
	hooks[4].id = taiHookFunctionExportForKernel(KERNEL_PID, &hooks[4].ref, "ScePower", 0x1590166F, 0xA7739DBE, hook_PowerSetGpuXbarClockFrequency);

	taiGetModuleInfoForKernel(KERNEL_PID, "SceCtrl", &SceCtrl_info);
	// Hook "sceCtrlPeekBufferPositive"
	hooks[5].id = taiHookFunctionExportForKernel(KERNEL_PID, &hooks[5].ref, "SceCtrl", TAI_ANY_LIBRARY, 0xEA1D3A34, hook_CtrlPeekBufferPositive);
	// Hook "sceCtrlReadBufferPositive"
	hooks[6].id = taiHookFunctionExportForKernel(KERNEL_PID, &hooks[6].ref, "SceCtrl", TAI_ANY_LIBRARY, 0x9B96A1AA, hook_CtrlReadBufferPositive);
	// Hook "sceCtrlPeekBufferPositive2"
	hooks[7].id = taiHookFunctionOffsetForKernel(KERNEL_PID, &hooks[7].ref, SceCtrl_info.modid, 0, 0x3EF8, 1, hook_CtrlPeekBufferPositive2);
	// Hook "sceCtrlReadBufferPositive2"
	hooks[8].id = taiHookFunctionOffsetForKernel(KERNEL_PID, &hooks[8].ref, SceCtrl_info.modid, 0, 0x449C, 1, hook_CtrlReadBufferPositive2);
	// Hook "sceCtrlPeekBufferPositiveExt"
	hooks[9].id = taiHookFunctionOffsetForKernel(KERNEL_PID, &hooks[9].ref, SceCtrl_info.modid, 0, 0x3928, 1, hook_CtrlPeekBufferPositiveExt);
	// Hook "sceCtrlReadBufferPositiveExt"
	hooks[10].id = taiHookFunctionOffsetForKernel(KERNEL_PID, &hooks[10].ref, SceCtrl_info.modid, 0, 0x3BCC, 1, hook_CtrlReadBufferPositiveExt);
	// Hook "sceCtrlPeekBufferPositiveExt2"
	hooks[11].id = taiHookFunctionOffsetForKernel(KERNEL_PID, &hooks[11].ref, SceCtrl_info.modid, 0, 0x4B48, 1, hook_CtrlPeekBufferPositiveExt2);
	// Hook "sceCtrlReadBufferPositiveExt2"
	hooks[12].id = taiHookFunctionOffsetForKernel(KERNEL_PID, &hooks[12].ref, SceCtrl_info.modid, 0, 0x4E14, 1, hook_CtrlReadBufferPositiveExt2);
}

// Uninstall kernel hooks
void hooks_uninstall(void)
{
	// Release hooks
	for (size_t i = 0; i < HOOK_COUNT; ++ i)
		if (hooks[i].id > 0)
			taiHookReleaseForKernel(hooks[i].id, hooks[i].ref);
}

static int hook_DisplaySetFrameBufInternalForDriver(int fb_id1, int fb_id2, const SceDisplayFrameBuf *pParam, int sync)
{
	int result = 1;

	if (fb_id1 && pParam)
	{
		// Check focused process
		SceUID pid = ksceKernelGetProcessId();
		if (pid != focus_pid)
			focus_changed(pid);

		// Lock display mutex
		if (kmutex_try_lock(&display.mutex))
		{
			// Set framebuffer
			display_set_framebuffer(pParam);

			// Draw menu
			if (menu.visible)
				menu.draw_func();

			// Unlock mutex
			kmutex_unlock(&display.mutex);
		}
	}

	if (hooks[0].ref)
		result = TAI_CONTINUE(int, hooks[0].ref, fb_id1, fb_id2, pParam, sync);
	return result;
}

static int hook_PowerSetArmClockFrequency(int freq)
{
	int result = 1;
	if (hooks[1].ref)
		result = TAI_CONTINUE(int, hooks[1].ref, freq);
	return result;
}

static int hook_PowerSetBusClockFrequency(int freq)
{
	int result = 1;
	if (hooks[2].ref)
		result = TAI_CONTINUE(int, hooks[2].ref, freq);
	return result;
}

static int hook_PowerSetGpuClockFrequency(int freq)
{
	int result = 1;
	if (hooks[3].ref)
		result = TAI_CONTINUE(int, hooks[3].ref, freq);
	return result;
}

static int hook_PowerSetGpuXbarClockFrequency(int freq)
{
	int result = 1;
	if (hooks[4].ref)
		result = TAI_CONTINUE(int, hooks[4].ref, freq);
	return result;
}

static int hook_CtrlPeekBufferPositive(int port, SceCtrlData *ctrl, int count)
{
	int result = 1;
	if (hooks[5].ref)
		result = TAI_CONTINUE(int, hooks[5].ref, port, ctrl, count);

	if (result > 0)
	{
		input_handle(port, ctrl);
		input_filter(port, ctrl);
	}

	return result;
}

static int hook_CtrlReadBufferPositive(int port, SceCtrlData *ctrl, int count)
{
	int result = 1;
	if (hooks[6].ref)
		result = TAI_CONTINUE(int, hooks[6].ref, port, ctrl, count);

	if (result > 0)
	{
		input_handle(port, ctrl);
		input_filter(port, ctrl);
	}

	return result;
}

static int hook_CtrlPeekBufferPositive2(int port, SceCtrlData *ctrl, int count)
{
	int result = 1;
	if (hooks[7].ref)
		result = TAI_CONTINUE(int, hooks[7].ref, port, ctrl, count);

	if (result > 0)
	{
		input_handle(port, ctrl);
		input_filter(port, ctrl);
	}

	return result;
}

static int hook_CtrlReadBufferPositive2(int port, SceCtrlData *ctrl, int count)
{
	int result = 1;
	if (hooks[8].ref)
		result = TAI_CONTINUE(int, hooks[8].ref, port, ctrl, count);

	if (result > 0)
	{
		input_handle(port, ctrl);
		input_filter(port, ctrl);
	}

	return result;
}

static int hook_CtrlPeekBufferPositiveExt(int port, SceCtrlData *ctrl, int count)
{
	int result = 1;
	if (hooks[9].ref)
		result = TAI_CONTINUE(int, hooks[9].ref, port, ctrl, count);

	if (result > 0)
	{
		input_handle(port, ctrl);
		input_filter(port, ctrl);
	}

	return result;
}

static int hook_CtrlReadBufferPositiveExt(int port, SceCtrlData *ctrl, int count)
{
	int result = 1;
	if (hooks[10].ref)
		result = TAI_CONTINUE(int, hooks[10].ref, port, ctrl, count);

	if (result > 0)
	{
		input_handle(port, ctrl);
		input_filter(port, ctrl);
	}

	return result;
}

static int hook_CtrlPeekBufferPositiveExt2(int port, SceCtrlData *ctrl, int count)
{
	int result = 1;
	if (hooks[11].ref)
		result = TAI_CONTINUE(int, hooks[11].ref, port, ctrl, count);

	if (result > 0)
	{
		input_handle(port, ctrl);
		input_filter(port, ctrl);
	}

	return result;
}

static int hook_CtrlReadBufferPositiveExt2(int port, SceCtrlData *ctrl, int count)
{
	int result = 1;
	if (hooks[12].ref)
		result = TAI_CONTINUE(int, hooks[12].ref, port, ctrl, count);

	if (result > 0)
	{
		input_handle(port, ctrl);
		input_filter(port, ctrl);
	}

	return result;
}
