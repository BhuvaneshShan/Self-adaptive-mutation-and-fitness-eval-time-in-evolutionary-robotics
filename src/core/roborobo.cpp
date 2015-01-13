// niko - contact: niko(at)factorycity.com
// started from "Oeil De Mars - level 1" (fork from rc6 release)
//
// http://factorycity.com
// niko.2008-2010 (started 20081216)
// RC2: 20090123.1848
// current version: (see gVersion value)
//

//
// personnal notes: 
// - keyboard layout bug due targetting ppc on my macbook
//
// Ressources:
//  http://lazyfoo.net/SDL_tutorials/
//  http://www.cs.clemson.edu/~malloy/courses/3dgames-2007/tutor/web/audio/audio.html
//  http://cs.unc.edu/~mcmillan/comp136/Lecture6/Lines.html
//
// Assumptions
//  max speed <= agent size (mandatory to avoid missing collisions)
//  captions and floorsensor uses only red component of storyzones image
//  when using zones, prefer no. 1 to 99 (0 is no zone, 100+ is conditional activation zones - see code)
//  gEnvironmentImage contains information about empty spaces (reserved color=0), environment objects (reserved colors=1..99) and agent object (reserved colors=100..max)
//		Agents are registered in gEnvironmentImage using agentId+100 (ie. agent no.0 is identified as gEnvironmentImage.pixelValue=100)
//		Assume: NO agent EVER overwrite non-blank pixels in gEnvironmentImage (justification: unregistering an agent simply empty the zone without any checking)
//		agent range sensors uses red/green/blue component of gEnvironmentImage, such as value=r<<16+g<<8+b (ie. everything from the integer value except the alpha component)
//  "Story zones" (zone perceived through floor sensor)
//		defined in the storyzone image. Only the red component is used.
//		3 classes of zones: 
//			001-099: zones are perceived whenever the agent is on it.
//			100-199: decimal are key stories, non-decimal are perceived only if zone at start of decimal has already been perceived. (e.g. 154 can be seen only if 150 has already been visited) 
//			200-255: decimal are key stories, non-decimal are perceived only if previous one has already been perceived. (e.g. 234 can be seen only if 230-233 has already been visited)
//		key stories (first story stories of each decimal) must be visited prior to story in the decimal to be displayed (eg. story11 is displayed only if story10 has been displayed -- exception if story10 does not exist) 

//	...
//
//
// 

#include <cstring>

#include "RoboroboMain/roborobo.h"

// Project headers

//#include "common.h"
#include "Utilities/SDL_gfxRoborobo.h"
#include "World/World.h"
#include "World/DefaultEnergyDistribution.h"
#include "Agents/Agent.h"
#include "Agents/RobotAgent.h"
#include "Agents/InspectorAgent.h"
#include "Utilities/Timer.h"
#include "Utilities/Misc.h"  // contains convertFromString(...)

#include "Config/GlobalConfigurationLoader.h"

/* **************************
 * * global data definition *			// dont forget to modify header file if you add other global data
 * **************************/

// =-=-=-=-=

long int gVersion = 20100629;
std::string gLogCommentText = "(under development)"; // 

std::string gStartTime = getCurrentTimeAsReadableString();

std::string gCompileTime = __TIME__;
std::string gCompileDate = __DATE__;


std::string gPropertiesFilename = "config/default.properties";
std::string gCurrentBuildInfo = "Krakow build"; // display through "-v[ersion]" command line option -- check/set value in config.h
// Bristol build -- june 2010
// Krakow build -- sept 2010


ExtendedProperties gProperties;

bool gVideoRecording = false;


//filenames

std::string gLogFilename = "logs/datalog.txt";
std::ofstream gLogFile;

std::string gAgentMaskImageFilename = "data/agent-mask.png";
std::string gAgentSpecsImageFilename = "data/agent-specs.png";
std::string gForegroundImageFilename = "data/foreground.png"; // MANDATORY: use png (jpg approximate encoding may lead to "invisible" obstacle settings (ie. color close to but not equal to #FFFFFF)
std::string gEnvironmentImageFilename = "data/environment.png";
std::string gBackgroundImageFilename = "data/background.png";
std::string gZoneImageFilename = "data/storyzones.png";
std::string gZoneCaptionPrefixFilename = "data/story";
std::string gEnergyDensityImageFilename = "";

//general purpose

int gRandomSeed = -1; // (default value should be "-1" => time-based random seed)

bool gVerbose = true;
bool gBatchMode = false;

int gScreenBPP = 32; // bits per pixel color (32 bits)
int gFramesPerSecond = 60; // frame rate

int gParallaxFactor = 1; //3

ConfigurationLoader *gConfigurationLoader = NULL;

//general data

bool gRadioNetwork = false; // default is false -- see header for information about computational cost.
//int  *gRadioNetworkArray;	// initialized in gWorld->initWorld();
std::vector<std::vector<int> > gRadioNetworkArray; // initialized in gWorld->initWorld();
int gMaxRadioDistanceToSquare = 1024; // arbitrary value -- note that it is internaly stored as d^2 (ie. 1024=32pixels)

bool gDisplayZoneCaption = false;
bool gDisplaySensors = true;

bool gPauseMode = false;
bool gInspectorMode = false; // manual control mode -- false: agent-centered ; true: envt centered (ie. if inspector agent exists)
bool gInspectorAgent = false; // is there an inspector agent? 

int gMaxIt = 0; // note: value *must* be defined in the properties file. ("-1" (ie. infinite) is a nice default setting).

bool gEnergyPoints_alwaysRender = false;
bool gEnergyMode = false;
int gMaxEnergyPoints = 0;
float gEnergyPointRadius = 0.0;
float gEnergyPointValue = 0.0;
int gEnergyPointRespawnLagMaxValue = 0;
std::vector<EnergyPoint> gEnergyPoints;
EnergyDistribution *gEnergyDistribution = new DefaultEnergyDistribution();

Uint32 gEnergyPointColor = 0xeab71fff;
bool gShowOrientationMarker = true;
int gOrientationMarkerRadius = 2;
Uint32 gOrientationMarkerColor = 0xff0000ff;
Uint32 gSensorRayFreeColor = 0x0000ffff;
Uint32 gSensorRayObstacleColor = 0xff0000ff;
Uint32 gSensorRayEnergyColor = 0x00ff00ff;

int gAgentCounter = 0;
int gAgentIndexFocus = 0;

//Screen attributes (800x600)
int gScreenWidth = 0;
int gScreenHeight = 0;

//The agent dimensions
int gAgentWidth = 0;
int gAgentHeight = 0;

int gMoveStepWidth = 1;
int gMoveStepHeight = 1;

int gInspectorAgentXStart = 100; // arbitrary...
int gInspectorAgentYStart = 355;

// agent dynamics and structure

int gNbOfAgents = 2;

float gMaxTranslationalSpeed = 2; // pixels/sec. (presets: 20, 5, ...)
float gMaxTranslationalDeltaValue = gMaxTranslationalSpeed; // maximum delta update value btw current and desired speed.
float gMaxRotationalSpeed = 20; // degrees/sec. (presets: 20, 50, ...)
int gSensorRange = 64;
bool gEnableDitchSensors = true;

int gMaxSpeedOnXaxis = 20; // "god" inspector mode max speed on both axis ("G" key to activate)
int gMaxSpeedOnYaxis = 10;

int gLocomotionMode = 0; // cf. header file

bool gInspectAgent = false;

// =-=-=-=-=

bool gKeyUp = false, gKeyDown = false, gKeyRight = false, gKeyLeft = false;
bool gFullScreenMode = false;
bool gSlowMotionMode = false;



bool gAgentRegistration = true; // agent "physical" state: [false]: ghost (ie. agents dont see with each other) [true]: physically exists (but optimized -- ie. visible only if they can be seen with non null probability)

bool gNiceRendering = true;

int gDisplayMode = 0;
int gFastDisplayModeSpeed = 60; //500;

bool gUserCommandMode = false;

//dimensions of the level
int gAreaWidth = 0;
int gAreaHeight = 0;

//camera
SDL_Rect gCamera;
SDL_Rect gCameraParallax;

//image surfaces

SDL_Surface *gScreen = NULL;
SDL_Surface *gForegroundImage = NULL;
SDL_Surface *gEnvironmentImage = NULL;
SDL_Surface *gBackgroundImage = NULL;
SDL_Surface *gAgentMaskImage = NULL;
SDL_Surface *gAgentSpecsImage = NULL;

//stories
//  note: key stories (first story stories of each decimal) must be visited prior to story in the decimal to be displayed (eg. story11 is displayed only if story10 has been displayed -- exception if story10 does not exist) 
//  lexicon: key stories (e.g. 0, 10, 20, 100) and sub-stories (e.g. 11, 144, 1, 255)
SDL_Surface *gZoneImage = NULL;
SDL_Surface *gZoneCaptionImage[256]; // all stories. total depends on max color component value (ie. defined in 0...255)
bool gZoneStatus[256]; // told stories

SDL_Surface *gEnergyImage = NULL;

// SDL event handler
SDL_Event gEvent; //The event structure


// special simulation data

InspectorAgent *inspectorAgent = NULL;
World *gWorld = NULL;
Timer fps;
int timetag = -1;
Timer timeWatch;

// Organism related globals
bool gUseOrganisms = false;
bool gRiverbed = false;
int gSpeedScale = NEUTRAL;
int gStreamDirection = 0;
double gStreamStrength = 1;
int gRiverbedZone = -1;
std::string gAgentPositiveMaskImageFilename = "data/agent-positive-mask.png";
std::string gAgentNegativeMaskImageFilename = "data/agent-negative-mask.png";
std::string gOrganismLogFilename = "logs/organisms.log";
std::string gOrganismSizesLogFilename = "logs/organism_sizes.log";
SDL_Surface *gAgentPositiveMaskImage = NULL;
SDL_Surface *gAgentNegativeMaskImage = NULL;
int gConnectionGap;
int gOrganismSampleFrequency = 250;
bool gRenderRobotId = false;
bool gShowAgentEnergyPoints = false;
std::string gFontName = "/usr/share/fonts/truetype/ttf-bitstream-vera/Vera.ttf";

int gSpawnWidth = gAreaWidth; // Maximum x coordinate where a robot can be placed initially
int gSpawnHeight = gAreaHeight; // Maximum y coordinate where a robot can be placed initially


/* ********************
 * * global functions *
 * ********************/

void clean_up() {
	//Free the surfaces

	SDL_FreeSurface(gAgentMaskImage);
	if(gUseOrganisms){
		SDL_FreeSurface(gAgentPositiveMaskImage);
		SDL_FreeSurface(gAgentNegativeMaskImage);
	}
	SDL_FreeSurface(gForegroundImage);
	SDL_FreeSurface(gEnvironmentImage);
	if (gBackgroundImage != NULL)
		SDL_FreeSurface(gBackgroundImage);
	SDL_FreeSurface(gZoneImage);
    SDL_FreeSurface(gEnergyImage);
	for (int i = 0; i != 256; i++)
		if (gZoneCaptionImage[i] != NULL) SDL_FreeSurface(gZoneCaptionImage[i]);

	if (inspectorAgent != NULL) delete(inspectorAgent);

	if (gUseOrganisms) {
		Organism::destroy();
	}

	if(gWorld != NULL) delete gWorld;
	if(gConfigurationLoader != NULL) delete gConfigurationLoader;


	//Quit SDL
	//SDL_Quit();
}

void displayHelp() {
	//if ( gVerbose && !gBatchMode)
	if (!gBatchMode) {
		std::cout << "\n=-=-=-=-=-=-=-=-=-=-=-= roborobo! =-=-=-=-=-=-=-=-=-=-=-=\n";
		std::cout << "Fast 2D autonomous agent simulator for large population of robots/agents\nnicolas.bredeche(at)gmail.com\n";
		std::cout << "Official version tag    : " << gVersion << std::endl;
		std::cout << "Current build name      : " << gCurrentBuildInfo << std::endl << std::endl;
		std::cout << "Compilation version tag : " << gCompileDate << " - " << gCompileTime << std::endl;

		std::cout << " >>>> Keys:" << std::endl;
		std::cout << "       h : help! (ie. this text)" << std::endl;
		std::cout << "       r : reset agents/environment" << std::endl;
		std::cout << "       c : agent-agent collision on/off" << std::endl;
		std::cout << "       n : radio network communication on/off" << std::endl;
		std::cout << "       d : set display mode - (1) default-60-fps; (2) fast; (3) fastest-no-display. (shift+d: inverse)" << std::endl;
		std::cout << "       v : verbose on/off (console)" << std::endl;
		std::cout << "       p : pause/freeze simulation (display mode)" << std::endl;
		std::cout << "       s : slow mode on/off (switch it off if display is off)" << std::endl;
		std::cout << "       x : (\"X-ray mode\") debug display mode on/off" << std::endl;
		std::cout << "       z : display zone caption on/off (display mode)" << std::endl;
		std::cout << "       g : (\"god mode\") switch agent/inspector mode - follow selected agent or move freely (display mode)" << std::endl;
		std::cout << "       i : inspect currently selected agent on/off (display sensor and motor values on console)" << std::endl;
		std::cout << "       j : show / dont show sensor raycasting and visual markers (display mode)" << std::endl;
		std::cout << "       o : take screenshot - save image to disk (in ./logs directory) (display mode)" << std::endl;
		std::cout << "       O : video recording start/stop - save images to disk (in ./logs directory) (display mode)" << std::endl;
		std::cout << "   <tab> : switch focus to next agent (shift+o: previous agent)" << std::endl;
		std::cout << " <enter> : (in agent following mode) trigger manual agent control mode" << std::endl;
		std::cout << " <enter> : (in inspector mode) sample and display sensor values at current location" << std::endl;
		std::cout << "   <esc> : quit" << std::endl;
		std::cout << "=-=-=-=-=-=-=-=-=-=-=-= roborobo! =-=-=-=-=-=-=-=-=-=-=-=\n\n";
	}
}

/* ******** *
 *   MAIN   *
 * ******** */

bool checkQuitEvent() {
	bool quit = false;
	//While there's events to handle
	while (SDL_PollEvent(&gEvent)) {
		//If the user has Xed out the window
		if (gEvent.type == SDL_QUIT) {
			//Quit the program
			quit = true;
			break;
		}
	}

	return quit;
}




// return true if quit.

bool handleKeyEvent(Uint8 *keyboardStates) {
	//Handle events for the dot  -- !n: must be out of the SDL_PollEvent loop if this method is used (see definition).
	// get keybord states

	if (keyboardStates[ SDLK_ESCAPE ]) {
		if (gVerbose)
			std::cout << "Terminating!" << std::endl;

		SDL_Delay(PAUSE_COMMAND);
		return true;
	}

	if (keyboardStates[ SDLK_d ]) {
		if (gSlowMotionMode)
			gSlowMotionMode = false;
		if (keyboardStates[ SDLK_RSHIFT ] || keyboardStates[ SDLK_LSHIFT ])
			gDisplayMode = (gDisplayMode + 2) % 3;
		else
			gDisplayMode = (gDisplayMode + 1) % 3;

		if (gVerbose)
			std::cout << "Display mode is now " << gDisplayMode << std::endl;

		SDL_Delay(PAUSE_COMMAND);
	}

	if (keyboardStates[ SDLK_r ]) {
		if (gVerbose)
			std::cout << "Reset!" << std::endl;

		gWorld->resetWorld();
		SDL_Delay(PAUSE_COMMAND);
	}

	if (gDisplayMode == 0 || gDisplayMode == 1) {
		if (keyboardStates[ SDLK_p ]) {
			SDL_Delay(PAUSE_COMMAND); // 200ms delay
			gPauseMode = !gPauseMode;
			if (gVerbose) {
				if (gPauseMode)
					std::cout << "pause mode is ON. (at " << gWorld->getIterations() << " steps)" << std::endl;
				else
					std::cout << "pause mode is OFF." << std::endl;
			}
		}

		if (keyboardStates[ SDLK_h ]) {
			displayHelp();
			SDL_Delay(PAUSE_COMMAND); // 200ms delay
		}

		if (keyboardStates[ SDLK_o ]) // screenshot and movie recording
		{
			if (keyboardStates[ SDLK_RSHIFT ] || keyboardStates[ SDLK_LSHIFT ]) {
				// start/stop movie recording

				gVideoRecording = !gVideoRecording;

				if (gVerbose) {
					if (gVideoRecording)
						std::cout << "Starting video recording." << std::endl;
					else
						std::cout << "Stopping video recording." << std::endl;
				}
			} else {
				// save screenshot

				saveScreenshot();

				if (gVerbose)
					std::cout << "Screenshot saved." << std::endl;
			}

			SDL_Delay(PAUSE_COMMAND); // 200ms delay
		}

		if (keyboardStates[ SDLK_v ]) {
			gVerbose = !gVerbose;

			if (gVerbose)
				std::cout << "Verbose mode is now ON." << std::endl;
			else
				std::cout << "Verbose mode is now OFF." << std::endl;

			SDL_Delay(PAUSE_COMMAND); // 200ms delay
		}

		if (keyboardStates[ SDLK_g ]) {
			gInspectorMode = !gInspectorMode;
			if (gInspectorMode == true && gUserCommandMode == true)
				gUserCommandMode = false;

			if (gVerbose) {
				if (gInspectorMode)
					std::cout << "Inspector agent is now active." << std::endl;
				else
					std::cout << "Inspector agent is now inactive." << std::endl;
			}

			SDL_Delay(PAUSE_COMMAND); // 200ms delay
		}

		if (keyboardStates[ SDLK_RETURN ]) {
			if (gInspectorMode == false) // (1) on/off take command of the robot OR (2) display id at inspector target
			{
				gUserCommandMode = !gUserCommandMode;
				if (gVerbose) {
					if (gUserCommandMode)
						std::cout << "Agent #" << gAgentIndexFocus << " is now remote-controlled." << std::endl;
					else
						std::cout << "Agent #" << gAgentIndexFocus << " is now autonomous." << std::endl;
				}
			} else {
				if (gVerbose) {
					// * inspector mode. Return key trigger sensor display. (note: non-collision enabled robot cannot be seen)

					// inspector virtual sensors values
					std::cout << "## Inspector Agent ##" << std::endl; //"Inspector virtual sensors:"

					int xTmp, yTmp;
					inspectorAgent->getCoord(xTmp, yTmp);

					// location
					std::cout << "\tcoordinates: (" << xTmp << "," << yTmp << ")" << std::endl;

					// virtual range sensor
					Uint32 inspectorSensorValue = getPixel32(gEnvironmentImage, xTmp, yTmp);
					Uint8 r, g, b;
					SDL_GetRGB(inspectorSensorValue, gEnvironmentImage->format, &r, &g, &b);
					inspectorSensorValue = (r << 16)+(g << 8) + b;
					std::cout << "\tvirtual range sensor: ";
					if (inspectorSensorValue == 0xFFFFFF) // either nothing or unregistered agent(s).
					{
						std::cout << "0xFFFFFF (ie. either nothing or unidentified robot)" << std::endl;

						// Agents may not be visible in the internal scene buffer due to optimization
						// Hence, we scan the list of agents to compare actual inspector location and agent location
						// Results from this scan should be interpreted as a list of either
						//  - nearby agents (possibly registered, or not)
						//  - agent precisely at this location, but not registered
						// note: registering Agent in the internal scene buffer is not mandatory if it is sure
						//       that it is not within the perceptual range of any other agents (speed up simulation).

						int radiusMax = gAgentWidth > gAgentHeight ? (gAgentWidth + 1) / 2 : (gAgentHeight + 1) / 2; // assume an upper bound for dimension.
						for (int i = 0; i != gAgentCounter; i++) // test for agents proximity based on localization 
						{
							int x = (int) (gWorld->getAgent(i)->getWorldModel()->getXReal());
							int y = (int) (gWorld->getAgent(i)->getWorldModel()->getYReal());
							if (abs(x - xTmp) < radiusMax && abs(y - yTmp) < radiusMax)
								std::cout << "\t\tAgent #" << i << " detected (closeby and/or unregistered)." << std::endl;
						}
					} else {
						std::cout << inspectorSensorValue;
						if (inspectorSensorValue >= 1000)
							std::cout << " (should be agent #" << inspectorSensorValue - 1000 << ")";
						std::cout << std::endl;
					}

					// virtual floor sensor
					inspectorSensorValue = getPixel32(gZoneImage, xTmp, yTmp);
					SDL_GetRGB(inspectorSensorValue, gZoneImage->format, &r, &g, &b);
					inspectorSensorValue = r;
					std::cout << "\tvirtual floor sensor: " << inspectorSensorValue << std::endl;
				}
			}
			SDL_Delay(PAUSE_COMMAND); // 200ms delay
		}

		//if (keyboardStates[ SDLK_f ])  -- not implemented
		//	toggle_fullscreen();

		if (keyboardStates[ SDLK_c ]) // agent-agent collision?
		{
			if (gAgentRegistration)
				for (int i = 0; i != gAgentCounter; i++)
					(gWorld->getAgent(i))->unregisterAgent();
			gAgentRegistration = !gAgentRegistration;

			if (gVerbose) {
				if (gAgentRegistration)
					std::cout << "Agents are \"registered\" (visible+tangible) [DEFAULT]." << std::endl;
				else
					std::cout << "Agents are *not* \"registered\" (invisible+intangible) [fastest, unrealistic]." << std::endl;
			}

			SDL_Delay(PAUSE_COMMAND); // 200ms delay
		}

		if (keyboardStates[ SDLK_x ]) {
			gNiceRendering = !gNiceRendering;

			if (gVerbose) {
				if (gNiceRendering)
					std::cout << "Render mode is \"user-compliant\" (ie. nice but possibly wrong rendering)." << std::endl;
				else
					std::cout << "Render mode is \"x-ray\" mode (ie. world as seen by robot(s))." << std::endl;
			}

			SDL_Delay(PAUSE_COMMAND); // 200ms delay
		}

		if (keyboardStates[ SDLK_z ]) {
			gDisplayZoneCaption = !gDisplayZoneCaption;

			if (gVerbose) {
				if (gDisplayZoneCaption)
					std::cout << "Zone information is displayed (if caption available)." << std::endl;
				else
					std::cout << "Zone information is never displayed." << std::endl;
			}

			SDL_Delay(PAUSE_COMMAND); // 200ms delay
		}

		if (keyboardStates[ SDLK_j ]) {
			gDisplaySensors = !gDisplaySensors;

			if (gVerbose) {
				if (gDisplaySensors)
					std::cout << "Sensor rays are shown." << std::endl;
				else
					std::cout << "Sensor rays are invisible." << std::endl;
			}

			SDL_Delay(PAUSE_COMMAND); // 200ms delay
		}

		if (keyboardStates[ SDLK_TAB ]) {
			if (keyboardStates[ SDLK_RSHIFT ] || keyboardStates[ SDLK_LSHIFT ])
				gAgentIndexFocus = ((gAgentIndexFocus - 1) + gAgentCounter) % gAgentCounter;
			else
				gAgentIndexFocus = (gAgentIndexFocus + 1) % gAgentCounter;

			if (gVerbose)
				std::cout << "Agent #" << gAgentIndexFocus << " is selected." << std::endl;

			SDL_Delay(PAUSE_COMMAND); // 200ms delay
		}

		if (keyboardStates[ SDLK_i ]) {
			if (gVerbose)
				std::cout << "Inspect agent #" << gAgentIndexFocus << "." << std::endl;
			gInspectAgent = !gInspectAgent;
			SDL_Delay(PAUSE_COMMAND);
		}

		if (keyboardStates[ SDLK_s ]) {
			gSlowMotionMode = !gSlowMotionMode;
			if (gSlowMotionMode)
				gDisplayMode = 0;

			if (gVerbose) {
				if (gSlowMotionMode)
					std::cout << "Slow motion is ON." << std::endl;
				else
					std::cout << "Slow motion is off." << std::endl;
			}

			SDL_Delay(PAUSE_COMMAND);
		}

		if (keyboardStates[ SDLK_n ]) {
			gRadioNetwork = !gRadioNetwork;

			if (gVerbose) {
				if (gRadioNetwork)
					std::cout << "Radio network is active (note: may slow down simulation, use with care)." << std::endl;
				else
					std::cout << "Radio network is inactive." << std::endl;
			}

			SDL_Delay(PAUSE_COMMAND); // 200ms delay
		}


		if (gSlowMotionMode)
			SDL_Delay(100);

	}/**/

	return false;
}

void updateDisplay() {
	//if ( world->getIterations() % 10 == 0 ) 
	if (gDisplayMode == 0 || (gDisplayMode == 1 && gWorld->getIterations() % gFastDisplayModeSpeed == 0)) {
		//Set the camera to either focused agent or inspector virtual location 
		if (gInspectorMode)
			inspectorAgent->set_camera();
		else
			gWorld->getAgent(gAgentIndexFocus)->set_camera();

		//Show the background image (parallax) and foreground image (active borders) [note: this is what costs a lot wrt. computation time]
		if (gBackgroundImage != NULL && gNiceRendering)
			apply_surface(0, 0, gBackgroundImage, gScreen, &gCameraParallax);
		else
			if (gNiceRendering)
			SDL_FillRect(gScreen, &gScreen->clip_rect, SDL_MapRGB(gScreen->format, 0xFF, 0xFF, 0xFF)); // clear screen if no background image
		else
			apply_surface(0, 0, gZoneImage, gScreen, &gCameraParallax);

		if (gNiceRendering)
			apply_surface(0, 0, gForegroundImage, gScreen, &gCamera);
		else
			apply_surface(0, 0, gEnvironmentImage, gScreen, &gCamera);


		//Show the dot on the screen
		for (int i = 0; i != gAgentCounter; i++) {
			if (gWorld->getAgent(i)->isRegistered())
				gWorld->getAgent(i)->unregisterAgent(); // remove agent from memory so as to correctly cast sensors (otw: may see itself)

			gWorld->getAgent(i)->show(); // show sensor rays.

			// re-registering agents (post-display)
			if (!gWorld->getAgent(i)->isRegistered())
				gWorld->getAgent(i)->registerAgent();
		}

		// show inspector agent location (single point)
		if (gInspectorMode)
			inspectorAgent->show();


		/**/
		if (SDL_Flip(gScreen) == -1) {
			std::cout << "[critical] cannot flip screen (problem with video)." << std::endl << std::endl;
			exit(1);
		}

		//Cap the frame rate
		if (fps.get_ticks() < 1000 / gFramesPerSecond) {
			SDL_Delay((1000 / gFramesPerSecond) - fps.get_ticks());
		}
		/**/

		// video capture (sync with screen update)
		if (gVideoRecording == true)
			saveScreenshot("movie");
	}
}

void initLogging() {
	// main log file.

	gLogFile.open(gLogFilename.c_str()); //, std::ofstream::out | std::ofstream::app);

	if (!gLogFile) {
		std::cout << "[error] Cannot open log file " << gLogFilename << "." << std::endl << std::endl;
		exit(1);
	}

	gLogFile << "# =-=-=-=-=-=-=-=-=-=-=" << std::endl;
	gLogFile << "# LOG DATA " << std::endl;
	gLogFile << "# =-=-=-=-=-=-=-=-=-=-=" << std::endl;
	gLogFile << "#" << std::endl;
	gLogFile << "# =-= Roborobo! " << std::endl;
	gLogFile << "# =-= Official version tag    : " << gVersion << std::endl;
	gLogFile << "# =-= Current build name      : " << gCurrentBuildInfo << std::endl;
	gLogFile << "# =-= Compilation version tag : " << gCompileDate << " - " << gCompileTime << std::endl;
	gLogFile << "#" << std::endl;
	gLogFile << "# Loaded time stamp           : " << gStartTime << std::endl;
	gLogFile << "#" << std::endl;

	//gLogFile << "# log comment      : " << gLogCommentText << std::endl; 

}

void stopLogging() {
	gLogFile.close();
}

void initMonitor(bool __isInspectorAgent) // default argument value is false
{
	//The frame rate regulator -- dont use them before SDL is initialized.
	timeWatch.start();
	timetag = timeWatch.get_ticks();

	if (__isInspectorAgent) {
		gInspectorAgent = true;
		inspectorAgent = new InspectorAgent();
	} else {
		gInspectorAgent = false;
	}
}

void updateMonitor(Uint8* __keyboardStates) {
	if (timetag == -1) {
		std::cout << "[critical]: Monitor not initialized (call to initMonitor() missing)." << std::endl;
		exit(1);
	}

	if (gPauseMode == false) {
		int frames = 1000;
		if (gWorld->getIterations() % frames == 0) {
			int timetag2 = timeWatch.get_ticks();
			int instantFps = (int) (frames * 1000.0 / ((double) (timetag2 - timetag)));
			if (gVerbose) std::cout << "[" << instantFps << " fps]" << std::endl;
			timetag = timetag2;
		}
	}

	if (gInspectorAgent == true) {
		// update inspector agent
		if (gDisplayMode == 0 || (gDisplayMode == 1 && gWorld->getIterations() % gFastDisplayModeSpeed == 0))
			if (inspectorAgent != NULL && gInspectorMode) {
				inspectorAgent->stepBehavior(__keyboardStates);
				inspectorAgent->move();
			}
	}
}

static bool _parseOption(char const* name, bool& value, bool required)
{
	if (gProperties.hasProperty(name)) {
		std::string s = gProperties.getProperty(name);
		if (s == "true" || s == "True" || s == "TRUE")
			value = true;
		else
			if (s == "false" || s == "False" || s == "FALSE")
			value = false;
		else {
			std::cerr << "[CORRUPT] " << name << " should be boolean (\"true\" or \"false\").\n";
			return false;
		}
	} else if (required) {
		std::cerr << "[MISSING] " << name << " bool value is missing.\n";
		return false;
	}

	return true;
}

static bool _parseOption(char const* name, int& value, bool required)
{
	if (gProperties.hasProperty(name)) {
		convertFromString<int>(value, gProperties.getProperty(name), std::dec);
	} else if (required) {
		std::cerr << "[MISSING] " << name << " int value is missing.\n";
		return false;
	}

	return true;
}

static bool _parseOption(char const* name, float& value, bool required)
{
	if (gProperties.hasProperty(name)) {
		convertFromString<float>(value, gProperties.getProperty(name), std::dec);
	} else if (required) {
		std::cerr << "[MISSING] " << name << " float value is missing.\n";
		return false;
	}

	return true;
}

static bool _parseColorOption(char const* name, Uint32& value, bool required)
{
	const char* HEX_DIGITS = "0123456789abcdefABCDEF";

	if (gProperties.hasProperty(name)) {
		std::string s = gProperties.getProperty(name);
		const char *str = s.c_str();

		if (strspn(str, HEX_DIGITS) != 8) {
			std::cerr << "[CORRUPT] " << name << " color value must have hex format RRGGBBAA.\n";
			return false;
		}

		value = 0;
		for (int i=0; i < 8; i++) {
			value = (value << 4) + (strchr(HEX_DIGITS, tolower(str[i])) - HEX_DIGITS);
		}
	} else if (required) {
		std::cerr << "[MISSING] " << name << " color value is missing.\n";
		return false;
	}

	return true;
}

bool loadProperties(std::string __propertiesFilename) {

	bool returnValue = true;

	std::ifstream in(__propertiesFilename.c_str());

	if (in == NULL)
		return false;
	gProperties.load(in);
	in.close();

	std::string s;
	if (gProperties.hasProperty("ConfigurationLoaderObjectName")) {
		std::string configurationLoaderObjectName = gProperties.getProperty("ConfigurationLoaderObjectName");
		gConfigurationLoader = ConfigurationLoader::make_ConfigurationLoader(configurationLoaderObjectName);
		if (!gConfigurationLoader) {
			std::cerr << "[CORRUPT] The configuration " << configurationLoaderObjectName << " isn't known by the ConfigurationLoader\n";
			returnValue = false;
		}
	} else {
		std::cerr << "[MISSING] ConfigurationLoaderObjectName string value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gAgentIndexFocus"))
		convertFromString<int>(gAgentIndexFocus, gProperties.getProperty("gAgentIndexFocus"), std::dec);
	else {
		std::cerr << "[MISSING] gAgentIndexFocus value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gInspectorAgentXStart"))
		convertFromString<int>(gInspectorAgentXStart, gProperties.getProperty("gInspectorAgentXStart"), std::dec);
	else {
		std::cerr << "[MISSING] gInspectorAgentXStart value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gInspectorAgentYStart"))
		convertFromString<int>(gInspectorAgentYStart, gProperties.getProperty("gInspectorAgentYStart"), std::dec);
	else {
		std::cerr << "[MISSING] gInspectorAgentYStart value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gDisplayMode"))
		convertFromString<int>(gDisplayMode, gProperties.getProperty("gDisplayMode"), std::dec);
	else {
		std::cerr << "[MISSING] gDisplayMode value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gFastDisplayModeSpeed"))
		convertFromString<int>(gFastDisplayModeSpeed, gProperties.getProperty("gFastDisplayModeSpeed"), std::dec);
	else {
		std::cerr << "[MISSING] gFastDisplayModeSpeed value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gFramesPerSecond"))
		convertFromString<int>(gFramesPerSecond, gProperties.getProperty("gFramesPerSecond"), std::dec);
	else {
		std::cerr << "[MISSING] gFramesPerSecond value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gLocomotionMode"))
		convertFromString<int>(gLocomotionMode, gProperties.getProperty("gLocomotionMode"), std::dec);
	else {
		std::cerr << "[MISSING] gLocomotionMode value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gMaxSpeedOnXaxis"))
		convertFromString<int>(gMaxSpeedOnXaxis, gProperties.getProperty("gMaxSpeedOnXaxis"), std::dec);
	else {
		std::cerr << "[MISSING] gMaxSpeedOnXaxis value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gMaxSpeedOnYaxis"))
		convertFromString<int>(gMaxSpeedOnYaxis, gProperties.getProperty("gMaxSpeedOnYaxis"), std::dec);
	else {
		std::cerr << "[MISSING] gMaxSpeedOnYaxis value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gMaxTranslationalSpeed"))
		convertFromString<float>(gMaxTranslationalSpeed, gProperties.getProperty("gMaxTranslationalSpeed"), std::dec);
	else {
		std::cerr << "[MISSING] gMaxTranslationalSpeed value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gMaxTranslationalDeltaValue")) // ie. max update (delta) btw current speed and desired speed
		convertFromString<float>(gMaxTranslationalDeltaValue, gProperties.getProperty("gMaxTranslationalDeltaValue"), std::dec);
	else {
		std::cerr << "[MISSING] gMaxTranslationalDeltaValue value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gMaxRotationalSpeed"))
		convertFromString<float>(gMaxRotationalSpeed, gProperties.getProperty("gMaxRotationalSpeed"), std::dec);
	else {
		std::cerr << "[MISSING] gMaxRotationalSpeed value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gMoveStepHeight"))
		convertFromString<int>(gMoveStepHeight, gProperties.getProperty("gMoveStepHeight"), std::dec);
	else {
		std::cerr << "[MISSING] gMoveStepHeight value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gMoveStepWidth"))
		convertFromString<int>(gMoveStepWidth, gProperties.getProperty("gMoveStepWidth"), std::dec);
	else {
		std::cerr << "[MISSING] gMoveStepWidth value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gMaxIt"))
		convertFromString<int>(gMaxIt, gProperties.getProperty("gMaxIt"), std::dec);
	else {
		std::cerr << "[MISSING] gMaxIt value is missing.\n";
		returnValue = false;
	}


	if (gProperties.hasProperty("gNbOfAgents"))
		convertFromString<int>(gNbOfAgents, gProperties.getProperty("gNbOfAgents"), std::dec);
	else {
		std::cerr << "[MISSING] gNbOfAgents value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gParallaxFactor"))
		convertFromString<int>(gParallaxFactor, gProperties.getProperty("gParallaxFactor"), std::dec);
	else {
		std::cerr << "[MISSING] gParallaxFactor value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gRandomSeed")) {
		convertFromString<int>(gRandomSeed, gProperties.getProperty("gRandomSeed"), std::dec);

		if (gRandomSeed == -1) // value = -1 means random seed. set seed, then update content of properties.
		{
			// set seed value
			gRandomSeed = (unsigned int) time(NULL); // time-based random seed, if needed.

			// update properties

			gProperties.setProperty("gRandomSeed", convertToString(gRandomSeed)); // update value.
		}
	} else {
		std::cerr << "[MISSING] gRandomSeed value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gScreenHeight"))
		convertFromString<int>(gScreenHeight, gProperties.getProperty("gScreenHeight"), std::dec);
	else {
		std::cerr << "[MISSING] gScreenHeight value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gScreenWidth"))
		convertFromString<int>(gScreenWidth, gProperties.getProperty("gScreenWidth"), std::dec);
	else {
		std::cerr << "[MISSING] gScreenWidth value is missing.\n";
		returnValue = false;
	}
	
	if (gProperties.hasProperty("gScreenWidth"))
		convertFromString<int>(gScreenWidth, gProperties.getProperty("gScreenWidth"), std::dec);
	else {
		std::cerr << "[MISSING] gScreenWidth value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gSensorRange"))
		convertFromString<int>(gSensorRange, gProperties.getProperty("gSensorRange"), std::dec);
	else {
		std::cerr << "[MISSING] gSensorRange value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gMaxRadioDistance")) {
		convertFromString<int>(gMaxRadioDistanceToSquare, gProperties.getProperty("gMaxRadioDistance"), std::dec);
		gMaxRadioDistanceToSquare *= gMaxRadioDistanceToSquare; // stored value to the square -- speed up computation
	} else {
		std::cerr << "[MISSING] gMaxRadioDistance value is missing (set to -1 if not used).\n";
		returnValue = false;
	}

	returnValue &= _parseOption("gBatchMode", gBatchMode, false);
	returnValue &= _parseOption("gPauseMode", gPauseMode, false);
	returnValue &= _parseOption("gUserCommandMode", gUserCommandMode, false);

	returnValue &= _parseOption("gInspectAgent", gInspectAgent, false);
	returnValue &= _parseOption("gInspectorAgent", gInspectorAgent, false);
	returnValue &= _parseOption("gInspectorMode", gInspectorMode, false);

	returnValue &= _parseOption("gAgentRegistration", gAgentRegistration, false);
	returnValue &= _parseOption("gVideoRecording", gVideoRecording, false);

	returnValue &= _parseOption("gDisplaySensors", gDisplaySensors, false);
	returnValue &= _parseOption("gDisplayZoneCaption", gDisplayZoneCaption, false);
	returnValue &= _parseOption("gNiceRendering", gNiceRendering, false);
	returnValue &= _parseOption("gShowOrientationMarker", gShowOrientationMarker, false);
	returnValue &= _parseOption("gOrientationMarkerRadius", gOrientationMarkerRadius, false);

	returnValue &= _parseColorOption("gEnergyPointColor", gEnergyPointColor, false);
	returnValue &= _parseColorOption("gOrientationMarkerColor", gOrientationMarkerColor, false);
	returnValue &= _parseColorOption("gSensorRayFreeColor", gSensorRayFreeColor, false);
	returnValue &= _parseColorOption("gSensorRayObstacleColor", gSensorRayObstacleColor, false);
	returnValue &= _parseColorOption("gSensorRayEnergyColor", gSensorRayEnergyColor, false);

	returnValue &= _parseOption("gVerbose", gVerbose, false);

	returnValue &= _parseOption("gRadioNetwork", gRadioNetwork, false);

	if (gProperties.hasProperty("gAgentMaskImageFilename"))
		gAgentMaskImageFilename = gProperties.getProperty("gAgentMaskImageFilename");
	else {
		std::cerr << "[MISSING] gAgentMaskImageFilename string value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gAgentSpecsImageFilename"))
		gAgentSpecsImageFilename = gProperties.getProperty("gAgentSpecsImageFilename");
	else {
		std::cerr << "[MISSING] gAgentSpecsImageFilename string value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gBackgroundImageFilename"))
		gBackgroundImageFilename = gProperties.getProperty("gBackgroundImageFilename");
	else {
		std::cerr << "[MISSING] gBackgroundImageFilename string value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gEnvironmentImageFilename"))
		gEnvironmentImageFilename = gProperties.getProperty("gEnvironmentImageFilename");
	else {
		std::cerr << "[MISSING] gEnvironmentImageFilename string value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gForegroundImageFilename"))
		gForegroundImageFilename = gProperties.getProperty("gForegroundImageFilename");
	else {
		std::cerr << "[MISSING] gForegroundImageFilename string value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gLogCommentText"))
		gLogCommentText = gProperties.getProperty("gLogCommentText");
	else {
		std::cerr << "[MISSING] gLogCommentText string value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gLogFilename"))
		gLogFilename = gProperties.getProperty("gLogFilename");
	else {
		gLogFilename = "logs/datalog_" + gStartTime + ".txt";
		gProperties.setProperty("gLogFilename", gLogFilename);

		std::cout << "[WARNING] No default gLogFilename string value. Log data will be written in \"" << gLogFilename << "\"\n";
		//returnValue = false;
	}

	if (gProperties.hasProperty("gZoneCaptionPrefixFilename"))
		gZoneCaptionPrefixFilename = gProperties.getProperty("gZoneCaptionPrefixFilename");
	else {
		std::cerr << "[MISSING] gZoneCaptionPrefixFilename string value is missing.\n";
		returnValue = false;
	}

	if (gProperties.hasProperty("gZoneImageFilename"))
		gZoneImageFilename = gProperties.getProperty("gZoneImageFilename");
	else {
		std::cerr << "[MISSING] gZoneImageFilename string value is missing.\n";
		returnValue = false;
	}

	returnValue &= _parseOption("gEnergyMode", gEnergyMode, false);

	if (gEnergyMode == true) {
		returnValue &= _parseOption("gMaxEnergyPoints", gMaxEnergyPoints, true);
		returnValue &= _parseOption("gEnergyPointRadius", gEnergyPointRadius, true);
		returnValue &= _parseOption("gEnergyPointValue", gEnergyPointValue, true);
		returnValue &= _parseOption("gEnergyPointRespawnLagMaxValue", gEnergyPointRespawnLagMaxValue, true);
		returnValue &= _parseOption("gEnergyPoints_alwaysRender", gEnergyPoints_alwaysRender, false);

		if (gProperties.hasProperty("gEnergyDensityImageFilename"))
		    gEnergyDensityImageFilename = gProperties.getProperty("gEnergyDensityImageFilename");
	}

	// Load organism related properties

	gUseOrganisms = false;

	returnValue &= _parseOption("gUseOrganisms", gUseOrganisms, false);

	if (gUseOrganisms) {
		if (gProperties.hasProperty("gAgentPositiveMaskImageFilename")){
			gAgentPositiveMaskImageFilename = gProperties.getProperty("gAgentPositiveMaskImageFilename");
		}else if(gUseOrganisms){
			std::cerr << "[MISSING] gAgentPositiveMaskImageFilename string value is missing.\n";
			if (gUseOrganisms)
				returnValue = false;
		}
		
		if (gProperties.hasProperty("gAgentNegativeMaskImageFilename")){
			gAgentNegativeMaskImageFilename = gProperties.getProperty("gAgentNegativeMaskImageFilename");
		}else if(gUseOrganisms){
			std::cerr << "[MISSING] gAgentNegativeMaskImageFilename string value is missing.\n";
			if (gUseOrganisms)
				returnValue = false;
		}

		if (gProperties.hasProperty("gConnectionGap"))
			convertFromString<int>(gConnectionGap, gProperties.getProperty("gConnectionGap"), std::dec);
		else if(gUseOrganisms){
			std::cerr << "[MISSING] gConnectionGap int value is missing, using default (20).\n";
			gConnectionGap = 20;
			//returnValue = false;
		}
		
		if (gProperties.hasProperty("gOrganismLogFilename"))
			gOrganismLogFilename = gProperties.getProperty("gOrganismLogFilename");
		else {
			gOrganismLogFilename = "logs/organisms_" + gStartTime + ".txt";
			gProperties.setProperty("gOrganismLogFilename", gOrganismLogFilename);

			std::cout << "[WARNING] No default gOrganismLogFilename string value. Log data will be written in \"" << gOrganismLogFilename << "\"\n";
			//returnValue = false;
		}
		
		if (gProperties.hasProperty("gOrganismSizesLogFilename"))
			gOrganismSizesLogFilename = gProperties.getProperty("gOrganismSizesLogFilename");
		else {
			gOrganismLogFilename = "logs/organisms_sizes_" + gStartTime + ".txt";
			gProperties.setProperty("gOrganismSizesLogFilename", gOrganismSizesLogFilename);

			std::cout << "[WARNING] No default gOrganismSizesLogFilename string value. Log data will be written in \"" << gOrganismLogFilename << "\"\n";
			//returnValue = false;
		}

		if (gProperties.hasProperty("gOrganismSampleFrequency"))
			convertFromString<int>(gOrganismSampleFrequency, gProperties.getProperty("gOrganismSampleFrequency"), std::dec);
		else if(gUseOrganisms){
			std::cerr << "[MISSING] gOrganismSampleFrequency int value is missing, using default (250).\n";
			gOrganismSampleFrequency = 250;
			//returnValue = false;
		}
	}
	
	returnValue &= _parseOption("gRiverbed", gRiverbed, false);

	if (gRiverbed) {
		if (gProperties.hasProperty("gStreamDirection")) {
			convertFromString<int>(gStreamDirection, gProperties.getProperty("gStreamDirection"), std::dec);
		} else {
			std::cerr << "[MISSING] gStreamDirection int value is missing, using default (0).\n";
			gStreamDirection = 0;
			//returnValue = false;
		}
		
		if (gProperties.hasProperty("gStreamStrength")) {
			convertFromString<double>(gStreamStrength, gProperties.getProperty("gStreamStrength"), std::dec);
		} else {
			std::cerr << "[MISSING] gStreamStrength double value is missing, using default (1).\n";
			gStreamStrength = 1.5;
			//returnValue = false;
		}
		if (gProperties.hasProperty("gRiverbedZone")) {
			convertFromString<int>(gRiverbedZone, gProperties.getProperty("gRiverbedZone"), std::dec);
		} else {
			std::cerr << "[MISSING] gRiverbedZone double value is missing, using default (-1), this means everywhere.\n";
			gRiverbedZone = -1;
			//returnValue = false;
		}
	}
    
    if (gProperties.hasProperty("gSpeedScale"))
		convertFromString<int>(gSpeedScale, gProperties.getProperty("gSpeedScale"), std::dec);
	else {
		// std::cerr << "[MISSING] gSpawnWidth value is missing.\n";
	}
	
	if (gProperties.hasProperty("gSpawnWidth"))
		convertFromString<int>(gSpawnWidth, gProperties.getProperty("gSpawnWidth"), std::dec);
	else {
		// std::cerr << "[MISSING] gSpawnWidth value is missing.\n";
	}
	
	if (gProperties.hasProperty("gSpawnHeight"))
		convertFromString<int>(gSpawnHeight, gProperties.getProperty("gSpawnHeight"), std::dec);
	else {
		// std::cerr << "[MISSING] gSpawnHeight value is missing.\n";
	}
	
	returnValue &= _parseOption("gRenderRobotId", gRenderRobotId, false);
	returnValue &= _parseOption("gShowAgentEnergyPoints", gShowAgentEnergyPoints, false);

	// Jan-Willem: Removed 'gRenderRobotId && ' from if statement so the font can be used for other purposes too
	if (gProperties.hasProperty("gFontName")) {
		gFontName = gProperties.getProperty("gFontName");
	}else if(gRenderRobotId){
		std::cerr << "[MISSING] gFontName string value is missing, using default (/usr/share/fonts/truetype/ttf-bitstream-vera/Vera.ttf).\n";
	}

	returnValue &= _parseOption("gEnableDitchSensors", gEnableDitchSensors, false);

	bool dumpProperties = true;

	returnValue &= _parseOption("gDumpProperties", dumpProperties, false);

	if (dumpProperties) {
		// * Dump a raw copy of the properties file from gProperties, ie. as it was parsed and understood.

		// prepare filename
		std::string outputFileNameTmp = "logs/";
		outputFileNameTmp += "properties_";
		outputFileNameTmp += gStartTime;
		outputFileNameTmp += ".txt";

		// open file
		const std::string outputFile = outputFileNameTmp;
		std::ofstream out(outputFile.c_str());

		// dump header information
		out << "# =-=-=-=-=-=-=-=-=-=-=" << std::endl;
		out << "# PROPERTIES FILE DUMP " << std::endl;
		out << "# =-=-=-=-=-=-=-=-=-=-=" << std::endl;
		out << "#" << std::endl;
		out << "# =-= Roborobo! " << std::endl;
		out << "# =-= Official version tag    : " << gVersion << std::endl;
		out << "# =-= Current build name      : " << gCurrentBuildInfo << std::endl;
		out << "# =-= Compilation version tag : " << gCompileDate << " - " << gCompileTime << std::endl;
		out << "#" << std::endl;
		out << "# Loaded time stamp           : " << gStartTime << std::endl;
		out << "#" << std::endl;
		out << "# Original Properties file    : " << __propertiesFilename << std::endl;
		out << "#" << std::endl;
		out << std::endl << std::endl;

		// dump properties content
		gProperties.store(out);

		// close file
		out.close();
	}

	return returnValue;
}

// ---- ---- ---- ----

void initRoborobo() {
	// load properties
	if (loadProperties(gPropertiesFilename) == false) {
		std::cout << "[critical] properties file contains error(s) or does not exist (\"" << gPropertiesFilename << "\")." << std::endl << std::endl;
		exit(1);
	}

	if (gBatchMode == false)
		displayHelp(); // display welcome text and help commands

	// * Initialize SDL	

	gCamera.x = 0;
	gCamera.y = 0;
	gCamera.w = gScreenWidth;
	gCamera.h = gScreenHeight;
	gCameraParallax.x = 0;
	gCameraParallax.y = 0;
	gCameraParallax.w = gScreenWidth;
	gCameraParallax.h = gScreenHeight;

	// Jan-Willem: Added SDL_HWACCEL
	if (initSDL(SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_HWACCEL) == false) // other option: SDL_HWACCEL
	{
		std::cout << "[critical] cannot initialize SDL: " << SDL_GetError() << std::endl << std::endl;
		exit(1);
	}

	// * Initialize log file(s)

	initLogging();

	// * Initialize Random seed -- loaded, or initialized, in loadProperties(...)

	srand(gRandomSeed); // fixed seed - useful to reproduce results (ie. deterministic sequence of random values)
	gLogFile << "# random seed             : " << gRandomSeed << std::endl;

	gWorld = new World();

	// * run
	gWorld->initWorld();

	if (gBatchMode == false)
		initMonitor(true); // add speed monitoring and inspector agent
}

bool runRoborobo(int __maxIt) // default parameter is -1 (infinite)
{
	//While the user hasn't quit
	bool quit = false;
	int currentIt = 0;
	while (quit == false && (currentIt < __maxIt || __maxIt == -1)) {
		if (gBatchMode) {
			gWorld->updateWorld();
			if (gWorld->getIterations() % 10000 == 0)
				if (gVerbose) std::cout << ".";
		} else {

			Uint8 *keyboardStates = SDL_GetKeyState(NULL);
			quit = checkQuitEvent() | handleKeyEvent(keyboardStates);

			//Start the frame timer
			fps.start();

			if (gPauseMode == false) {
				if (gUserCommandMode == true && gInspectorMode == false)
					gWorld->updateWorld(keyboardStates);
				else
					gWorld->updateWorld();
			}
			//Update the screen
			updateDisplay();
			updateMonitor(keyboardStates);
		}

		currentIt++;
		
		if (gUseOrganisms && (gWorld->getIterations() % gOrganismSampleFrequency == 0)) {
			Organism::logAll();
		}
		
		if (gWorld->getNbOfAgent() <= 0) {
			quit = true;
		}
	}

	return quit;
}

void closeRoborobo() {
	// * clean up and quit
	stopLogging();
	clean_up();
	SDL_Quit();
}

void resetRoboroboWorld() {
	gWorld->resetWorld();
}


// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----

int launchRoborobo() // the main loop.
{
	initRoborobo();
	runRoborobo(gMaxIt);
	closeRoborobo();

	return 0;
}

