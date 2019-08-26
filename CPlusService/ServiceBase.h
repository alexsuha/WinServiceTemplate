#ifndef SERVICE_BASE_H_
#define SERVICE_BASE_H_

#include <windows.h>

namespace LustCore
{
	class ServiceBase
	{
	public:
		static bool Run(ServiceBase &service);

		ServiceBase(PWSTR pszServiceName,
			bool fCanStop = true,
			bool fCanShutdown = true,
			bool fCanPauseContinue = false);

		virtual ~ServiceBase(void);

		void Stop();

	protected:
		virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv);
		virtual void OnStop();
		virtual void OnPause();
		virtual void OnContinue();
		virtual void OnShutdown();
		void SetServiceStatus(DWORD dwCurrentState,
			DWORD dwWin32ExitCode = NO_ERROR,
			DWORD dwWaitHint = 0);
		void WriteEventLogEntry(PWSTR pszMessage, WORD wType);
		void WriteErrorLogEntry(PWSTR pszFunction,
			DWORD dwError = GetLastError());

	private:
		static void WINAPI ServiceMain(DWORD dwArgc, PWSTR *lpszArgv);
		static void WINAPI ServiceCtrlHandler(DWORD dwCtrl);
		
		void Start(DWORD dwArgc, PWSTR *pszArgv);
		void Pause();
		void Continue();
		void Shutdown();

		static ServiceBase *s_service;
		PWSTR m_name;
		SERVICE_STATUS m_status;
		SERVICE_STATUS_HANDLE m_statusHandle;
	};
}

#endif // SERVICE_BASE_H_
