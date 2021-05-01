#include <user_api.h>
#include <string.h>
#include <xs_klist.h>
#include <xs_adapter_lora.h>
#include <xs_adapter_manager.h>

extern DoubleLinklistType online_user_head;

void net_lora_gateway(int argc, char *argv[])
{
    // 1.New specific agreement (LORA) adapter
    static struct AdapterLora lora_adapter;
    memset(&lora_adapter, 0, sizeof(lora_adapter));
    struct AdapterDone lora_example_done = {
        .NetAiitOpen = LoraAdapterOpen,
        .NetAiitClose = NULL,
        .NetAiitSend = NULL,
        .NetAiitReceive = LoraAdapterReceive,
        .NetAiitJoin = LoraAdapterJoin,
        .NetAiitIoctl = NULL,
    };
    lora_adapter.parent.done = lora_example_done; // Bind adapter operation
    lora_adapter.name = "lora_dev_456"; // Set adapter name
    lora_adapter.spi_lora_fd = -1; // Set adapter information
    lora_adapter.deve_ui = "xxx";
    lora_adapter.app_key = "yyy";

    // 2.Register the adapter in the list
    LoraAdapterInit();
    LoraAdapterRegister((adapter_t)&lora_adapter);

    // 3.Find from the list of registered adapters
    adapter_t padapter = LoraAdapterFind("lora_dev_456");
    if (NONE == padapter)
    {
        printf("adapter find failed!\n");
        return;
    }

    // 4.Open adapter
    if (0 != padapter->done.NetAiitOpen(padapter))
    {
        printf("adapter open failed!\n");
        return;
    }

    // 5.Join the specified network segment as gateway
    padapter->done.NetAiitJoin(padapter, ROLE_TYPE_MASTER, CONNECTION_COMMUNICATION_LORA_NET_ID);
}



static void net_lora_connectedlist(int argc, char *argv[])
{
    DoubleLinklistType* pNode;
    printf("******** connected users *********\n");
    DOUBLE_LINKLIST_FOR_EACH(pNode, &online_user_head)
    {
        OnlineUser* pUser =CONTAINER_OF(pNode, OnlineUser, link);
        printf("%s\n", pUser->user_name);
    }
    printf("*********************************\n");
}