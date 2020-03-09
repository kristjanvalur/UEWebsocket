// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System.IO;
using System;
using UnrealBuildTool;

public class WebSocket : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string EngineMajorVersion;
    private string EngineMinorVersion;
    private string EnginePatchVersion;

    string GetEngineDirectory()
    {
        string magicKey = "UE_ENGINE_DIRECTORY=";
        for (var i = 0; i < PublicDefinitions.Count; i++)
        {
            if (PublicDefinitions[i].IndexOf(magicKey) >= 0)
            {
                return PublicDefinitions[i].Substring(magicKey.Length + 1);
            }
        }

        return "";
    }

    public WebSocket(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivatePCHHeaderFile = "Public/WebSocket.h";

        string strEngineDir = GetEngineDirectory();
        string strEngineVersion = ReadEngineVersion(strEngineDir);

        System.Console.WriteLine("version:" + strEngineVersion);

        PrivateIncludePaths.AddRange(
			new string[] {
				"WebSocket/Private",
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "Json",
                "JsonUtilities",
                "Sockets",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "Json",
                "JsonUtilities",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        if(int.Parse(EngineMinorVersion) > 23)
        {
            PublicDefinitions.Add("PLATFORM_HTML5=0");
        }
		
        // This file is changed from the official version.
        // It is assumes that we have a libwebsocket_static422.lib which has been compiled against the 4.22
        // version of the openssl third party libs.

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("PLATFORM_UWP=0");
            PrivateDependencyModuleNames.Add("zlib");
			
			// OpenSSL versions provided with UE4 for windows
			// We have compiled the libwebsockets libs to use the same as the engine uses.
			// for 4.22+
			// 4.24 1.1.1c
			// 4.22+ 1.1.1
			// 4.14+ 1.0.2h
			
            // kards change, also 22, 23, 24
            if (Int32.Parse(EngineMinorVersion) >= 20)
            {
                PrivateDependencyModuleNames.Add("OpenSSL");
            }

            PrivateIncludePaths.Add("WebSocket/ThirdParty/include/Win64");
            string strStaticPath = Path.GetFullPath(Path.Combine(ModulePath, "ThirdParty/lib/Win64/"));
            PublicLibraryPaths.Add(strStaticPath);

            // for 4.21
            if(EngineMinorVersion == "21" || EngineMinorVersion == "20")
            {
                string[] StaticLibrariesX64 = new string[] {
                "websockets_static.lib",
                };

                foreach (string Lib in StaticLibrariesX64)
                {
                    PublicAdditionalLibraries.Add(Lib);
                }
            }
            /*else if(EngineMinorVersion == "22" || EngineMinorVersion == "23")*/
            else
            {
				// use this library, compatible with the OpenSSL module in the engine.
                PublicAdditionalLibraries.Add("websockets_static422.lib");
               

            }

            
        }
        if (Target.Platform == UnrealTargetPlatform.Win32)
        {
            PublicDefinitions.Add("PLATFORM_UWP=0");
            PrivateDependencyModuleNames.Add("zlib");
            PrivateDependencyModuleNames.Add("OpenSSL");
            PrivateIncludePaths.Add("WebSocket/ThirdParty/include/Win32");

            string strStaticPath = Path.GetFullPath(Path.Combine(ModulePath, "ThirdParty/lib/Win32/"));
            PublicLibraryPaths.Add(strStaticPath);

            // 4.22 and 4.21
            if (EngineMinorVersion == "21" || EngineMinorVersion == "20")
            {
                string[] StaticLibrariesX32 = new string[] {
                    "websockets_static.lib",
                    //"libcrypto.lib",
                    //"libssl.lib",
                };

                foreach (string Lib in StaticLibrariesX32)
                {
                    PublicAdditionalLibraries.Add(Lib);
                }
            }
            /*else if(EngineMinorVersion == "22"|| EngineMinorVersion == "23")*/
            else
            {
                string[] StaticLibrariesX32 = new string[] {
                    "websockets_static422.lib",
                    //"libcrypto.lib",
                    //"libssl.lib",
                };

                foreach (string Lib in StaticLibrariesX32)
                {
                    PublicAdditionalLibraries.Add(Lib);
                }
            }
        }
        /*else if(Target.Platform == UnrealTargetPlatform.HTML5)
        {
            PublicDefinitions.Add("PLATFORM_UWP=0");
            string strStaticPath = Path.GetFullPath(Path.Combine(ModulePath, "ThirdParty/lib/HTML5/"));
            PublicLibraryPaths.Add(strStaticPath);

            string[] StaticLibrariesHTML5 = new string[] {
                "WebSocket.js",
            };

            foreach (string Lib in StaticLibrariesHTML5)
            {
                PublicAdditionalLibraries.Add(strStaticPath + Lib);
            }
        }*/
        else if(Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDefinitions.Add("PLATFORM_UWP=0");
            PrivateIncludePaths.Add("WebSocket/ThirdParty/include/Mac");
            string strStaticPath = Path.GetFullPath(Path.Combine(ModulePath, "ThirdParty/lib/Mac/"));
            //PublicLibraryPaths.Add(strStaticPath);

            string[] StaticLibrariesMac = new string[] {
                "libwebsockets.a",
                "libssl.a",
                "libcrypto.a"
            };

            foreach (string Lib in StaticLibrariesMac)
            {
                PublicAdditionalLibraries.Add(Path.Combine(strStaticPath, Lib) );
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PublicDefinitions.Add("PLATFORM_UWP=0");
            PrivateDependencyModuleNames.Add("OpenSSL");
            PrivateIncludePaths.Add("WebSocket/ThirdParty/include/Linux");
            string strStaticPath = Path.GetFullPath(Path.Combine(ModulePath, "ThirdParty/lib/Linux/"));
            PublicLibraryPaths.Add(strStaticPath);

            string[] StaticLibrariesLinux = null;
            if (int.Parse(EngineMinorVersion) >= 24)
            {
                StaticLibrariesLinux = new string[] {
                    "libwebsockets424.a",
                };
            }
            else
            {
                StaticLibrariesLinux = new string[] {
                    "libwebsockets.a",
                };
            }

            foreach (string Lib in StaticLibrariesLinux)
            {
                PublicAdditionalLibraries.Add(Path.Combine(strStaticPath, Lib));
            }
        }
        else if(false && Target.Platform == UnrealTargetPlatform.IOS)
        {
            PublicDefinitions.Add("PLATFORM_UWP=0");
            PrivateIncludePaths.Add("WebSocket/ThirdParty/include/IOS");
            PrivateDependencyModuleNames.Add("OpenSSL");

            string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath + "/Source/");
            PluginPath = PluginPath.Replace("\\", "/");

            string strStaticPath = PluginPath + "/ThirdParty/lib/IOS/";// Path.GetFullPath(Path.Combine(ModulePath, "ThirdParty/lib/IOS/"));
            PublicLibraryPaths.Add(strStaticPath);

            string[] StaticLibrariesIOS = new string[] {
                "websockets",
                //"ssl",
                //"crypto"
            };

            foreach (string Lib in StaticLibrariesIOS)
            {
                PublicAdditionalLibraries.Add(Lib);
                PublicAdditionalShadowFiles.Add(Path.Combine(strStaticPath, "lib" + Lib + ".a") );
            }
        }
        else if(Target.Platform == UnrealTargetPlatform.IOS)
        {
            PublicDefinitions.Add("PLATFORM_UWP=0");
            // libwebsockets and openssl are provided by ue4
            // UE4.23:  libwebsockets 3.0.0, openssl 1.0.1s
            
            // Just use the libwebsockets built into UE4
            PrivateIncludePathModuleNames.AddRange(
                new string[] 
                {
                    "libWebSockets",
                }
            );
            AddEngineThirdPartyPrivateStaticDependencies(Target, "libWebSockets");
            AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL");

            string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "WebSocket_UPL.xml"));
        }
        else if(false && Target.Platform == UnrealTargetPlatform.Android)
        {
            PublicDefinitions.Add("PLATFORM_UWP=0");
            PrivateIncludePaths.Add("WebSocket/ThirdParty/include/Android");
            string strStaticPath = Path.GetFullPath(Path.Combine(ModulePath, "ThirdParty/lib/Android/armeabi-v7a"));
            PublicLibraryPaths.Add(strStaticPath);

            string strStaticArm64Path = Path.GetFullPath(Path.Combine(ModulePath, "ThirdParty/lib/Android/arm64-v8a"));
            PublicLibraryPaths.Add(strStaticArm64Path);

			// openssl provided by ue4 for android:
			// 4.23 1.0.1s, part of libcurl
			// Android apps are statically linked so we cannot have different versions of libraries
			// in the modules.  openssl is already linked in as part of libcurl.
			// We therefore cannot use the newer 1.1.1 series in this plugin.

			PrivateDependencyModuleNames.Add("OpenSSL");
            string[] StaticLibrariesAndroid = new string[] {
                "websockets",
            //    "ssl",
            //    "crypto"
            };

            foreach (string Lib in StaticLibrariesAndroid)
            {
                PublicAdditionalLibraries.Add(Lib);
            }

            string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "WebSocket_UPL.xml"));
        }
		else if(Target.Platform == UnrealTargetPlatform.Android)
        {
            PublicDefinitions.Add("PLATFORM_UWP=0");
            // libwebsockets and openssl are provided by ue4 for Android
            // UE4.23:  libwebsockets 3.0.0, openssl 1.0.1s
            
            // Just use the libwebsockets built into UE4
            PrivateIncludePathModuleNames.AddRange(
                new string[] 
                {
                    "libWebSockets",
                }
            );
            AddEngineThirdPartyPrivateStaticDependencies(Target, "libWebSockets");
            AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL");

            string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "WebSocket_UPL.xml"));
        }
    }

    private string ReadEngineVersion(string EngineDirectory)
    {
        string EngineVersionFile = Path.Combine(EngineDirectory, "Runtime", "Launch", "Resources", "Version.h");
        string[] EngineVersionLines = File.ReadAllLines(EngineVersionFile);
        for (int i = 0; i < EngineVersionLines.Length; ++i)
        {
            if (EngineVersionLines[i].StartsWith("#define ENGINE_MAJOR_VERSION"))
            {
                EngineMajorVersion = EngineVersionLines[i].Split('\t')[1].Trim(' ');
            }
            else if (EngineVersionLines[i].StartsWith("#define ENGINE_MINOR_VERSION"))
            {
                EngineMinorVersion = EngineVersionLines[i].Split('\t')[1].Trim(' ');
            }
            else if (EngineVersionLines[i].StartsWith("#define ENGINE_PATCH_VERSION"))
            {
                EnginePatchVersion = EngineVersionLines[i].Split('\t')[1].Trim(' ');
            }

        }

        return EngineMajorVersion + "." + EngineMinorVersion + "." + EnginePatchVersion;
    }
}
