#include <atlbase.h>
#include <atlconv.h>         

#include <string>
#include <stdio.h>
#include <windows.h>
#include <Tlhelp32.h>

/**
typedef struct tagPROCESSENTRY32 {
    DWORD dwSize; ����������������������������������Ϣ�ṹ���С, �״ε���֮ǰ�����ʼ��
        DWORD cntUsage; ������������������������ ���ý��̵Ĵ���, ���ô���Ϊ0ʱ, ����̽���
        DWORD th32ProcessID; �����������������������̵�ID
        ULONG_PTR th32DefaultHeapID; ��������  ����Ĭ�϶ѵı�ʶ��, ������ʹ�ö�����û��
        DWORD th32ModuleID;                  ����ģ��ı�ʶ��
        DWORD cntThreads; ��������������������  ����������ִ���߳���
        DWORD th32ParentProcessID;           ������ID
        LONG  pcPriClassBase; ���������������� �����̵߳Ļ������ȼ�
        DWORD dwFlags; ������������������������ ����
        TCHAR szExeFile[MAX_PATH];          ���̵�·��
} PROCESSENTRY32;
typedef PROCESSENTRY32* PPROCESSENTRY32;
**/



BOOL GetProcessName(DWORD PID, PTSTR szProcessName, size_t cchSize)
{
    /* Opens an existing local process object. */
    HANDLE hProcess =
        OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
            FALSE,
            PID);

    if (hProcess == NULL)
    {
        return false;
    }

    DWORD dwSize = (DWORD)cchSize;
    /* Retrieves the full name of the executable image
     * for the specified process.
     */
    QueryFullProcessImageName(
        hProcess,
        0,
        szProcessName,
        &dwSize);

    /* Don't forget to close the process handle */
    CloseHandle(hProcess);

    return true;
}

static LPCWSTR stringToLPCWSTR(std::string orig)
{
    size_t origsize = orig.length() + 1;
    const size_t newsize = 100;
    size_t convertedChars = 0;
    wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (orig.length() - 1));
    mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);

    return wcstring;
}

static std::string WCharToMByte(LPCWSTR lpcwszStr)
{
    std::string str;
    DWORD dwMinSize = 0;
    LPSTR lpszStr = NULL;
    dwMinSize = WideCharToMultiByte(CP_OEMCP, NULL, lpcwszStr, -1, NULL, 0, NULL, FALSE);
    if (0 == dwMinSize)
    {
        return FALSE;
    }
    lpszStr = new char[dwMinSize];
    WideCharToMultiByte(CP_OEMCP, NULL, lpcwszStr, -1, lpszStr, dwMinSize, NULL, FALSE);
    str = lpszStr;
    delete[] lpszStr;
    return str;
}


BOOL KillProcess(DWORD dwPid)
{
    //ɱ������
    /*
    ɱ�����̷�ʽ�ܶ���
    1.TerminateProcess
    2.ѭ�������߳�,�����߳�
    3.�����ڴ�.�����ڴ�ɶ�����Ϊ ���ɷ���.�����쳣�Լ��Ƴ�
    4.����NT����ZwUnmapViewOfSection ȡ������ӳ��
    5.�ں� ���ڴ����㷨 ǿɱ����
    .... ˼·�ܶ�.���Ը�ĵط�Ҳ�ܶ�.
    */
    HANDLE hProcess = NULL;
    if (dwPid != 0)
    {
        //hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
        hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwPid);
        
        if (hProcess != NULL)
        {
            TerminateProcess(hProcess, 0);
        }

        if(hProcess != INVALID_HANDLE_VALUE)
            CloseHandle(hProcess);
    }
    return 0;
}


int main(int argc, char* argv[])
{
    

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot)
    {
        return 0;
    }

    PROCESSENTRY32 pi;
    pi.dwSize = sizeof(PROCESSENTRY32); //��һ��ʹ�ñ����ʼ����Ա
    BOOL bRet = Process32First(hSnapshot, &pi);
    WCHAR szProcessName[MAX_PATH] = {};

    WCHAR process1[MAX_PATH] = { L"notepad.exe" };
    _wcslwr_s(process1, wcslen(process1) + 1);

    while (bRet)
    {
        /*
        ѭ����������Լ��Ķ������
        */
        //USES_CONVERSION;
        //std::string str(W2A(pi.szExeFile));

        
        printf("����ID = %d ,����·�� = %ls\r\n", pi.th32ProcessID, pi.szExeFile);

        _wcslwr_s(pi.szExeFile, wcslen(pi.szExeFile) + 1);
        int result = _tcscmp(process1, pi.szExeFile);
        if (result == 0) {
            printf("����·�� = %ls,�ȽϽ�� = %d \r\n", pi.szExeFile, result);
            KillProcess(pi.th32ProcessID);
        }

        //GetProcessName(pi.th32ProcessID, szProcessName, MAX_PATH);
        //printf("����·�� = %s\r\n", szProcessName);

        bRet = Process32Next(hSnapshot, &pi);
    }

    CloseHandle(hSnapshot);

    return 0;
}


