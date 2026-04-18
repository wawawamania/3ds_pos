#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000
#define HTTPC_SHAREDMEM 0x1000

static u32 *socBuffer = NULL;

static Result init_network(void) {
    Result rc;

    socBuffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    if (!socBuffer) return -1;

    rc = socInit(socBuffer, SOC_BUFFERSIZE);
    if (R_FAILED(rc)) {
        free(socBuffer);
        socBuffer = NULL;
        return rc;
    }

    rc = httpcInit(HTTPC_SHAREDMEM);
    if (R_FAILED(rc)) {
        socExit();
        free(socBuffer);
        socBuffer = NULL;
        return rc;
    }

    return 0;
}

static void exit_network(void) {
    httpcExit();
    socExit();

    if (socBuffer) {
        free(socBuffer);
        socBuffer = NULL;
    }
}

static Result post_order(char *out, size_t outSize) {
    Result rc;
    httpcContext context;
    u32 statuscode = 0;

    const char *url = "http://192.168.11.46:8000/orders?productId=1&productName=cola&qty=1";

    memset(&context, 0, sizeof(context));
    memset(out, 0, outSize);

    rc = httpcOpenContext(&context, HTTPC_METHOD_POST, url, 1);
    if (R_FAILED(rc)) return rc;

    httpcSetKeepAlive(&context, HTTPC_KEEPALIVE_DISABLED);
    httpcAddRequestHeaderField(&context, "Connection", "close");
    httpcAddRequestHeaderField(&context, "User-Agent", "3DS POS Client");

    rc = httpcBeginRequest(&context);
    if (R_FAILED(rc)) goto cleanup;

    rc = httpcGetResponseStatusCode(&context, &statuscode);
    if (R_FAILED(rc)) goto cleanup;

    snprintf(out, outSize, "HTTP %lu", (unsigned long)statuscode);
    rc = 0;

cleanup:
    httpcCloseContext(&context);
    return rc;
}

int main(int argc, char **argv)
{
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("3DS POS Client v2\n");
    printf("POST target:\n");
    printf("192.168.11.46:8000/orders\n\n");

    printf("A: POST order\n");
    printf("START: exit\n\n");

    Result rc = init_network();
    if (R_FAILED(rc)) {
        printf("network init failed: 0x%08lX\n", (unsigned long)rc);
    } else {
        printf("network init ok\n");
    }

    char response[256];

    while (aptMainLoop())
    {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) break;

        if (kDown & KEY_A)
        {
            printf("\x1b[10;1Hposting order...            \n");

            rc = post_order(response, sizeof(response));

            if (R_FAILED(rc)) {
                printf("\x1b[11;1Hrequest failed: 0x%08lX      \n", (unsigned long)rc);
            } else {
                printf("\x1b[11;1Hresponse: %s                \n", response);
            }
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    exit_network();
    gfxExit();
    return 0;
}