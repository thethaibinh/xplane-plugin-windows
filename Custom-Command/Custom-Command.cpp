// Downloaded from https://developer.x-plane.com/code-sample/custom-command/


// Custom Commands
//
// This example program illustrates creating a custom command.  In this case clicking a generic trigger on
// your aircraft panel writes to the DataRef controlling the pilots head position either continuously or one
// knotch at a time. We could use the existing position command: sim/view/move_right, however for illustrative
// purposes, this example uses a custom command:  BSUB/ViewPoint/MoveRight
//
// For this example to work it is necessary to create a generic trigger on the panel of your aircraft keyed
// to the command: BSUB/ViewPoint/MoveRight.
//
// Content added by BlueSideUpBob.
//

#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMScenery.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

XPLMDataRef gHeadPositionXDataRef = NULL;

XPLMCommandRef MyCommand = NULL;

int    MyCommandHandler(XPLMCommandRef        inCommand,
					 XPLMCommandPhase      inPhase,
					 void *                inRefcon);

PLUGIN_API int XPluginStart(
					 char *        outName,
					 char *        outSig,
					 char *        outDesc)
{
	// Plugin Info
	strcpy(outName, "CommandControl");
	strcpy(outSig, "BlueSideUpBob.Example.CommandControl");
	strcpy(outDesc, "This example illustrates creating and sending a custom command to X-Plane using a generic trigger.");

	// Create the test command, this will move the pilots point of view 10 cm to the right.
	MyCommand = XPLMCreateCommand("BSUB/ViewPoint/MoveRight", "Move Right");

	// Register our custom command
	XPLMRegisterCommandHandler(MyCommand,              // in Command name
							 MyCommandHandler,       // in Handler
							 1,                      // Receive input before plugin windows.
							 (void *) 0);            // inRefcon.

	gHeadPositionXDataRef = XPLMFindDataRef("sim/aircraft/view/acf_peX");

	return 1;
}

PLUGIN_API void    XPluginStop(void)
{
	XPLMUnregisterCommandHandler(MyCommand, MyCommandHandler, 0, 0);
}


PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(
			 XPLMPluginID    inFromWho,
			 long            inMessage,
			 void *          inParam)
{

}

int    MyCommandHandler(XPLMCommandRef       inCommand,
				 XPLMCommandPhase     inPhase,
				 void *               inRefcon)
{

	//  Use the structure below to have the command executed
	//  continuously while the button is being held down.
	if (inPhase == xplm_CommandContinue)
	{
		XPLMSetDataf(gHeadPositionXDataRef, XPLMGetDataf(gHeadPositionXDataRef) + .1);
	}

	//  Use this structure to have the command executed on button up only.
	if (inPhase == xplm_CommandEnd)
	{
		XPLMSetDataf(gHeadPositionXDataRef, XPLMGetDataf(gHeadPositionXDataRef) + .1);
	}

	// Return 1 to pass the command to plugin windows and X-Plane.
	// Returning 0 disables further processing by X-Plane.
	// In this case we might return 0 or 1 because X-Plane does not duplicate our command.
	return 0;
}

