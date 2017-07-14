#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "remote_control_protocol.h"
#include "mydebug.h"

int mysystem(const char *command)
{
	int res;

	debug_msg("Execute: %s", command);
	res = system(command);
	debug_msg("res:%d",res);
	if(-1 == res){
		debug_msg("Failed to execute: system error.");
		return CMD_ERROR;
	}
	else{
		if(WIFEXITED(res)){
			if(0 == WEXITSTATUS(res)){
				debug_msg("Execute successfully.");
			}
			else{
				debug_msg("Failed to execute: shell failed[%d]", WEXITSTATUS(res));
				return CMD_ERROR;
			}
		}
		else{
			debug_msg("Failed to execute.");
			return CMD_ERROR;
		}
	}
	return SUCCESS;
}
