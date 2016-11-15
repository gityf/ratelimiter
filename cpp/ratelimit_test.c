#include <stdio.h>
#include "ratelimit.h"

int main() {
	struct ratelimit_state state;
	ratelimit_default_init(&state);
	while(1) {
		if (0 == ratelimit(&state)) {
			break;
		}
	}
	printf("burst:%d, missed:%d, printed:%d\n",
		state.burst, state.missed, state.printed);
	sleep(1);
	while(1) {
		if (0 == ratelimit(&state)) {
			break;
		}
	}
	printf("burst:%d, missed:%d, printed:%d\n",
		state.burst, state.missed, state.printed);
	ratelimit_state_exit(&state);
	return 0;
}