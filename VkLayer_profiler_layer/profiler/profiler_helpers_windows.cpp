#ifdef WIN32
#include "profiler_helpers.h"

#if !defined _DEBUG && !defined NDEBUG
#define NDEBUG // for assert.h
#endif

#include <Windows.h>
#include <assert.h>

namespace Profiler
{
    /***********************************************************************************\

    Function:
        GetApplicationPath

    Description:
        Returns directory in which current exe is located.

    \***********************************************************************************/
    std::filesystem::path ProfilerPlatformFunctions::GetApplicationPath()
    {
        static std::filesystem::path applicationPath;

        if( applicationPath.empty() )
        {
            // Grab handle to the application module
            const HMODULE hCurrentModule = GetModuleHandleA( nullptr );

            std::string buffer;

            DWORD lastError = ERROR_INSUFFICIENT_BUFFER;
            while( lastError == ERROR_INSUFFICIENT_BUFFER )
            {
                // Increase size of the buffer a bit
                buffer.resize( buffer.size() + MAX_PATH );

                GetModuleFileNameA( hCurrentModule, buffer.data(), buffer.size() );

                // Update last error value
                lastError = GetLastError();
            }

            if( lastError == ERROR_SUCCESS )
            {
                applicationPath = buffer;
            }
            else
            {
                // Failed to get exe path
                applicationPath = "";
            }
        }

        return applicationPath;
    }

    /***********************************************************************************\

    Function:
        IsPreemptionEnabled

    Description:
        Checks if the scheduler allows preemption of DMA packets sent to the GPU.

    \***********************************************************************************/
    bool ProfilerPlatformFunctions::IsPreemptionEnabled()
    {
        // Read registry key
        DWORD enablePreemptionValue = 1;
        DWORD enablePreemptionValueSize = sizeof( DWORD );

        // Currently the only way to disable GPU DMA packet preemption is to set
        // HKLM\SYSTEM\CurrentControlSet\Control\GraphicsDriver\Scheduler\EnablePreemption
        // DWORD value to 0.
        RegGetValueA(
            HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\GraphicsDriver\\Scheduler",
            "EnablePreemption",
            RRF_RT_REG_DWORD,
            nullptr,
            &enablePreemptionValue,
            &enablePreemptionValueSize );

        return static_cast<bool>(enablePreemptionValue);
    }

    /***********************************************************************************\

    Function:
        WriteDebugUnformatted

    Description:
        Write string to debug output.

    \***********************************************************************************/
    void ProfilerPlatformFunctions::WriteDebugUnformatted( const char* str )
    {
        [[maybe_unused]]
        const size_t messageLength = std::strlen( str );
        // Output strings must end with newline
        assert( str[ messageLength - 1 ] == '\n' );

        OutputDebugStringA( str );
    }

}

#endif // WIN32