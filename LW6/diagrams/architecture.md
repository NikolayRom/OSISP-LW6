```mermaid
sequenceDiagram
    autonumber
    participant App as Приложение (main.cpp)
    participant Reg as Реестр (HKLM EventLog)
    participant Kernel as Windows API (Advapi32)
    participant Svc as Служба EventLog (svchost)
    participant LogFile as Файл Application.evtx
    participant Viewer as Event Viewer (eventvwr)

    Note over App, Reg: Опциональный этап: Регистрация источника (--register)
    App->>Reg: RegCreateKeyExW(HKLM\...\Application\OSiSP_Lab06_Source_095)
    App->>Reg: RegSetValueExW(TypesSupported=7, EventMessageFile)

    Note over App, Svc: Основной этап: Запись событий в журнал
    App->>Kernel: RegisterEventSourceW(NULL, "OSiSP_Lab06_Source_095")
    Kernel-->>App: Возврат дескриптора HANDLE hEventSource

    loop Запись событий (Info, Warning, Error)
        App->>Kernel: ReportEventW(hEventSource, Type, ID, 5 строк параметров)
        Kernel->>Svc: Межпроцессный вызов (LPC/RPC)
        Svc->>LogFile: Атомарная дозапись в Application.evtx
    end

    App->>Kernel: DeregisterEventSource(hEventSource)
    Kernel-->>App: Освобождение дескриптора

    Note over LogFile, Viewer: Просмотр администратором
    Viewer->>LogFile: Чтение и отображение во вкладке "Приложение"