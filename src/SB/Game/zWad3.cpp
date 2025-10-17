#include "zWad3.h"

#include "zVolume.cpp"
#include "zVent.cpp"
#include "zVar.cpp"
#include "zSurface.cpp"
#include "zScene.cpp"

//                                      xSGT / zSaveLoad

U32 saveSuccess;
F32 time_last;
F32 time_current;
F32 time_elapsed = 0.01f;
iTime t0;
iTime t1;
S32 promptSel;
S32 badCard;
S32 sAvailable;
S32 sNeeded;
S32 sAccessType;
U8 preAutoSaving;

S32 currentCard = -1;
S32 currentGame = -1;
F32 dontPoll = 1.0f;
S32 autoSaveCard = -1;

char currSceneStr[32] = "TEMP";
char sceneRead[32] = "0000";
zSaveLoadUI zSaveLoadUITable[62] = { { 0, 0, "ld gameslot group" },
                                     { 1, 0, "ld memcards group" },
                                     { 2, 0, "ld format prompt group" },
                                     { 3, 0, "ld mc missing group" },
                                     { 4, 0, "mnu3 ld mc1" },
                                     { 5, 0, "mnu3 ld mc2" },
                                     { 6, 0, "ld gameslot 0" },
                                     { 7, 0, "ld gameslot 1" },
                                     { 8, 0, "ld gameslot 2" },
                                     { 9, 0, "ld gameslot 3" },
                                     { 10, 0, "ld gameslot 4" },
                                     { 11, 0, "ld gameslot 5" },
                                     { 12, 0, "ld gameslot 6" },
                                     { 13, 0, "ld gameslot 7" },
                                     { 14, 0, "ld format prompt" },
                                     { 15, 0, "ld format yes" },
                                     { 16, 0, "ld format no" },
                                     { 17, 0, "ld mc missing" },
                                     { 18, 0, "ld nogames" },
                                     { 19, 0, "ld nogames" },
                                     { 20, 0, "mnu3 start group" },
                                     { 21, 0, "sv gameslot group" },
                                     { 22, 0, "sv memcards group" },
                                     { 23, 0, "sv mc1" },
                                     { 24, 0, "sv mc2" },
                                     { 25, 0, "sv gameslot 0" },
                                     { 26, 0, "sv gameslot 1" },
                                     { 27, 0, "sv gameslot 2" },
                                     { 28, 0, "sv gameslot 3" },
                                     { 29, 0, "sv gameslot 4" },
                                     { 30, 0, "sv gameslot 5" },
                                     { 31, 0, "sv gameslot 6" },
                                     { 32, 0, "sv gameslot 7" },
                                     { 33, 0, "sv format prompt" },
                                     { 34, 0, "sv overwrite" },
                                     { 35, 0, "sv overwrite damaged" },
                                     { 36, 0, "sv mc missing" },
                                     { 37, 0, "sv nospace" },
                                     { 38, 0, "sv nospacegame" },
                                     { 39, 0, "ld mc dontremove" },
                                     { 40, 0, "sv mc dontremove" },
                                     { 41, 0, "mnu4 mc dontremove" },
                                     { 42, 0, "ld badload" },
                                     { 43, 0, "sv badsave" },
                                     { 44, 0, "mnu3 badformat" },
                                     { 45, 0, "mnu3 badformatnocard" },
                                     { 46, 0, "mnu4 badformat" },
                                     { 47, 0, "mnu4 badformatnocard" },
                                     { 48, 0, "mnu3 format confirm" },
                                     { 49, 0, "mnu4 format confirm" },
                                     { 50, 0, "sv format group" },
                                     { 51, 0, "mnu3 disk free" },
                                     { 52, 0, "mnu4 disk free" },
                                     { 53, 0, "sv card damaged" },
                                     { 54, 0, "sv badsavenocard" },
                                     { 55, 0, "ld damaged card" },
                                     { 56, 0, "sv damaged card" },
                                     { 57, 0, "ld wrong device" },
                                     { 58, 0, "sv wrong device" },
                                     { 59, 0, "ld damaged save" },
                                     { 60, 0, "ld damaged save game" },
                                     { 0, 0, NULL } };

char* thumbIconMap[15] = { "ThumbIconHB", "ThumbIconJF", "ThumbIconBB", "ThumbIconGL",
                           "ThumbIconB1", "ThumbIconRB", "ThumbIconBC", "ThumbIconSM",
                           "ThumbIconB2", "ThumbIconKF", "ThumbIconGY", "ThumbIconDB",
                           "ThumbIconB3", "ThumbIconHB", "ThumbIconHB" };

#include "xSGT.cpp"
#include "zSaveLoad.cpp"

#include "zRewardsMgr.cpp"
#include "zRenderState.cpp"
#include "zPlayerSB.cpp"
#include "zPlayer.cpp"
