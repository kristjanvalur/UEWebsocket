/*
* uewebsocket - unreal engine 4 websocket plugin
*
* Copyright (C) 2017 feiwu <feixuwu@outlook.com>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation:
*  version 2.1 of the License.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
*  MA  02110-1301  USA
*/


#include "WebSocket.h"
#include "WebSocketBlueprintLibrary.h"
#include "WebSocketContext.h"
#include "UObjectGlobals.h"
#include "WebSocketBase.h"
#include "Paths.h"
#include "FileManager.h"
#include "FileHelper.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/Base64.h"
#include <fstream>

#include "SystemCA.h"

#define MAX_PAYLOAD	64*1024

#if PLATFORM_WINDOWS
#define CAN_USE_PLATFORM_CA 1  /* windows can use the built in windows platform bundle */
#else
#define CAN_USE_PLATFORM_CA 0 /* rely on the bundle in WebSocketCA. TODO: Allow windows cert store */
#endif


extern TWeakObjectPtr<UWebSocketContext> s_websocketCtx;

// the global log level
#if 1 || ! defined UE_BUILD_DEBUG
static int log_level = LLL_ERR | LLL_WARN | LLL_NOTICE;
#else
static int log_level = LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO | LLL_CLIENT | LLL_DEBUG;
#endif

#if PLATFORM_UWP
#elif PLATFORM_HTML5
#else
static struct lws_protocols protocols[] = {
	/* first protocol must always be HTTP handler */

	{
		"UEWebsocket",
		UWebSocketContext::callback_echo,
		0,
		MAX_PAYLOAD,
	},
	{
		NULL, NULL, 0, 0		/* End of list */
	}
};

static const struct lws_extension exts[] = {
	{
		"permessage-deflate",
		lws_extension_callback_pm_deflate,
		"permessage-deflate; client_no_context_takeover"
	},
	{
		"deflate-frame",
		lws_extension_callback_pm_deflate,
		"deflate_frame"
	},
	{ NULL, NULL, NULL /* terminator */ }
};
#endif

void UWebSocketContext::BeginDestroy()
{
	Super::BeginDestroy();
	DestroyCtx();
	s_websocketCtx.Reset();
}

#if PLATFORM_UWP
#elif PLATFORM_HTML5
#else

// helper to decide to close, below.
static bool should_close(UWebSocketBase *pWebSocketBase)
{
	if (pWebSocketBase)
	{
		// a close was requested
		if (pWebSocketBase->closing)
		{
			pWebSocketBase->Cleanlws();
			pWebSocketBase->OnClosed.Broadcast();
			return true;
		}
	}
	else
	{
		// the WebSocketBase has gone.  Close quietly.
		return true;
	}
	return false;
}


int UWebSocketContext::callback_echo(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	UWebSocketBase* pWebSocketBase = (UWebSocketBase*)user;
	
	switch (reason)
	{
	case LWS_CALLBACK_CLIENT_CLOSED:
		if (should_close(pWebSocketBase)) return -1;
		pWebSocketBase->Cleanlws();
		pWebSocketBase->OnClosed.Broadcast();
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
	{
		FString strError = UTF8_TO_TCHAR(in);
		UE_LOG(WebSocket, Error, TEXT("libwebsocket connect error:%s"), *strError);
		if (should_close(pWebSocketBase)) return -1;
		pWebSocketBase->Cleanlws();
		pWebSocketBase->OnConnectError.Broadcast(strError);
	}
		break;

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		if (should_close(pWebSocketBase)) return -1; 
		pWebSocketBase->OnConnectComplete.Broadcast();
		break;

	case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
	{
		if (should_close(pWebSocketBase)) return -1; 
		unsigned char **p = (unsigned char **)in, *end = (*p) + len;
		if (!pWebSocketBase->ProcessHeader(p, end))
		{
			return -1;
		}
	}
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		if (should_close(pWebSocketBase)) return -1; 
		pWebSocketBase->ProcessRead((const char*)in, (int)len, !!lws_frame_is_binary(wsi));
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		if (should_close(pWebSocketBase)) return -1; 
		pWebSocketBase->ProcessWriteable();
		break;

	case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS:
	{
#if CAN_USE_PLATFORM_CA
		add_system_ca_bundle(user);  // user is SSL_CTX*
#endif
		break;
	}

	default:
		break;
	}

	return 0;
}
#endif

UWebSocketContext::UWebSocketContext()
{
#if PLATFORM_UWP
#elif PLATFORM_HTML5
#else
	mlwsContext = nullptr;
#endif
}

extern char g_caArray[];

static void log_handler(int level, const char *line)
{
	FString str(line);
	str.TrimEndInline();
	UWebSocketLib::DoLog(level, str);
	if (level == LLL_ERR)
	{
		UE_LOG(LibWebsockets, Error, TEXT("lws %d: %s"), level, *str);
	}
	else if (level == LLL_WARN)
	{
		UE_LOG(LibWebsockets, Warning, TEXT("lws %d: %s"), level, *str);
	}
	else {
		UE_LOG(LibWebsockets, Log, TEXT("lws %d: %s"), level, *str);
	}
}

void UWebSocketContext::CreateCtx(const FWebSocketContextOptions &options)
{

	lws_set_log_level(log_level, log_handler);

#if PLATFORM_UWP
#elif PLATFORM_HTML5
#else
	struct lws_context_creation_info info;
	memset(&info, 0, sizeof info);

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
	info.ws_ping_pong_interval = options.iPingPongInterval; // can set pingpong here.
	info.timeout_secs = options.iTimeOutSecs;
	info.ssl_cert_filepath = NULL;
	info.ssl_private_key_filepath = NULL;

	info.extensions = exts;
	info.options |= LWS_SERVER_OPTION_VALIDATE_UTF8;
	info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#if CAN_USE_PLATFORM_CA
	bool use_platform_cert_bundle = options.bUsePlatformCertBundle;
#else
	bool use_platform_cert_bundle = false;
#endif

	if (use_platform_cert_bundle)
	{
		info.options |= LWS_SERVER_OPTION_CREATE_VHOST_SSL_CTX;
	}
	else {
		FString PEMFilename = FPaths::ProjectSavedDir() / TEXT("ca-bundle.pem");
		PEMFilename = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*PEMFilename);
#if PLATFORM_ANDROID
		extern FString GExternalFilePath;
		PEMFilename = GExternalFilePath / TEXT("ca-bundle.pem");
#endif
		mstrCAPath = TCHAR_TO_UTF8(*PEMFilename);
		std::ofstream f(mstrCAPath.c_str());
		if (f.is_open())
		{
			FString strBase64CA(ANSI_TO_TCHAR(g_caArray));
			FString strCAContent;
			FBase64::Decode(strBase64CA, strCAContent);
			std::string strOriginCA = TCHAR_TO_UTF8(*strCAContent);
			f.write(strOriginCA.c_str(), strOriginCA.size());
			f.close();
		}
		else
		{
			UE_LOG(WebSocket, Warning, TEXT(" websocket: fail open file: '%s'"), *PEMFilename);
			UKismetSystemLibrary::PrintString(this, TEXT(" websocket: fail open file:") + PEMFilename, true, true, FLinearColor(0.0, 0.66, 1.0), 1000);
		}

		UE_LOG(WebSocket, Log, TEXT(" websocket: using generated PEM file: '%s'"), *PEMFilename);
		//UKismetSystemLibrary::PrintString(this, TEXT("full dir:") + PEMFilename, true, true, FLinearColor(0.0, 0.66, 1.0), 1000);

		info.client_ssl_ca_filepath = mstrCAPath.c_str();
	}

	mlwsContext = lws_create_context(&info);
	if (mlwsContext == nullptr)
	{
		//UE_LOG(WebSocket, Error, TEXT("libwebsocket Init fail"));
	}
#endif
}

void UWebSocketContext::DestroyCtx()
{
	if (mlwsContext != nullptr)
	{
		lws_context_destroy(mlwsContext);
		mlwsContext = nullptr;
	}
}

void UWebSocketContext::Tick(float DeltaTime)
{
#if PLATFORM_UWP
#elif PLATFORM_HTML5
#else
	if (mlwsContext != nullptr)
	{
		lws_service(mlwsContext, 0);
	}
#endif
}

bool UWebSocketContext::IsTickable() const
{
	return true;
}

TStatId UWebSocketContext::GetStatId() const
{
	return TStatId();
}

void UWebSocketContext::SetLogLevel(int32 level)
{
	log_level = 0;
	if (level & (1 << (int)(EWebSocketLogLevel::ERR))) log_level |= LLL_ERR;
	if (level & (1 << (int)(EWebSocketLogLevel::WARN))) log_level |= LLL_WARN;
	if (level & (1 << (int)(EWebSocketLogLevel::NOTICE))) log_level |= LLL_NOTICE;
	if (level & (1 << (int)(EWebSocketLogLevel::INFO))) log_level |= LLL_INFO;
	if (level & (1 << (int)(EWebSocketLogLevel::DEBUG))) log_level |= LLL_DEBUG;

	if (level & (1 << (int)(EWebSocketLogLevel::PARSER))) log_level |= LLL_PARSER;
	if (level & (1 << (int)(EWebSocketLogLevel::HEADER))) log_level |= LLL_HEADER;
	if (level & (1 << (int)(EWebSocketLogLevel::EXT))) log_level |= LLL_EXT;
	if (level & (1 << (int)(EWebSocketLogLevel::CLIENT))) log_level |= LLL_CLIENT;
	if (level & (1 << (int)(EWebSocketLogLevel::LATENCY))) log_level |= LLL_LATENCY;
	if (level & (1 << (int)(EWebSocketLogLevel::USER))) log_level |= LLL_USER;
	if (level & (1 << (int)(EWebSocketLogLevel::THREAD))) log_level |= LLL_THREAD;

	lws_set_log_level(log_level, log_handler);
}

UWebSocketBase* UWebSocketContext::CreateSocket()
{
#if PLATFORM_UWP
#elif PLATFORM_HTML5
#else
	if (mlwsContext == nullptr)
	{
		return nullptr;
	}
#endif

	UWebSocketBase* pNewSocketBase = NewObject<UWebSocketBase>();

#if PLATFORM_UWP
#elif PLATFORM_HTML5
#else
	pNewSocketBase->mWebSocketContext = this;
#endif
	return pNewSocketBase;
}

UWebSocketBase* UWebSocketContext::Connect(const FString& uri, const TMap<FString, FString>& header, const FWebSocketConnectOptions &options, bool& connectFail)
{

	UWebSocketBase* pNewSocketBase = CreateSocket();
	if (pNewSocketBase == nullptr)
	{
		return nullptr;
	}

	connectFail = !(pNewSocketBase->Connect(uri, header, options) );

	return pNewSocketBase;
}

UWebSocketBase* UWebSocketContext::Connect(const FString& uri, bool& connectFail)
{
	return Connect(uri, TMap<FString, FString>(), FWebSocketConnectOptions(), connectFail);
}
