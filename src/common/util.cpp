// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

#include "util.h"

#include <cstdio>
#include <cstring>

#include <common/file_system_shim.h>

#include <config/cryptonote_config.h>

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shlobj.h>
#include <strsafe.h>
#else 
#include <sys/utsname.h>
#endif


namespace tools
{
    #ifdef WIN32
      std::string get_windows_version_display_string()
      {
        typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
        typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
    #define BUFSIZE 10000

        char pszOS[BUFSIZE] = {0};
        OSVERSIONINFOEX osvi;
        SYSTEM_INFO si;
        PGNSI pGNSI;
        PGPI pGPI;
        BOOL bOsVersionInfoEx;
        DWORD dwType;

        ZeroMemory(&si, sizeof(SYSTEM_INFO));
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*) &osvi);

        if(!bOsVersionInfoEx) return pszOS;

        // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.

        pGNSI = (PGNSI) GetProcAddress(
          GetModuleHandle(TEXT("kernel32.dll")),
          "GetNativeSystemInfo");
        if(NULL != pGNSI)
          pGNSI(&si);
        else GetSystemInfo(&si);

        if ( VER_PLATFORM_WIN32_NT==osvi.dwPlatformId &&
          osvi.dwMajorVersion > 4 )
        {
          StringCchCopy(pszOS, BUFSIZE, TEXT("Microsoft "));

          // Test for the specific product.

          if ( osvi.dwMajorVersion == 6 )
          {
            if( osvi.dwMinorVersion == 0 )
            {
              if( osvi.wProductType == VER_NT_WORKSTATION )
                StringCchCat(pszOS, BUFSIZE, TEXT("windows Vista "));
              else StringCchCat(pszOS, BUFSIZE, TEXT("windows Server 2008 " ));
            }

            if ( osvi.dwMinorVersion == 1 )
            {
              if( osvi.wProductType == VER_NT_WORKSTATION )
                StringCchCat(pszOS, BUFSIZE, TEXT("windows 7 "));
              else StringCchCat(pszOS, BUFSIZE, TEXT("windows Server 2008 R2 " ));
            }

            pGPI = (PGPI) GetProcAddress(
              GetModuleHandle(TEXT("kernel32.dll")),
              "GetProductInfo");

            pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);

            switch( dwType )
            {
            case PRODUCT_ULTIMATE:
              StringCchCat(pszOS, BUFSIZE, TEXT("Ultimate Edition" ));
              break;
            case PRODUCT_PROFESSIONAL:
              StringCchCat(pszOS, BUFSIZE, TEXT("Professional" ));
              break;
            case PRODUCT_HOME_PREMIUM:
              StringCchCat(pszOS, BUFSIZE, TEXT("Home Premium Edition" ));
              break;
            case PRODUCT_HOME_BASIC:
              StringCchCat(pszOS, BUFSIZE, TEXT("Home Basic Edition" ));
              break;
            case PRODUCT_ENTERPRISE:
              StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition" ));
              break;
            case PRODUCT_BUSINESS:
              StringCchCat(pszOS, BUFSIZE, TEXT("Business Edition" ));
              break;
            case PRODUCT_STARTER:
              StringCchCat(pszOS, BUFSIZE, TEXT("Starter Edition" ));
              break;
            case PRODUCT_CLUSTER_SERVER:
              StringCchCat(pszOS, BUFSIZE, TEXT("Cluster Server Edition" ));
              break;
            case PRODUCT_DATACENTER_SERVER:
              StringCchCat(pszOS, BUFSIZE, TEXT("Datacenter Edition" ));
              break;
            case PRODUCT_DATACENTER_SERVER_CORE:
              StringCchCat(pszOS, BUFSIZE, TEXT("Datacenter Edition (core installation)" ));
              break;
            case PRODUCT_ENTERPRISE_SERVER:
              StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition" ));
              break;
            case PRODUCT_ENTERPRISE_SERVER_CORE:
              StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition (core installation)" ));
              break;
            case PRODUCT_ENTERPRISE_SERVER_IA64:
              StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition for Itanium-based Systems" ));
              break;
            case PRODUCT_SMALLBUSINESS_SERVER:
              StringCchCat(pszOS, BUFSIZE, TEXT("Small Business Server" ));
              break;
            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
              StringCchCat(pszOS, BUFSIZE, TEXT("Small Business Server Premium Edition" ));
              break;
            case PRODUCT_STANDARD_SERVER:
              StringCchCat(pszOS, BUFSIZE, TEXT("Standard Edition" ));
              break;
            case PRODUCT_STANDARD_SERVER_CORE:
              StringCchCat(pszOS, BUFSIZE, TEXT("Standard Edition (core installation)" ));
              break;
            case PRODUCT_WEB_SERVER:
              StringCchCat(pszOS, BUFSIZE, TEXT("Web Server Edition" ));
              break;
            }
          }

          if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
          {
            if( GetSystemMetrics(SM_SERVERR2) )
              StringCchCat(pszOS, BUFSIZE, TEXT( "windows Server 2003 R2, "));
            else if ( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER )
              StringCchCat(pszOS, BUFSIZE, TEXT( "windows Storage Server 2003"));
            else if ( osvi.wSuiteMask & VER_SUITE_WH_SERVER )
              StringCchCat(pszOS, BUFSIZE, TEXT( "windows Home Server"));
            else if( osvi.wProductType == VER_NT_WORKSTATION &&
              si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
            {
              StringCchCat(pszOS, BUFSIZE, TEXT( "windows XP Professional x64 Edition"));
            }
            else StringCchCat(pszOS, BUFSIZE, TEXT("windows Server 2003, "));

            // Test for the server type.
            if ( osvi.wProductType != VER_NT_WORKSTATION )
            {
              if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 )
              {
                if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                  StringCchCat(pszOS, BUFSIZE, TEXT( "Datacenter Edition for Itanium-based Systems" ));
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                  StringCchCat(pszOS, BUFSIZE, TEXT( "Enterprise Edition for Itanium-based Systems" ));
              }

              else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
              {
                if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                  StringCchCat(pszOS, BUFSIZE, TEXT( "Datacenter x64 Edition" ));
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                  StringCchCat(pszOS, BUFSIZE, TEXT( "Enterprise x64 Edition" ));
                else StringCchCat(pszOS, BUFSIZE, TEXT( "Standard x64 Edition" ));
              }

              else
              {
                if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
                  StringCchCat(pszOS, BUFSIZE, TEXT( "Compute Cluster Edition" ));
                else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                  StringCchCat(pszOS, BUFSIZE, TEXT( "Datacenter Edition" ));
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                  StringCchCat(pszOS, BUFSIZE, TEXT( "Enterprise Edition" ));
                else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
                  StringCchCat(pszOS, BUFSIZE, TEXT( "Web Edition" ));
                else StringCchCat(pszOS, BUFSIZE, TEXT( "Standard Edition" ));
              }
            }
          }

          if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
          {
            StringCchCat(pszOS, BUFSIZE, TEXT("windows XP "));
            if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
              StringCchCat(pszOS, BUFSIZE, TEXT( "Home Edition" ));
            else StringCchCat(pszOS, BUFSIZE, TEXT( "Professional" ));
          }

          if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
          {
            StringCchCat(pszOS, BUFSIZE, TEXT("windows 2000 "));

            if ( osvi.wProductType == VER_NT_WORKSTATION )
            {
              StringCchCat(pszOS, BUFSIZE, TEXT( "Professional" ));
            }
            else
            {
              if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                StringCchCat(pszOS, BUFSIZE, TEXT( "Datacenter Server" ));
              else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                StringCchCat(pszOS, BUFSIZE, TEXT( "Advanced Server" ));
              else StringCchCat(pszOS, BUFSIZE, TEXT( "Server" ));
            }
          }

          // Include service pack (if any) and build number.

          if(std::strlen(osvi.szCSDVersion) > 0 )
          {
            StringCchCat(pszOS, BUFSIZE, TEXT(" ") );
            StringCchCat(pszOS, BUFSIZE, osvi.szCSDVersion);
          }

          TCHAR buf[80];

          StringCchPrintf( buf, 80, TEXT(" (build %d)"), osvi.dwBuildNumber);
          StringCchCat(pszOS, BUFSIZE, buf);

          if ( osvi.dwMajorVersion >= 6 )
          {
            if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
              StringCchCat(pszOS, BUFSIZE, TEXT( ", 64-bit" ));
            else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )
              StringCchCat(pszOS, BUFSIZE, TEXT(", 32-bit"));
          }

          return pszOS;
        }
        else
        {
          printf( "This sample does not support this version of windows.\n");
          return pszOS;
        }
      }
    #else
    std::string get_nix_version_display_string()
    {
      utsname un;

      if(uname(&un) < 0)
        return std::string("*nix: failed to get os version");
      return std::string() + un.sysname + " " + un.version + " " + un.release;
    }
    #endif



    std::string get_os_version_string()
    {
        #ifdef WIN32
        return get_windows_version_display_string();
        #else
        return get_nix_version_display_string();
        #endif
    }



    #ifdef WIN32
      std::string get_special_folder_path(int nfolder, bool iscreate)
      {
        char psz_path[MAX_PATH] = "";

        if(SHGetSpecialFolderPathA(NULL, psz_path, nfolder, iscreate)) {
          return psz_path;
        }

        return "";
      }
    #endif

    std::string getDefaultDataDirectory()
    {
        // windows < Vista: C:\Documents and Settings\Username\Application Data\CRYPTONOTE_NAME
        // windows >= Vista: C:\Users\Username\AppData\Roaming\CRYPTONOTE_NAME
        // Mac: ~/Library/Application Support/CRYPTONOTE_NAME
        // Unix: ~/.CRYPTONOTE_NAME
        std::string config_folder;
        #ifdef WIN32
        // windows
        config_folder = get_special_folder_path(CSIDL_APPDATA, true) + "/" + CryptoNote::CRYPTONOTE_NAME;
        #else
        std::string pathRet;
        char* pszHome = getenv("HOME");
        if (pszHome == NULL || std::strlen(pszHome) == 0)
          pathRet = "/";
        else
          pathRet = pszHome;
        #ifdef MAC_OSX
        // Mac
        pathRet /= "Library/Application Support";
        config_folder =  (pathRet + "/" + CryptoNote::CRYPTONOTE_NAME);
        #else
        // Unix
        config_folder = (pathRet + "/." + CryptoNote::CRYPTONOTE_NAME);
        #endif
        #endif

        return config_folder;
    }

    bool create_directories_if_necessary(const std::string& path)
    {
          std::error_code e;
          fs::create_directories(path, e);
          return e.value() == 0;
    }

    bool directoryExists(const std::string &path)
    {
          std::error_code e;
          return fs::is_directory(path, e);
    }
}
