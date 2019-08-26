#include "SampleService.h"
#include "ThreadPool.h"

namespace LustCore
{

	SampleService::SampleService(PWSTR pszServiceName, bool fCanStop /*= true*/, bool fCanShutDown /*= true*/, bool fCanPauseCOntinue /*= false*/)
		: ServiceBase(pszServiceName, fCanStop, fCanShutDown, fCanPauseCOntinue)
	{
		m_fStopping = FALSE;

		m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (m_hStoppedEvent == NULL)
		{
			throw GetLastError();
		}
	}

	SampleService::~SampleService(void)
	{
		if (m_hStoppedEvent)
		{
			CloseHandle(m_hStoppedEvent);
			m_hStoppedEvent = NULL;
		}
	}

	void SampleService::OnStart(DWORD dwArgc, PWSTR *pszArgv)
	{
		WriteEventLogEntry(const_cast<PWSTR>(L"CppWindowsService in OnStart"),
			EVENTLOG_INFORMATION_TYPE);

		ThreadPool::QueueUserWorkItem(&SampleService::ServiceWorkerThread, this);
	}

	void SampleService::OnStop()
	{
		// Periodically check if the service is stopping.
		while (!m_fStopping)
		{
			// Perform main service function here...
			::Sleep(2000);  // Simulate some lengthy operations.
		}
		// Signal the stopped event.
		SetEvent(m_hStoppedEvent);
	}

	void SampleService::ServiceWorkerThread(void)
	{
		WriteEventLogEntry(const_cast<PWSTR>(L"CppWindowsService in OnStop"),
			EVENTLOG_INFORMATION_TYPE);
		m_fStopping = TRUE;
		if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
		{
			throw GetLastError();
		}
	}

}