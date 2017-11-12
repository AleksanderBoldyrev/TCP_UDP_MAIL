#ifndef API_H
#define API_H

#include <string>

using namespace std;

#define DELIM_PARSE '|'

static const int API_SIZE = 10;

static const string API [API_SIZE] = 
{
	"Yes",
	"Noop",
	"Answer",
	"Start",
	"Init",
	"Operation Code",
	"Exit",
	"Register",
	"Login",
	"In system"
};

/// !!! MANAGE ALL CHANGES AT ServerWorker::parseOpCode

enum STATE
{
	SERV_OK = 0,
	NO_OPERATION = 1,
	ANSWER = 2,
	START = 3,
	INIT = 4,
	OPCODE = 5,
	EXIT = 6,
	REG = 7,
	LOG = 8,
        INSYS = 9
};

#endif