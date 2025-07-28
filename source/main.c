#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <ogc/pad.h>
#include <wiiuse/wpad.h>
#include <unistd.h>

// Local Files
#include "ios.h"
#include "globals.h"
#include "parental.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void return_to_loader(void)
{
	printf("\n\nExiting...");
	exit(0);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
    IOS_ReloadIOS(IOS_GetVersion());
    disable_ahbprot(); // Must be done after an IOS reload

	// Initialise the video system
	VIDEO_Init();

	// These functions initialises the attached controllers
    PAD_Init();
	WPAD_Init();

	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	// Initialise the console, required for printf
	console_init(xfb,20,20,rmode->fbWidth-20,rmode->xfbHeight-20,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	//SYS_STDIO_Report(true);

	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);

	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);

	// Clear the framebuffer
	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);

	// Make the display visible
	VIDEO_SetBlack(false);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();


	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf ("\x1b[%d;%dH", row, column );
	printf("\x1b[2;0H");

    //---------------------------------------------------------------------------------
    printf("Wii Parental Controls Remover %s\nCreated by Aep\n", VERSION);
	printf("Press HOME (or START on GameCube Controller) to exit.\n\n");

    if (SYSCONF_Init() != 0) {
        printf("\nFailed to initialize SYSCONF! Is the file on the NAND?");
		sleep(5);
		return_to_loader();
	}

	bool parental_controls_enabled = parental_enabled();
	printf("Parental Controls Enabled: %s\n", parental_controls_enabled ? "Yes" : "No");

	if (!parental_controls_enabled) {
		printf("\nYour Wii does not have parental controls enabled.");
	}
	else
	{
		printf("Press +/A to disable parental controls!\n");
	}

    //---------------------------------------------------------------------------------

	while(1) {

		// Call WPAD_ScanPads each loop, this reads the latest controller states
        PAD_ScanPads();
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);
        u32 pressed_gc = PAD_ButtonsDown(0);

		if (((pressed & WPAD_BUTTON_PLUS) || (pressed_gc & PAD_BUTTON_A)) && parental_controls_enabled)
		{
			printf("Disabling parental controls...\n");
			parental_controls_enabled = false;
			set_parental(0x00); // 0x00: off, 0x80: on
			printf("Successfully disabled parental controls!\n");
		}

		// We return to the launcher application via exit
		if ( pressed & WPAD_BUTTON_HOME || pressed_gc & PAD_BUTTON_START ) return_to_loader();

		// Wait for the next frame
		VIDEO_WaitVSync();
	}

	return 0;
}