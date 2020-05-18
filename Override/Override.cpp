// Downloaded from https://developer.x-plane.com/code-sample/override/


#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMDataAccess.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if IBM
#include <windows.h>
#endif
#include <vector>

#define NUMBER_OF_OVERRIDES 20

static int MenuItem1, MenuItem2, MenuItem3;

static XPWidgetID OverrideWidget = NULL, OverridePanel = NULL, OverridePreviousButton = NULL, OverrideNextButton = NULL;
static XPWidgetID OverrideEdit[8] = {NULL};
static XPWidgetID OverrideCheckBox[8] = {NULL};

static char DataRefGroup[] = "sim/operation/override/";
static char DataRefDesc[NUMBER_OF_OVERRIDES][40] = {"override_planepath", "override_joystick", "override_artstab",
											 "override_flightcontrol", "override_gearbrake", "override_navneedles",
												"override_adf", "override_dme", "override_gps", "override_flightdir", "override_annunciators",
												"override_autopilot","override_pfc_autopilot_lites",
												"override_joystick_heading", "override_joystick_pitch",
												"override_joystick_roll", "override_throttles",
												"override_groundplane", "disable_cockpit_object", "disable_twosided_fuselage"};

static int NumberOfOverrides, OverrideScreenNumber, MaxScreenNumber;
typedef	std::vector<XPLMDataRef> aXPLMDataRefID;

static aXPLMDataRefID DataRefID;
static XPLMDataRef gSpecialDataRef;

static void OverrideMenuHandler(void *, void *);
static void CreateOverride(int x1, int y1, int w, int h);
static int OverrideHandler(
						XPWidgetMessage			inMessage,
						XPWidgetID				inWidget,
						intptr_t				inParam1,
						intptr_t				inParam2);
static void RefreshOverride(void);
static void GetDataRefIds(void);

static int GetDataRefState(XPLMDataRef DataRefID);
static void SetDataRefState(XPLMDataRef DataRefID, int State);

PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	XPLMMenuID	id;
	int			item;

	strcpy(outName, "Override");
	strcpy(outSig, "xpsdk.examples.override");
	strcpy(outDesc, "A plug-in that Overrides Xplane.");

	item = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Override Xplane", NULL, 1);

	id = XPLMCreateMenu("Override", XPLMFindPluginsMenu(), item, OverrideMenuHandler, NULL);

	XPLMAppendMenuItem(id, "Enable/Disable Override", (void *) "EnableDisableOverride", 1);

	MenuItem1 = 0;
	MenuItem2 = 0;
	MenuItem3 = 0;

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	if (MenuItem1 == 1)
	{
		XPDestroyWidget(OverrideWidget, 1);
		MenuItem1 = 0;
	}
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam)
{
}

float OverrideLoopCB(float elapsedMe, float elapsedSim, int counter, void * refcon)
{
	return 1;
}

void OverrideMenuHandler(void * mRef, void * iRef)
{
	if (!strcmp((char *) iRef, "EnableDisableOverride"))
	{
		if (MenuItem1 == 0)
		{
			OverrideScreenNumber = 0;
			CreateOverride(300, 550, 350, 380);
			MenuItem1 = 1;
		}
		else
			if(!XPIsWidgetVisible(OverrideWidget))
				XPShowWidget(OverrideWidget);
	}
}


void CreateOverride(int x, int y, int w, int h)
{
	int x2 = x + w;
	int y2 = y - h;
	int Item;
	int WindowCentre = x+w/2;
	int yOffset;
	char Buffer[255];

	DataRefID.clear();
	memset(OverrideCheckBox, 0, sizeof(OverrideCheckBox));

	GetDataRefIds();

	OverrideWidget = XPCreateWidget(x, y, x2, y2,
					1, "Xplane Override", 1, NULL,
					xpWidgetClass_MainWindow);

	XPSetWidgetProperty(OverrideWidget, xpProperty_MainWindowHasCloseBoxes, 1);

	OverridePanel = XPCreateWidget(x+50, y-50, x2-50, y2+50,
					1, "", 0, OverrideWidget,
					xpWidgetClass_SubWindow);

	XPSetWidgetProperty(OverridePanel, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);


	OverridePreviousButton = XPCreateWidget(WindowCentre-80, y2+24, WindowCentre-10, y2+2,
					1, "Previous", 0, OverrideWidget,
					xpWidgetClass_Button);

	XPSetWidgetProperty(OverridePreviousButton, xpProperty_ButtonType, xpPushButton);

	OverrideNextButton = XPCreateWidget(WindowCentre+10, y2+24, WindowCentre+80, y2+2,
					1, "Next", 0, OverrideWidget,
					xpWidgetClass_Button);

	XPSetWidgetProperty(OverrideNextButton, xpProperty_ButtonType, xpPushButton);

	for (Item=0; Item<8; Item++)
	{
		yOffset = (45+28+(Item*30));
		strcpy(Buffer, "");
		OverrideEdit[Item] = XPCreateWidget(x+60, y-yOffset, x+60+200, y-yOffset-20,
					1, Buffer, 0, OverrideWidget,
					xpWidgetClass_TextField);
		XPSetWidgetProperty(OverrideEdit[Item], xpProperty_TextFieldType, xpTextEntryField);
	}

	for (Item=0; Item<8; Item++)
	{
		yOffset = (45+28+(Item*30));
		OverrideCheckBox[Item] = XPCreateWidget(x+260, y-yOffset, x+260+22, y-yOffset-20,
					1, "", 0, OverrideWidget,
					xpWidgetClass_Button);

		XPSetWidgetProperty(OverrideCheckBox[Item], xpProperty_ButtonType, xpRadioButton);
		XPSetWidgetProperty(OverrideCheckBox[Item], xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
		XPSetWidgetProperty(OverrideCheckBox[Item], xpProperty_ButtonState, 1);
	}

	RefreshOverride();

	XPAddWidgetCallback(OverrideWidget, (XPWidgetFunc_t)OverrideHandler);
}

int	OverrideHandler(
						XPWidgetMessage			inMessage,
						XPWidgetID				inWidget,
						intptr_t				inParam1,
						intptr_t				inParam2)
{
	int Item, State;

	if (inMessage == xpMessage_CloseButtonPushed)
		{
			if (MenuItem1 == 1)
			{
				XPHideWidget(OverrideWidget);
			}
			return 1;
		}

	if (inMessage == xpMsg_PushButtonPressed)
	{
		if (inParam1 == (intptr_t)OverridePreviousButton)
		{
			OverrideScreenNumber--;
			if (OverrideScreenNumber<0)
				OverrideScreenNumber = 0;
			RefreshOverride();
			return 1;
		}

		if (inParam1 == (intptr_t)OverrideNextButton)
		{
			OverrideScreenNumber++;
			if (OverrideScreenNumber>MaxScreenNumber)
				OverrideScreenNumber = MaxScreenNumber;
			RefreshOverride();
			return 1;
		}
	}

	if (inMessage == xpMsg_ButtonStateChanged)
	{
		for (Item=0; Item<8; Item++)
		{
			if (DataRefID[Item+(OverrideScreenNumber*8)])
			{
				State = XPGetWidgetProperty(OverrideCheckBox[Item], xpProperty_ButtonState, 0);
				SetDataRefState(DataRefID[Item+(OverrideScreenNumber*8)], State);
			}
		}
	}
	return 0;
}						


void RefreshOverride(void)
{
	int Item;
	char Buffer[255];
	
	for (Item=0; Item<8; Item++)
	{
		strcpy(Buffer, "");
		if ((Item+(OverrideScreenNumber*8)) < NumberOfOverrides)
		{
			if (DataRefID[Item+(OverrideScreenNumber*8)])
			{
				XPSetWidgetDescriptor(OverrideEdit[Item], DataRefDesc[Item+(OverrideScreenNumber*8)]);
				if (GetDataRefState(DataRefID[Item+(OverrideScreenNumber*8)]))
					XPSetWidgetProperty(OverrideCheckBox[Item], xpProperty_ButtonState, 1);
				else
					XPSetWidgetProperty(OverrideCheckBox[Item], xpProperty_ButtonState, 0);
				XPSetWidgetProperty(OverrideCheckBox[Item], xpProperty_Enabled, 1);
			}
		}
		else
		{
			XPSetWidgetDescriptor(OverrideEdit[Item], Buffer);
			XPSetWidgetProperty(OverrideCheckBox[Item], xpProperty_ButtonState, 0);
		}
	}
	if (OverrideScreenNumber == 0)
		XPSetWidgetProperty(OverridePreviousButton, xpProperty_Enabled, 0);
	else
		XPSetWidgetProperty(OverridePreviousButton, xpProperty_Enabled, 1);
		XPSetWidgetDescriptor(OverridePreviousButton, "Previous");

	if (OverrideScreenNumber == MaxScreenNumber)
		XPSetWidgetProperty(OverrideNextButton, xpProperty_Enabled, 0);
	else
		XPSetWidgetProperty(OverrideNextButton, xpProperty_Enabled, 1);	
		XPSetWidgetDescriptor(OverrideNextButton, "Next");
}						

void GetDataRefIds(void)
{
	int Item, ItemIndex=0;
	XPLMDataRef TempDataRefID;
	char TempDesc[256];

	NumberOfOverrides = NUMBER_OF_OVERRIDES;

	for (Item=0; Item<NumberOfOverrides; Item++)
	{
		strcpy(TempDesc, DataRefGroup);
		strcat(TempDesc, DataRefDesc[Item]);
		TempDataRefID = XPLMFindDataRef(TempDesc);
		if (Item == 0)
			gSpecialDataRef = TempDataRefID;
		DataRefID.push_back(TempDataRefID);
	}

	MaxScreenNumber = (NumberOfOverrides-1) / 8;
}

int GetDataRefState(XPLMDataRef DataRefID)
{
	int	DataRefi, IntVals[8];
	
	memset(IntVals, 0, sizeof(IntVals));
	if (DataRefID == gSpecialDataRef)
	{
		XPLMGetDatavi(DataRefID, IntVals, 0, 8);
		DataRefi = IntVals[0];
	}
	else
		DataRefi = XPLMGetDatai(DataRefID);

	return DataRefi;
}

void SetDataRefState(XPLMDataRef DataRefID, int State)
{
	int	IntVals[8];
	
	memset(IntVals, 0, sizeof(IntVals));
	if (DataRefID == gSpecialDataRef)
	{
		IntVals[0] = State;
		XPLMSetDatavi(DataRefID, IntVals, 0, 8);
	}
	else
		XPLMSetDatai(DataRefID, State);
}

