#include <user_api.h>
#include <string.h>
#include <xs_adapter_lora.h>
#include <xs_adapter_manager.h>
#include <stdio.h>
#include <stdlib.h>

char client_name[DEVNAME_LEN_MAX] = "lora_dev_123";

void success_cb(void *param)
{
    printf("success_cb, param = %s\n", param);
}

void invert_param(void *param)
{
    printf("success_cb02 invoke, invert bool param.\n");
    bool *bparam = (bool *)param;
    if (*bparam)
    {
        *bparam = false;
    }
    else
    {
        *bparam = true;
    }
}

void net_lora_client(int argc, char *argv[])
{
    int pkg_count = 10;
    if (argc >= 1)
    {
        memset(client_name, 0, DEVNAME_LEN_MAX);
        strncpy(client_name, argv[1], strlen(argv[1]));
        printf("lora client set clientName(%s).\n", client_name);
    }
    
    if (argc >= 2)
    {
        pkg_count = atoi(argv[2]);
        printf("lora client set pkg_count(%d).\n", pkg_count);
    }

    // 1.Create an adapter for a specific agreement (LORA)
    static struct AdapterLora lora_adapter;
    memset(&lora_adapter, 0, sizeof(lora_adapter));
    struct AdapterDone lora_example_done = {
        .NetAiitOpen = LoraAdapterOpen,
        .NetAiitClose = LoraAdapterCose,
        .NetAiitSend = LoraAdapterSendc2g,
        .NetAiitReceive = NULL,
        .NetAiitJoin = LoraAdapterJoin,
        .NetAiitIoctl = NULL,
    };
    lora_adapter.parent.done = lora_example_done; // Bind adapter operation
    lora_adapter.name = client_name;     // Set adapter name
    lora_adapter.spi_lora_fd = -1;      // Set adapter information
    lora_adapter.deve_ui = "xxx";
    lora_adapter.app_key = "yyy";

    // 2.Register the adapter in the list
    LoraAdapterInit();
    LoraAdapterRegister((adapter_t)&lora_adapter);

    // 3.Find from the list of registered adapters
    adapter_t padapter = LoraAdapterFind(client_name);
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

    // 5.Join the specified network segment as client
    printf("NetAiitJoin start. \n");
    padapter->done.NetAiitJoin(padapter, ROLE_TYPE_SLAVE, CONNECTION_COMMUNICATION_LORA_NET_ID);
    printf("NetAiitJoin end. \n");

    // 6.Point to point sending data to gateway
    int i = 0;
    while (i < pkg_count)
    {
        char data[120] = {0};
        sprintf(data, "*****  I am %s, data_num = %d ******" ,client_name, i);
        
        bool v = false;
        padapter->done.NetAiitSend(padapter, data, strlen(data) + 1, true, 10000, 0, invert_param, &v, NULL);
        while (!v) // Asynchronous analog synchronization
        {
            UserTaskDelay(100);
        }
        printf("send success(main thread)... %s\n" ,data);

        i++;
        UserTaskDelay(800); // Contract interval
    }
    printf("all pkg send success(main thread), quit.\n");
    
    padapter->done.NetAiitClose(padapter);
    printf("client quit.\n");
}
