// Downloaded from https://developer.x-plane.com/code-sample/control/


#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMCamera.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if IBM
#include <windows.h>
#endif

#define MAX_ITEMS 12
	
static XPLMDataRef gControlDataRef[MAX_ITEMS];

static char DataRefString[MAX_ITEMS][255] = {	"sim/joystick/yoke_pitch_ratio", "sim/joystick/yoke_roll_ratio", "sim/joystick/yoke_heading_ratio",
										"sim/joystick/artstab_pitch_ratio", "sim/joystick/artstab_roll_ratio", "sim/joystick/artstab_heading_ratio",
										"sim/joystick/FC_ptch", "sim/joystick/FC_roll", "sim/joystick/FC_hdng",
										"sim/flightmodel/weight/m_fuel1", "sim/flightmodel/weight/m_fuel2", "sim/flightmodel/weight/m_fuel3"};

static char DataRefDesc[MAX_ITEMS][255] = {"Yoke Pitch", "Yoke Roll", "Yoke Heading", "AS Pitch", "AS Roll", "AS Heading", "FC Pitch", "FC Roll", "FC Heading", "Fuel 1", "Fuel 2", "Fuel 3"};


static float IncrementValue[MAX_ITEMS] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 10.0, 10.0, 10.0};

static int	Element = 0, IntVals[128];
static float FloatVals[128];
static int ByteVals[128];

static int MenuItem1;

static XPWidgetID			ControlWidget = NULL, ControlWindow = NULL;
static XPWidgetID			ControlApplyButton = NULL; 
static XPWidgetID			ControlText[MAX_ITEMS] = {NULL};
static XPWidgetID			ControlEdit[MAX_ITEMS] = {NULL};
static XPWidgetID			UpArrow[MAX_ITEMS] = {NULL};
static XPWidgetID			DownArrow[MAX_ITEMS] = {NULL};

static void ControlMenuHandler(void *, void *);

static void CreateControl(int x1, int y1, int w, int h);

static int ControlHandler(
						XPWidgetMessage			inMessage,
						XPWidgetID				inWidget,
						intptr_t				inParam1,
						intptr_t				inParam2);

static void DisplayFindDataRef(void);
static void ApplyValues(void);
static void RefreshValues(void);

inline	float	HACKFLOAT(float val)
{
	return val;
}
/*
#if IBM
inline	float	HACKFLOAT(float val)
{
	return val;
}
#else
inline long long HACKFLOAT(float val)
{
	double	d = val;
	long long temp;
	temp = *((long long *) &d);
	return temp;
}
#endif
*/

PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	XPLMMenuID	id;
	int			item;

	strcpy(outName, "Control");
	strcpy(outSig, "xpsdk.examples.Control");
	strcpy(outDesc, "A plug-in that allows Controling of lat/lon etc.");

	item = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Control", NULL, 1);

	id = XPLMCreateMenu("Control", XPLMFindPluginsMenu(), item, ControlMenuHandler, NULL);
	XPLMAppendMenuItem(id, "Control", (void *)"Control", 1);
	
	MenuItem1 = 0;

	for (int Item=0; Item<MAX_ITEMS; Item++)
		gControlDataRef[Item] = XPLMFindDataRef(DataRefString[Item]);

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	if (MenuItem1 == 1)
	{
		XPDestroyWidget(ControlWidget, 1);
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
	if (inFrom == XPLM_PLUGIN_XPLANE)
	{
		switch(inMsg)
		{
			case XPLM_MSG_PLANE_LOADED:
				RefreshValues();
				break;
		}
	}
}

void ControlMenuHandler(void * mRef, void * iRef)
{
	if (!strcmp((char *) iRef, "Control"))
	{
		if (MenuItem1 == 0)
		{
			CreateControl(300, 550, 350, 530);
			MenuItem1 = 1;
		}
		else
		{
			if(!XPIsWidgetVisible(ControlWidget))
			{
				RefreshValues();
				XPShowWidget(ControlWidget);
	}
}						
	}
}						


void CreateControl(int x, int y, int w, int h)
{
	int x2 = x + w;
	int y2 = y - h;
	char buffer[512];
	float FloatValue[MAX_ITEMS];
	int Item;

	for (Item=0; Item<MAX_ITEMS; Item++)
		FloatValue[Item] = XPLMGetDataf(gControlDataRef[Item]);
	
	ControlWidget = XPCreateWidget(x, y, x2, y2,
					1,	// Visible
					"Control",	// desc
					1,		// root
					NULL,	// no container
					xpWidgetClass_MainWindow);

	XPSetWidgetProperty(ControlWidget, xpProperty_MainWindowHasCloseBoxes, 1);

	ControlWindow = XPCreateWidget(x+50, y-50, x2-50, y2+50,
					1,	// Visible
					"",	// desc
					0,		// root
					ControlWidget,
					xpWidgetClass_SubWindow);

	XPSetWidgetProperty(ControlWindow, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);

	for (Item=0; Item<MAX_ITEMS; Item++)
	{
		ControlText[Item] = XPCreateWidget(x+60, y-(70 + (Item*30)), x+115, y-(92 + (Item*30)),
							1,	// Visible
							DataRefDesc[Item],// desc
							0,		// root
							ControlWidget,
							xpWidgetClass_Caption);

		sprintf(buffer, "%f", HACKFLOAT(FloatValue[Item]));
		ControlEdit[Item] = XPCreateWidget(x+160, y-(70 + (Item*30)), x+250, y-(92 + (Item*30)),
							1, buffer, 0, ControlWidget,
							xpWidgetClass_TextField);

		XPSetWidgetProperty(ControlEdit[Item], xpProperty_TextFieldType, xpTextEntryField);

		UpArrow[Item] = XPCreateWidget(x+252, y-(66 + (Item*30)), x+264, y-(81 + (Item*30)),
							1, "", 0, ControlWidget,
							xpWidgetClass_Button);

		XPSetWidgetProperty(UpArrow[Item], xpProperty_ButtonType, xpLittleUpArrow);

		DownArrow[Item] = XPCreateWidget(x+252, y-(81 + (Item*30)), x+264, y-(96 + (Item*30)),
							1, "", 0, ControlWidget,
							xpWidgetClass_Button);
	
		XPSetWidgetProperty(DownArrow[Item], xpProperty_ButtonType, xpLittleDownArrow);
	}

	ControlApplyButton = XPCreateWidget(x+120, y-440, x+210, y-462,
					1, "Apply Data", 0, ControlWidget, 
					xpWidgetClass_Button);

	XPSetWidgetProperty(ControlApplyButton, xpProperty_ButtonType, xpPushButton);

	XPAddWidgetCallback(ControlWidget, ControlHandler);
}

int	ControlHandler(
						XPWidgetMessage			inMessage,
						XPWidgetID				inWidget,
						intptr_t				inParam1,
						intptr_t				inParam2)
{
	char buffer[512];
	float FloatValue[MAX_ITEMS];
	int Item;

	for (Item=0; Item<MAX_ITEMS; Item++)
		FloatValue[Item] = XPLMGetDataf(gControlDataRef[Item]);

	if (inMessage == xpMessage_CloseButtonPushed)
	{
		if (MenuItem1 == 1)
		{
			XPHideWidget(ControlWidget);
		}
		return 1;
	}

	if (inMessage == xpMsg_PushButtonPressed)
	{

		if (inParam1 == (intptr_t)ControlApplyButton)
		{
			ApplyValues();
			return 1;
		}

		for (Item=0; Item<MAX_ITEMS; Item++)
		{
			if (inParam1 == (intptr_t)UpArrow[Item])
			{
				FloatValue[Item] += IncrementValue[Item];
				sprintf(buffer, "%f", HACKFLOAT(FloatValue[Item]));
				XPSetWidgetDescriptor(ControlEdit[Item], buffer);
				XPLMSetDataf(gControlDataRef[Item], FloatValue[Item]);
				return 1;
			}
		}

		for (Item=0; Item<MAX_ITEMS; Item++)
		{
			if (inParam1 == (intptr_t)DownArrow[Item])
			{
				FloatValue[Item] -= IncrementValue[Item];
				sprintf(buffer, "%f", HACKFLOAT(FloatValue[Item]));
				XPSetWidgetDescriptor(ControlEdit[Item], buffer);
				XPLMSetDataf(gControlDataRef[Item], FloatValue[Item]);
				return 1;
			}
		}
	}

	return 0;
}						

void ApplyValues(void)
{
	char	buffer[512];

	for (int Item=0; Item<MAX_ITEMS; Item++)
	{
		XPGetWidgetDescriptor(ControlEdit[Item], buffer, 512);
		XPLMSetDataf(gControlDataRef[Item], atof(buffer));
	}
}

void RefreshValues(void)
{
	char	buffer[512];
	float FloatValue[MAX_ITEMS];
	int Item;

	for (Item=0; Item<MAX_ITEMS; Item++)
		FloatValue[Item] = XPLMGetDataf(gControlDataRef[Item]);
	
	for (Item=0; Item<MAX_ITEMS; Item++)
	{
		sprintf(buffer, "%f", HACKFLOAT(FloatValue[Item]));
		XPSetWidgetDescriptor(ControlEdit[Item], buffer);
	}
}

