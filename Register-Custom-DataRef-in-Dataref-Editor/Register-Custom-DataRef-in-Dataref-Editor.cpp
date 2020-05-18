// Downloaded from https://developer.x-plane.com/code-sample/register-custom-dataref-in-dataref-editor/


// Custom Commands

 // Register Custom DataRefs in DataRefEditor.  
 //
 // This plugin adds a few lines of code and a flight loop callback to the Custom Commands Control 
 // Custom DataRef example.  The flight loop callback sends the message to DataRefEditor to 
 // register your custom dataref in DataRefEditor.  Returning 0 from the flight loop sends the message
 // only once.  A similar flight loop is required for each of you custom datarefs.  
 // 
 // Content added by BlueSideUpBob.
 
 #define XPLM200 = 1;  // This example requires SDK2.0
 
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
 
 #define MSG_ADD_DATAREF 0x01000000           //  Add dataref to DRE message
 
 XPLMDataRef gCounterDataRef = NULL;          //  Our custom dataref
 
 int     gCounterValue;                       //  Our custom dataref's value  
 float   RegCounterDataRefInDRE(float elapsedMe, float elapsedSim, int counter, void * refcon);  //  Declare callback to register dataref
 int     GetCounterDataRefCB(void* inRefcon);
 void    SetCounterDataRefCB(void* inRefcon, int outValue);
 
 XPLMCommandRef CounterUpCommand = NULL;	//  Our two custom commands
 XPLMCommandRef CounterDownCommand = NULL;
 
 int    CounterUpCommandHandler(XPLMCommandRef       inCommand,          //  Our two custom command handlers
                                XPLMCommandPhase     inPhase,
                                void *               inRefcon);
 
 int    CounterDownCommandHandler(XPLMCommandRef     inCommand,
                                 XPLMCommandPhase    inPhase,
                                 void *              inRefcon);
 
 PLUGIN_API int XPluginStart(
         char *        outName,
         char *        outSig,
         char *        outDesc)
 {
 
 // Plugin Info
     strcpy(outName, "CustomCommandsAndDataRefs");
     strcpy(outSig, "BlueSideUpBob.Example.CustomCommandsAndDataRef");
     strcpy(outDesc, "This example illustrates creating and using custom commands to control a custom DataRef.");
 
 //  Create our custom integer dataref
 gCounterDataRef = XPLMRegisterDataAccessor("BSUB/CounterDataRef",
                                             xplmType_Int,                                  // The types we support
                                             1,                                             // Writable
                                             GetCounterDataRefCB, SetCounterDataRefCB,      // Integer accessors
                                             NULL, NULL,                                    // Float accessors
                                             NULL, NULL,                                    // Doubles accessors
                                             NULL, NULL,                                    // Int array accessors
                                             NULL, NULL,                                    // Float array accessors
                                             NULL, NULL,                                    // Raw data accessors
                                             NULL, NULL);                                   // Refcons not used
 
 
 // Find and intialize our Counter dataref
 gCounterDataRef = XPLMFindDataRef ("BSUB/CounterDataRef");
 XPLMSetDatai(gCounterDataRef, 0);
 
 XPLMRegisterFlightLoopCallback(RegCounterDataRefInDRE, 1, NULL);   // This FLCB will register our custom dataref in DRE
 
 // Create our commands; these will increment and decrement our custom dataref.
 CounterUpCommand = XPLMCreateCommand("BSUB/CounterUpCommand", "Counter Up");
 CounterDownCommand = XPLMCreateCommand("BSUB/CounterDownCommand", "Counter Down");
 
 // Register our custom commands
 XPLMRegisterCommandHandler(CounterUpCommand,           // in Command name
                            CounterUpCommandHandler,    // in Handler
                            1,                          // Receive input before plugin windows.
                            (void *) 0);                // inRefcon.
 
 XPLMRegisterCommandHandler(CounterDownCommand,
                            CounterDownCommandHandler,
                            1,
                            (void *) 0);
 
 return 1;
 }
 
  
 PLUGIN_API void     XPluginStop(void)
 { 
 XPLMUnregisterDataAccessor(gCounterDataRef);
 XPLMUnregisterCommandHandler(CounterUpCommand, CounterUpCommandHandler, 0, 0);
 XPLMUnregisterCommandHandler(CounterDownCommand, CounterDownCommandHandler, 0, 0);
 XPLMUnregisterFlightLoopCallback(RegCounterDataRefInDRE, NULL);	 //  Don't forget to unload this callback.  
 }
  
 PLUGIN_API void XPluginDisable(void)
 {
 }
 
 PLUGIN_API int XPluginEnable(void)
 {
     return 1;
 }
 
 PLUGIN_API void XPluginReceiveMessage(XPLMPluginID    inFromWho,
                                      long             inMessage,
                                      void *           inParam)
 {
 }
 
 int     GetCounterDataRefCB(void* inRefcon)
 {
     return gCounterValue;
 }
 
 void	SetCounterDataRefCB(void* inRefcon, int inValue)
 {
      gCounterValue = inValue;
 }
 
 int    CounterUpCommandHandler(XPLMCommandRef       inCommand,
                                XPLMCommandPhase     inPhase,
                                void *               inRefcon)
 {
 //  If inPhase == 0 the command is executed once on button down.
 if (inPhase == 0)
     {
      gCounterValue++;
      if(gCounterValue > 10) {gCounterValue = 10;}
     }
 // Return 1 to pass the command to plugin windows and X-Plane.
 // Returning 0 disables further processing by X-Plane.
  
 return 0;
 }
 
 int    CounterDownCommandHandler(XPLMCommandRef       inCommand,
                         XPLMCommandPhase     inPhase,
                         void *               inRefcon)
 {
     if (inPhase == 1)
     {
           gCounterValue--;
           if(gCounterValue < -10) {gCounterValue = -10;}
     }
 
 return 0;
 }
  
 //  This single shot FLCB registers our custom dataref in DRE
 float RegCounterDataRefInDRE(float elapsedMe, float elapsedSim, int counter, void * refcon)
 {
     XPLMPluginID PluginID = XPLMFindPluginBySignature("xplanesdk.examples.DataRefEditor");
     if (PluginID != XPLM_NO_PLUGIN_ID)
     {
          XPLMSendMessageToPlugin(PluginID, MSG_ADD_DATAREF, (void*)"BSUB/CounterDataRef");   
     }
 
     return 0;  // Flight loop is called only once!  
 }

