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

#pragma once

#include "UObject/NoExportTypes.h"
#include "Tickable.h"

// openssl (included by lws) has a struct called UI which conflicts with UE4. Redefine it.
#define UI UI_ST
#if PLATFORM_UWP
#elif PLATFORM_HTML5
#elif PLATFORM_WINDOWS  
#include "PreWindowsApi.h"
#include "libwebsockets.h"
#include "PostWindowsApi.h" 
#else
#include "libwebsockets.h"
#endif
#undef UI

// see if we are 3.2 or greater
#define LWS_LIBRARY_VERSION_32_PLUS (LWS_LIBRARY_VERSION_MAJOR > 3 || (LWS_LIBRARY_VERSION_MAJOR == 3 && LWS_LIBRARY_VERSION_MINOR >= 2))

#include <iostream>

#include "WebSocketBase.h"

#include "WebSocketContext.generated.h"


class UWebSocketBase;
/**
 * 
 */
UCLASS(Blueprintable)
class UWebSocketContext : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
public:

	UWebSocketContext();

	~UWebSocketContext();
	
	void CreateCtx(const FWebSocketContextOptions &coptions);
	void DestroyCtx();

	virtual void BeginDestroy() override;

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	static void SetLogLevel(int32 level);
	
	//Create an unconnected websocket object
	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	UWebSocketBase* CreateSocket();

	UWebSocketBase* Connect(const FString& uri, bool& connectFail);
	UWebSocketBase* Connect(const FString& uri, const TMap<FString, FString>& header, const FWebSocketConnectOptions &options, bool& connectFail);
#if PLATFORM_UWP
#elif PLATFORM_HTML5
#else
	static int callback_echo(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
	struct lws_context *GetContext() { return mlwsContext;}
#endif
	
private:

#if PLATFORM_UWP
#elif PLATFORM_HTML5
#else
	struct lws_context* mlwsContext;
	std::string mstrCAPath;
#endif
};
