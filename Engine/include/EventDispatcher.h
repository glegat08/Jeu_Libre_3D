#pragma once
#include "Receiver.h"
#include "Event.h"
#include "Dispatcher.h"

using EventDispatcher = KGR::FNDispatcher<
	Receiver,				//LhsType
	Event,					//RhsType
	void,					//ReturnType
	KGR::WhrapperTypes<>>;  //Args