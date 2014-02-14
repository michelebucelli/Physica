/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
LOG MANAGEMENT
*/

#ifndef _LOG
#define _LOG

#include <iostream>
#include <fstream>

#include "files.h"

using namespace std;

extern ofstream outputLog;
extern int logTime;

#define LOG_HEADING									outputLog << "=========================== BEGINNING PROGRAM EXECUTION  ===========================\n"
#define LOG(MSG)									outputLog << MSG << endl;
#define LOG_HR										outputLog << "====================================================================================\n"
#define LOG_CLOSING_STD								outputLog << "========================= PROGRAM TERMINATED SUCCESSFULLY  =========================\n"

#define LOG_TIME_0(MSG)								outputLog << MSG << " (" << SDL_GetTicks() << "msec)" << endl

#define REPORT_TIME(INSTRUCTION,MESSAGE)			logTime = SDL_GetTicks(); INSTRUCTION; outputLog << MESSAGE << " (" << SDL_GetTicks() - logTime << "msec)" << endl
#define REPORT_TIME_0(INSTRUCTION,MESSAGE)			logTime = 0; INSTRUCTION; outputLog << MESSAGE << " (" << SDL_GetTicks() << "msec)" << endl
#define REPORT_RES_TIME(INSTRUCTION,MESSAGE)		logTime = SDL_GetTicks(); outputLog << INSTRUCTION << MESSAGE << " (" << SDL_GetTicks() - logTime << "msec)" << endl
#define REPORT_RES_TIME_0(INSTRUCTION,MESSAGE)		logTime = 0; outputLog << INSTRUCTION; outputLog << MESSAGE << " (" << SDL_GetTicks() - logTime << "msec)" << endl

#define LOG_WARN(MESSAGE)							outputLog << "WARNING: " << MESSAGE << endl
#define LOG_CHECK(CONDITION,MESSAGE)				if (!CONDITION) outputLog << "WARNING: " << MESSAGE << endl
#define LOG_ERR(MESSAGE)							outputLog << "ERROR:   " << MESSAGE << endl

#endif
