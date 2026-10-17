/*
 * PROJECT:     ReactOS Win32 Base API (ClyFiOS)
 * LICENSE:     GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:     Windows Error Reporting registration functions
 *
 * ReactOS has no Windows Error Reporting service. These functions used to be
 * unimplemented stubs, which raise EXCEPTION_WINE_STUB when called. Some
 * programs call them unconditionally during startup (for example Supermium's
 * kernel32 polyfill calls WerSetFlags from its DllMain), so the stub made the
 * whole program fail to start with STATUS_DLL_INIT_FAILED (0xC0000142).
 *
 * Without a WER service, accepting the registrations and doing nothing is the
 * correct behavior: nothing will ever collect a report anyway.
 */

#include <k32_vista.h>
#include <werapi.h>

#define NDEBUG
#include <debug.h>

/* Flags set with WerSetFlags, so WerGetFlags can return them */
static DWORD BasepWerFlags = 0;

HRESULT
WINAPI
WerSetFlags(
    _In_ DWORD dwFlags)
{
    BasepWerFlags = dwFlags;
    return S_OK;
}

HRESULT
WINAPI
WerGetFlags(
    _In_ HANDLE hProcess,
    _Out_ PDWORD pdwFlags)
{
    if (!pdwFlags)
        return E_INVALIDARG;

    /* We only track the flags of the current process */
    *pdwFlags = (hProcess == NtCurrentProcess() ||
                 GetProcessId(hProcess) == GetCurrentProcessId()) ? BasepWerFlags : 0;
    return S_OK;
}

HRESULT
WINAPI
WerRegisterFile(
    _In_ PCWSTR pwzFile,
    _In_ WER_REGISTER_FILE_TYPE regFileType,
    _In_ DWORD dwFlags)
{
    UNREFERENCED_PARAMETER(regFileType);
    UNREFERENCED_PARAMETER(dwFlags);

    if (!pwzFile)
        return E_INVALIDARG;
    return S_OK;
}

HRESULT
WINAPI
WerUnregisterFile(
    _In_ PCWSTR pwzFilePath)
{
    if (!pwzFilePath)
        return E_INVALIDARG;
    return S_OK;
}

HRESULT
WINAPI
WerRegisterMemoryBlock(
    _In_ PVOID pvAddress,
    _In_ DWORD dwSize)
{
    if (!pvAddress || !dwSize)
        return E_INVALIDARG;
    return S_OK;
}

HRESULT
WINAPI
WerUnregisterMemoryBlock(
    _In_ PVOID pvAddress)
{
    if (!pvAddress)
        return E_INVALIDARG;
    return S_OK;
}
