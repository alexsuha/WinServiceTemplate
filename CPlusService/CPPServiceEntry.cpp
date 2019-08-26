#include <stdio.h>
#include <windows.h>
#include "ServiceInstaller.h"
#include "ServiceBase.h"
#include "SampleService.h"

// Internal name of the service
#define SERVICE_NAME             L"CppWindowsService"
// Displayed name of the service
#define SERVICE_DISPLAY_NAME     L"CppWindowsService Sample Service"
// Service start options.
#define SERVICE_START_TYPE       SERVICE_DEMAND_START
// List of service dependencies - "dep1\0dep2\0\0"
#define SERVICE_DEPENDENCIES     L""
// The name of the account under which the service should run
#define SERVICE_ACCOUNT          L"NT AUTHORITY\\LocalService"
// The password to the service account name
#define SERVICE_PASSWORD         NULL


int wmain(int argc, wchar_t *argv[])
{
	wprintf(L"GetModuleFileName failed");

	if ((argc > 1) && ((*argv[1] == L'-' || (*argv[1] == L'/'))))
	{
		if (_wcsicmp(L"install", argv[1] + 1) == 0)
		{
			LustCore::InstallService(
				const_cast<PWSTR>(SERVICE_NAME),               // Name of service
				const_cast<PWSTR>(SERVICE_DISPLAY_NAME),       // Name to display
				SERVICE_START_TYPE,         // Service start type
				const_cast<PWSTR>(SERVICE_DEPENDENCIES),       // Dependencies
				const_cast<PWSTR>(SERVICE_ACCOUNT),            // Service running account
				SERVICE_PASSWORD            // Password of the account
				);
		}
		else if (_wcsicmp(L"remove", argv[1] + 1) == 0)
		{
			// Uninstall the service when the command is 
			// "-remove" or "/remove".
			LustCore::UninstallService(const_cast<PWSTR>(SERVICE_NAME));
		}
	}
	else
	{
		wprintf(L"Parameters:\n");
		wprintf(L" -install  to install the service.\n");
		wprintf(L" -remove   to remove the service.\n");

		LustCore::SampleService service(const_cast<PWSTR>(SERVICE_NAME));
		if (!LustCore::ServiceBase::Run(service))
		{
			wprintf(L"Service failed to run w/err 0x%08lx\n", GetLastError());
		}
	}

	wprintf(L"Service run finished");
	return 0;
}