/*
 * PROJECT:     ReactOS Win32 Base API (ClyFiOS)
 * LICENSE:     GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:     Process/thread attribute lists (InitializeProcThreadAttributeList & co.)
 *
 * These used to be "-stub" exports that just returned FALSE. Chromium-based
 * programs (e.g. the Supermium browser) build an attribute list to start
 * their crash handler process and treat a failure as fatal: chrome.exe hit
 * an immediate-crash CHECK (int3; ud2) right after
 * InitializeProcThreadAttributeList failed.
 *
 * The list itself is implemented fully (storage, size negotiation,
 * duplicate detection). CreateProcess does not act on the attributes yet;
 * EXTENDED_STARTUPINFO_PRESENT is accepted and the list is ignored, which
 * means e.g. PROC_THREAD_ATTRIBUTE_HANDLE_LIST falls back to normal handle
 * inheritance.
 */

#include <k32_vista.h>

#define NDEBUG
#include <debug.h>

typedef struct _PROC_THREAD_ATTRIBUTE_ENTRY
{
    DWORD_PTR Attribute;
    SIZE_T Size;
    PVOID Value;
} PROC_THREAD_ATTRIBUTE_ENTRY, *PPROC_THREAD_ATTRIBUTE_ENTRY;

/* Layout compatible with Windows/Wine (the struct is opaque to callers) */
typedef struct _PROC_THREAD_ATTRIBUTE_LIST
{
    DWORD PresentFlags;   /* bit N set = attribute number N already present */
    DWORD Size;           /* capacity, in entries */
    DWORD Count;          /* used entries */
    DWORD Reserved;
    PVOID Unknown;
    PROC_THREAD_ATTRIBUTE_ENTRY Entries[ANYSIZE_ARRAY];
} PROC_THREAD_ATTRIBUTE_LIST;

BOOL
WINAPI
InitializeProcThreadAttributeList(
    _Out_opt_ LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
    _In_ DWORD dwAttributeCount,
    _Reserved_ DWORD dwFlags,
    _Inout_ PSIZE_T lpSize)
{
    SIZE_T Needed;

    if (dwFlags != 0 || !lpSize)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    Needed = FIELD_OFFSET(PROC_THREAD_ATTRIBUTE_LIST, Entries) +
             (SIZE_T)dwAttributeCount * sizeof(PROC_THREAD_ATTRIBUTE_ENTRY);

    if (!lpAttributeList || *lpSize < Needed)
    {
        *lpSize = Needed;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    lpAttributeList->PresentFlags = 0;
    lpAttributeList->Size = dwAttributeCount;
    lpAttributeList->Count = 0;
    lpAttributeList->Reserved = 0;
    lpAttributeList->Unknown = NULL;
    *lpSize = Needed;
    return TRUE;
}

BOOL
WINAPI
UpdateProcThreadAttribute(
    _Inout_ LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
    _In_ DWORD dwFlags,
    _In_ DWORD_PTR Attribute,
    _In_reads_bytes_opt_(cbSize) PVOID lpValue,
    _In_ SIZE_T cbSize,
    _Out_writes_bytes_opt_(cbSize) PVOID lpPreviousValue,
    _In_opt_ PSIZE_T lpReturnSize)
{
    DWORD Number;
    PPROC_THREAD_ATTRIBUTE_ENTRY Entry;

    UNREFERENCED_PARAMETER(lpPreviousValue);
    UNREFERENCED_PARAMETER(lpReturnSize);

    if (!lpAttributeList || dwFlags != 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (lpAttributeList->Count >= lpAttributeList->Size)
    {
        SetLastError(ERROR_GEN_FAILURE);
        return FALSE;
    }

    /* Basic size validation for the common attributes */
    switch (Attribute)
    {
        case PROC_THREAD_ATTRIBUTE_PARENT_PROCESS:
            if (cbSize != sizeof(HANDLE))
            {
                SetLastError(ERROR_BAD_LENGTH);
                return FALSE;
            }
            break;

        case PROC_THREAD_ATTRIBUTE_HANDLE_LIST:
            if (cbSize == 0 || (cbSize % sizeof(HANDLE)) != 0)
            {
                SetLastError(ERROR_BAD_LENGTH);
                return FALSE;
            }
            break;

        default:
            /* Accept other attributes as-is; they are not acted upon yet */
            break;
    }

    Number = (DWORD)(Attribute & PROC_THREAD_ATTRIBUTE_NUMBER);
    if (Number < 32)
    {
        if (lpAttributeList->PresentFlags & (1UL << Number))
        {
            SetLastError(ERROR_OBJECT_NAME_EXISTS);
            return FALSE;
        }
        lpAttributeList->PresentFlags |= (1UL << Number);
    }

    Entry = &lpAttributeList->Entries[lpAttributeList->Count++];
    Entry->Attribute = Attribute;
    Entry->Size = cbSize;
    Entry->Value = lpValue;
    return TRUE;
}

VOID
WINAPI
DeleteProcThreadAttributeList(
    _Inout_ LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList)
{
    /* The caller owns the memory; nothing to free */
    UNREFERENCED_PARAMETER(lpAttributeList);
}
