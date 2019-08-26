#ifndef SAMPLE_SERVICE_H_
#define SAMPLE_SERVICE_H_

#include "ServiceBase.h"

namespace LustCore
{
	class SampleService : public ServiceBase
	{
	public:
		SampleService(PWSTR pszServiceName,
			bool fCanStop = true,
			bool fCanShutDown = true,
			bool fCanPauseCOntinue = false);

		virtual ~SampleService(void);

	protected:
		virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv);
		virtual void OnStop();

		void ServiceWorkerThread(void);

	private:
		bool m_fStopping;
		HANDLE m_hStoppedEvent;
	};
}

#endif // SAMPLE_SERVICE_H_
