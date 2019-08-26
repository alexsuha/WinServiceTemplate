#include "ServiceBase.h"
#include <assert.h>
#include <strsafe.h>

namespace LustCore
{
	ServiceBase *ServiceBase::s_service = nullptr;

	bool ServiceBase::Run(ServiceBase &service)
	{
		s_service = &service;

		SERVICE_TABLE_ENTRYW serviceTable[] = 
		{
			{ service.m_name, ServiceMain},
			{ nullptr, nullptr }
		};

		return StartServiceCtrlDispatcherW(serviceTable);
	}

	ServiceBase::ServiceBase(PWSTR pszServiceName, bool fCanStop /*= true*/, bool fCanShutdown /*= true*/, bool fCanPauseContinue /*= false*/)
	{
		// Service name must be a valid string and cannot be NULL.
		m_name = (pszServiceName == nullptr) ? const_cast<PWSTR>(L"") : pszServiceName;

		m_statusHandle = nullptr;

		// The service runs in its own process.
		m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

		// The service is starting.
		m_status.dwCurrentState = SERVICE_START_PENDING;

		// The accepted commands of the service.
		DWORD dwControlsAccepted = 0;
		if (fCanStop)
			dwControlsAccepted |= SERVICE_ACCEPT_STOP;
		if (fCanShutdown)
			dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
		if (fCanPauseContinue)
			dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
		m_status.dwControlsAccepted = dwControlsAccepted;

		m_status.dwWin32ExitCode = NO_ERROR;
		m_status.dwServiceSpecificExitCode = 0;
		m_status.dwCheckPoint = 0;
		m_status.dwWaitHint = 0;
	}

	ServiceBase::~ServiceBase(void)
	{
	}

	void ServiceBase::Stop()
	{
		DWORD dwOriginalState = m_status.dwCurrentState;
		try
		{
			// Tell SCM that the service is stopping.
			SetServiceStatus(SERVICE_STOP_PENDING);

			// Perform service-specific stop operations.
			OnStop();

			// Tell SCM that the service is stopped.
			SetServiceStatus(SERVICE_STOPPED);
		}
		catch (DWORD dwError)
		{
			// Log the error.
			WriteErrorLogEntry(const_cast<PWSTR>(L"Service Stop"), dwError);

			// Set the orginal service status.
			SetServiceStatus(dwOriginalState);
		}
		catch (...)
		{
			// Log the error.
			WriteEventLogEntry(const_cast<PWSTR>(L"Service failed to stop."), EVENTLOG_ERROR_TYPE);

			// Set the orginal service status.
			SetServiceStatus(dwOriginalState);
		}
	}

	void ServiceBase::OnStart(DWORD dwArgc, PWSTR *pszArgv)
	{

	}

	void ServiceBase::OnStop()
	{
	}

	void ServiceBase::OnPause()
	{
	}

	void ServiceBase::OnContinue()
	{
	}

	void ServiceBase::OnShutdown()
	{
	}

	void ServiceBase::SetServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
	{
		static DWORD dwCheckPoint = 1;

		// Fill in the SERVICE_STATUS structure of the service.

		m_status.dwCurrentState = dwCurrentState;
		m_status.dwWin32ExitCode = dwWin32ExitCode;
		m_status.dwWaitHint = dwWaitHint;

		m_status.dwCheckPoint =
			((dwCurrentState == SERVICE_RUNNING) ||
			(dwCurrentState == SERVICE_STOPPED)) ?
			0 : dwCheckPoint++;

		// Report the status of the service to the SCM.
		::SetServiceStatus(m_statusHandle, &m_status);
	}

	void ServiceBase::WriteEventLogEntry(PWSTR pszMessage, WORD wType)
	{
		HANDLE hEventSource = NULL;
		LPCWSTR lpszStrings[2] = { NULL, NULL };

		hEventSource = RegisterEventSourceW(NULL, m_name);
		if (hEventSource)
		{
			lpszStrings[0] = m_name;
			lpszStrings[1] = pszMessage;

			ReportEventW(hEventSource,  // Event log handle
				wType,                 // Event type
				0,                     // Event category
				0,                     // Event identifier
				NULL,                  // No security identifier
				2,                     // Size of lpszStrings array
				0,                     // No binary data
				lpszStrings,           // Array of strings
				NULL                   // No binary data
			);

			DeregisterEventSource(hEventSource);
		}
	}

	void ServiceBase::WriteErrorLogEntry(PWSTR pszFunction, DWORD dwError)
	{
		wchar_t szMessage[260];
		StringCchPrintfW(szMessage, ARRAYSIZE(szMessage),
			L"%s failed w/err 0x%08lx", pszFunction, dwError);
		WriteEventLogEntry(szMessage, EVENTLOG_ERROR_TYPE);
	}

	void WINAPI ServiceBase::ServiceMain(DWORD dwArgc, PWSTR * pszArgv)
	{
		assert(s_service != NULL);

		// Register the handler function for the service
		s_service->m_statusHandle = RegisterServiceCtrlHandlerW(
			s_service->m_name, ServiceCtrlHandler);
		if (s_service->m_statusHandle == NULL)
		{
			throw GetLastError();
		}

		// Start the service.
		s_service->Start(dwArgc, pszArgv);
	}

	void WINAPI ServiceBase::ServiceCtrlHandler(DWORD dwCtrl)
	{
		switch (dwCtrl)
		{
		case SERVICE_CONTROL_STOP: s_service->Stop(); break;
		case SERVICE_CONTROL_PAUSE: s_service->Pause(); break;
		case SERVICE_CONTROL_CONTINUE: s_service->Continue(); break;
		case SERVICE_CONTROL_SHUTDOWN: s_service->Shutdown(); break;
		case SERVICE_CONTROL_INTERROGATE: break;
		default: break;
		}
	}

	void ServiceBase::Start(DWORD dwArgc, PWSTR * pszArgv)
	{
		try
		{
			SetServiceStatus(SERVICE_START_PENDING);
			OnStart(dwArgc, pszArgv);
			SetServiceStatus(SERVICE_RUNNING);
		}
		catch (DWORD dwError)
		{
			WriteErrorLogEntry(const_cast<PWSTR>(L"Service Start"), dwError);
			SetServiceStatus(SERVICE_STOPPED, dwError);
		}
		catch (...)
		{
			WriteEventLogEntry(const_cast<PWSTR>(L"Service failed to start."), EVENTLOG_ERROR_TYPE);
			SetServiceStatus(SERVICE_STOPPED);
		}
	}

	void ServiceBase::Pause()
	{
		try
		{
			// Tell SCM that the service is pausing.
			SetServiceStatus(SERVICE_PAUSE_PENDING);

			// Perform service-specific pause operations.
			OnPause();

			// Tell SCM that the service is paused.
			SetServiceStatus(SERVICE_PAUSED);
		}
		catch (DWORD dwError)
		{
			// Log the error.
			WriteErrorLogEntry(const_cast<PWSTR>(L"Service Pause"), dwError);

			// Tell SCM that the service is still running.
			SetServiceStatus(SERVICE_RUNNING);
		}
		catch (...)
		{
			// Log the error.
			WriteEventLogEntry(const_cast<PWSTR>(L"Service failed to pause."), EVENTLOG_ERROR_TYPE);

			// Tell SCM that the service is still running.
			SetServiceStatus(SERVICE_RUNNING);
		}
	}

	void ServiceBase::Continue()
	{
		try
		{
			// Tell SCM that the service is resuming.
			SetServiceStatus(SERVICE_CONTINUE_PENDING);

			// Perform service-specific continue operations.
			OnContinue();

			// Tell SCM that the service is running.
			SetServiceStatus(SERVICE_RUNNING);
		}
		catch (DWORD dwError)
		{
			// Log the error.
			WriteErrorLogEntry(const_cast<PWSTR>(L"Service Continue"), dwError);

			// Tell SCM that the service is still paused.
			SetServiceStatus(SERVICE_PAUSED);
		}
		catch (...)
		{
			// Log the error.
			WriteEventLogEntry(const_cast<PWSTR>(L"Service failed to resume."), EVENTLOG_ERROR_TYPE);

			// Tell SCM that the service is still paused.
			SetServiceStatus(SERVICE_PAUSED);
		}
	}

	void ServiceBase::Shutdown()
	{
	}

}