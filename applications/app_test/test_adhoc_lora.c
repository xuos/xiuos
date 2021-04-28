#include <xiuos.h>

extern void net_lora_client(int argc, char *argv[]);
extern void net_lora_gateway(int argc, char *argv[]);

void demo_lora_adhoc()
{
	#ifdef CONNECTION_COMMUNICATION_SET_AS_LORA_CLIENT
		char pgk_count[32];
		char* param[3];
		param[0] = "xxx";
		param[1] = CONNECTION_COMMUNICATION_LORA_CLIENT_NAME;
		itoa(CONNECTION_COMMUNICATION_LORA_CLIENT_PKG_COUNT, pgk_count, 10);
		param[2] = pgk_count;
		net_lora_client(2, param);
	#endif
	#ifdef CONNECTION_COMMUNICATION_SET_AS_LORA_GATEWAY
		net_lora_gateway(0, 0);
	#endif
}
