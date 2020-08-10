// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

// logging from the websockets library
DECLARE_LOG_CATEGORY_EXTERN(LibWebsockets, Log, All);
// logging from the UEWebsocket library
DECLARE_LOG_CATEGORY_EXTERN(WebSocket, Log, All);

class FWebSocketModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};