#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <random>

const uint32_t STUDENT_ID = 95;
const uint32_t SEED = 1000 + STUDENT_ID * 97; const DWORD    BASE_EVENT_ID = 1000 + STUDENT_ID;      
const wchar_t* SOURCE_NAME = L"OSiSP_Lab06_Source_095";

std::wstring GetCurrentTimeString() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t buf[64];
    swprintf_s(buf, L"%04d-%02d-%02d %02d:%02d:%02d.%03d",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    return std::wstring(buf);
}

void PrintSystemError(const wchar_t* operation, DWORD errCode) {
    LPWSTR msg = nullptr;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&msg, 0, NULL
    );
    std::wcout << L"[ОШИБКА WinAPI] " << operation << L" | Код: " << errCode
        << L" | Текст: " << (msg ? msg : L"Неизвестная системная ошибка") << std::endl;
    if (msg) LocalFree(msg);
}

bool RegisterEventSourceInRegistry() {
    std::wstring keyPath = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" + std::wstring(SOURCE_NAME);
    HKEY hKey = NULL;
    DWORD disposition = 0;

    LSTATUS status = RegCreateKeyExW(
        HKEY_LOCAL_MACHINE,
        keyPath.c_str(),
        0, NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_SET_VALUE,
        NULL,
        &hKey,
        &disposition
    );

    if (status != ERROR_SUCCESS) {
        PrintSystemError(L"RegCreateKeyExW (Требуются права Администратора)", (DWORD)status);
        return false;
    }

        DWORD typesSupported = 7;
    RegSetValueExW(hKey, L"TypesSupported", 0, REG_DWORD, (const BYTE*)&typesSupported, sizeof(typesSupported));

        wchar_t sysDllPath[] = L"%SystemRoot%\\System32\\kernel32.dll";
    RegSetValueExW(hKey, L"EventMessageFile", 0, REG_EXPAND_SZ, (const BYTE*)sysDllPath, (DWORD)((wcslen(sysDllPath) + 1) * sizeof(wchar_t)));

    RegCloseKey(hKey);
    std::wcout << L"[РЕЕСТР] Источник '" << SOURCE_NAME << L"' успешно зарегистрирован в реестре Windows!\n";
    return true;
}

void UnregisterEventSourceFromRegistry() {
    std::wstring keyPath = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" + std::wstring(SOURCE_NAME);
    LSTATUS status = RegDeleteKeyW(HKEY_LOCAL_MACHINE, keyPath.c_str());
    if (status == ERROR_SUCCESS) {
        std::wcout << L"[РЕЕСТР] Источник '" << SOURCE_NAME << L"' успешно удален из реестра.\n";
    }
    else {
        PrintSystemError(L"RegDeleteKeyW", (DWORD)status);
    }
}

bool WriteAppEvent(HANDLE hEventSource, WORD eventType, DWORD eventId,
    const std::wstring& opType, const std::wstring& resultStr, const std::wstring& details) {
    if (!hEventSource) return false;

    std::wstring timeStr = GetCurrentTimeString();

            std::wstring idStr = std::to_wstring(eventId);
    std::wstring payloadStr = L"EventID: " + idStr +
        L" | Time: " + timeStr +
        L" | Op: " + opType +
        L" | Result: " + resultStr +
        L" | Details: " + details;

    LPCWSTR strings[5] = {
        idStr.c_str(),
        timeStr.c_str(),
        opType.c_str(),
        resultStr.c_str(),
        details.c_str()
    };

        BOOL success = ReportEventW(
        hEventSource,                       eventType,                          0,                                  eventId,                            NULL,                               5,                                  (DWORD)((payloadStr.length() + 1) * sizeof(wchar_t)),         strings,                            (LPVOID)payloadStr.c_str()      );

    if (!success) {
        PrintSystemError(L"ReportEventW", GetLastError());
        return false;
    }
    return true;
}

double RunExperimentBatch(HANDLE hEventSource, int count) {
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);

    std::mt19937 rng(SEED + count);
    std::uniform_int_distribution<int> typeDist(1, 3);

    QueryPerformanceCounter(&start);

    for (int i = 1; i <= count; ++i) {
        int rType = typeDist(rng);
        WORD evType = EVENTLOG_INFORMATION_TYPE;
        std::wstring opName = L"DATA_PROCESSING";
        std::wstring resName = L"SUCCESS";
        DWORD curId = BASE_EVENT_ID + (i % 10);

        if (rType == 2) {
            evType = EVENTLOG_WARNING_TYPE;
            opName = L"RESOURCE_CHECK";
            resName = L"WARN_THRESHOLD_EXCEEDED";
        }
        else if (rType == 3) {
            evType = EVENTLOG_ERROR_TYPE;
            opName = L"NETWORK_TRANSACTION";
            resName = L"ERR_IO_TIMEOUT";
        }

        std::wstring detail = L"Batch test event index #" + std::to_wstring(i);
        WriteAppEvent(hEventSource, evType, curId, opName, resName, detail);
    }

    QueryPerformanceCounter(&end);
    return (double)(end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
}

int main(int argc, char* argv[]) {
    _wsetlocale(LC_ALL, L"Russian");

        for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--register") {
            RegisterEventSourceInRegistry();
            return 0;
        }
        else if (arg == "--unregister") {
            UnregisterEventSourceFromRegistry();
            return 0;
        }
    }

    std::wcout << L"===================================================================\n";
    std::wcout << L"  Лабораторная работа №6 | Вариант 22 (Windows Event Log: Запись)  \n";
    std::wcout << L"  STUDENT_ID = " << STUDENT_ID << L" | BASE_EVENT_ID = " << BASE_EVENT_ID << L"\n";
    std::wcout << L"  Источник: " << SOURCE_NAME << L" | Журнал: Application\n";
    std::wcout << L"===================================================================\n\n";

        HANDLE hEventSource = RegisterEventSourceW(NULL, SOURCE_NAME);
    if (!hEventSource) {
        PrintSystemError(L"RegisterEventSourceW", GetLastError());
        system("pause");
        return 1;
    }
    std::wcout << L"[УСПЕХ] Дескриптор источника событий получен успешно.\n\n";

        std::wcout << L"--- Демонстрация записи базовых событий ---\n";

        if (WriteAppEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, BASE_EVENT_ID + 1,
        L"INIT_MODULE", L"STATUS_OK", L"Система успешно инициализирована")) {
        std::wcout << L"  [+] Записано событие INFORMATION (ID: " << BASE_EVENT_ID + 1 << L")\n";
    }

        if (WriteAppEvent(hEventSource, EVENTLOG_WARNING_TYPE, BASE_EVENT_ID + 2,
        L"BUFFER_MONITOR", L"STATUS_LIMIT_90%", L"Буфер заполнен на 90%")) {
        std::wcout << L"  [+] Записано событие WARNING     (ID: " << BASE_EVENT_ID + 2 << L")\n";
    }

        if (WriteAppEvent(hEventSource, EVENTLOG_ERROR_TYPE, BASE_EVENT_ID + 3,
        L"DISK_TRANSACTION", L"STATUS_WRITE_FAIL", L"Ошибка сброса кэша на диск")) {
        std::wcout << L"  [+] Записано событие ERROR       (ID: " << BASE_EVENT_ID + 3 << L")\n";
    }

    std::wcout << L"\n[ИНФОРМАЦИЯ] Демонстрационные события отправлены в журнал 'Application'.\n"
        << L"Их можно проверить в утилите: Win+R -> eventvwr.msc -> Журналы Windows -> Приложение.\n\n";

        std::wcout << L"===================================================================\n";
    std::wcout << L"  ЭКСПЕРИМЕНТАЛЬНАЯ ЧАСТЬ: Запись серий событий (по 3 прогона)     \n";
    std::wcout << L"===================================================================\n";

    std::vector<int> batches = { 10, 50, 100 };

    std::wcout << std::left << std::setw(22) << L"Серия событий"
        << std::setw(10) << L"Запуск 1"
        << std::setw(10) << L"Запуск 2"
        << std::setw(10) << L"Запуск 3"
        << std::setw(14) << L"Среднее (мс)"
        << std::setw(16) << L"Событий/сек" << L"\n";
    std::wcout << L"-----------------------------------------------------------------------------------\n";

    for (int count : batches) {
        std::vector<double> runs(3);
        for (int r = 0; r < 3; ++r) {
            runs[r] = RunExperimentBatch(hEventSource, count);
        }

        double avgMs = (runs[0] + runs[1] + runs[2]) / 3.0;
        double eps = (count / (avgMs / 1000.0)); 
        std::wcout << std::left << std::setw(22) << (L"Серия: " + std::to_wstring(count) + L" событий")
            << std::fixed << std::setprecision(2)
            << std::setw(10) << runs[0]
            << std::setw(10) << runs[1]
            << std::setw(10) << runs[2]
            << std::setw(14) << avgMs
            << std::setw(16) << (uint64_t)eps << L"\n";
    }

    std::wcout << L"===================================================================================\n";

        DeregisterEventSource(hEventSource);
    std::wcout << L"\n[ЗАВЕРШЕНИЕ] Ресурсы освобождены (DeregisterEventSource).\n";
    system("pause");

    return 0;
}