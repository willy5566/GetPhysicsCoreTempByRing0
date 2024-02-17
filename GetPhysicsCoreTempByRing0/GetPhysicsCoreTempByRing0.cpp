// GetPhysicsCoreTempByRing0.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <iostream>
#include <Windows.h>
#include "include/OlsApi.h"

DWORD GetPhysicsCoreId(PDWORD coreids) {
    DWORD size = 0;
    DWORD count = 0;
    PBYTE processorInfos = NULL;
    if (GetLogicalProcessorInformationEx(RelationProcessorCore, (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)processorInfos, &size) == FALSE) {
        processorInfos = (PBYTE)malloc(size);
        if (GetLogicalProcessorInformationEx(RelationProcessorCore, (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)processorInfos, &size) == FALSE) {
            printf("GetLogicalProcessorInformationEx Error: %d\n", GetLastError());
        }
        DWORD start = 0;
        while (start < size)
        {
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX processorInfo = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)(processorInfos + start);
            for (size_t i = 0; i < 64; i++)
            {
                DWORD firstMask = (processorInfo->Processor.GroupMask->Mask >> i) & 0x1;
                if (firstMask > 0) {
                    coreids[count] = i;
                    count++;
                    break;
                }
            }

            start += processorInfo->Size;
        }
        free(processorInfos);
    }
    return count;
}

int main()
{
    // 取得邏輯處理器數量
    SYSTEM_INFO sysinfo{};
    GetSystemInfo(&sysinfo);
    DWORD numberOfProcessors = sysinfo.dwNumberOfProcessors;

    // 取得物理處理器的AffinityMask與數量
    PDWORD masks = (PDWORD)malloc(sizeof(DWORD) * numberOfProcessors);
    DWORD count = GetPhysicsCoreId(masks);

    // 使用Ring0取得所有物理處理器的溫度
    if (InitializeOls() != TRUE) {
        std::cout << "DLL Load Error!\n";
        getchar();
        return false;
    }

    DWORD eax, edx;
    DWORD TjMax;
    DWORD IAcore;
    DWORD PKGsts;
    int Cputemp;
    for (size_t i = 0; i < count; i++)
    {
        DWORD_PTR affinityMask = 0x01 << masks[i];

        RdmsrTx(0x1A2, &eax, &edx, affinityMask);
        TjMax = eax;
        TjMax &= 0xFF0000;
        TjMax = TjMax >> 16;

        RdmsrTx(0x19C, &eax, &edx, affinityMask);
        IAcore = eax;
        IAcore &= 0xFF0000;
        IAcore = IAcore >> 16;

        Cputemp = (int)(TjMax - IAcore);
        std::cout << "Cputemp :" << Cputemp << "\n";
    }

    free(masks);

    DeinitializeOls();

    getchar();
}

// 執行程式: Ctrl + F5 或 [偵錯] > [啟動但不偵錯] 功能表
// 偵錯程式: F5 或 [偵錯] > [啟動偵錯] 功能表

// 開始使用的提示: 
//   1. 使用 [方案總管] 視窗，新增/管理檔案
//   2. 使用 [Team Explorer] 視窗，連線到原始檔控制
//   3. 使用 [輸出] 視窗，參閱組建輸出與其他訊息
//   4. 使用 [錯誤清單] 視窗，檢視錯誤
//   5. 前往 [專案] > [新增項目]，建立新的程式碼檔案，或是前往 [專案] > [新增現有項目]，將現有程式碼檔案新增至專案
//   6. 之後要再次開啟此專案時，請前往 [檔案] > [開啟] > [專案]，然後選取 .sln 檔案
