#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

static u32 *socBuffer = NULL;

static Result init_network(void) {
    Result rc;

    socBuffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    if (!socBuffer) {
        return -1;
    }

    rc = socInit(socBuffer, SOC_BUFFERSIZE);
    if (R_FAILED(rc)) {
        free(socBuffer);
        socBuffer = NULL;
        return rc;
    }

    // HTTP GETだけなら sharedmem_size は 0 でOK
    rc = httpcInit(0);
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

static Result fetch_status(char *out, size_t outSize) {
    Result rc;
    httpcContext context;
    u32 statuscode = 0;
    u32 contentsize = 0;
    u32 downloaded = 0;

    // ここを自分のMacのローカルIPにする
    const char *url = "http://192.168.11.63:8000/status";

    memset(&context, 0, sizeof(context));
    memset(out, 0, outSize);

    rc = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 1);
    if (R_FAILED(rc)) return rc;

    rc = httpcBeginRequest(&context);
    if (R_FAILED(rc)) {
        httpcCloseContext(&context);
        return rc;
    }

    rc = httpcGetResponseStatusCode(&context, &statuscode);
    if (R_FAILED(rc)) {
        httpcCloseContext(&context);
        return rc;
    }

    if (statuscode != 200) {
        snprintf(out, outSize, "HTTP %lu", (unsigned long)statuscode);
        httpcCloseContext(&context);
        return 0;
    }

    rc = httpcGetDownloadSizeState(&context, NULL, &contentsize);
    if (R_FAILED(rc)) {
        httpcCloseContext(&context);
        return rc;
    }

    if (contentsize >= outSize) {
        contentsize = (u32)outSize - 1;
    }

    rc = httpcDownloadData(&context, (u8*)out, contentsize, &downloaded);
    if (rc == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING) {
        rc = 0;
    }

    if (R_FAILED(rc)) {
        httpcCloseContext(&context);
        return rc;
    }

    out[downloaded] = '\0';
    httpcCloseContext(&context);
    return 0;
}

int main(int argc, char **argv)
{
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("3DS POS Client\n");
    printf("A: connect /status\n");
    printf("START: exit\n\n");

    Result rc = init_network();
    if (R_FAILED(rc)) {
        printf("network init failed: 0x%08lX\n", (unsigned long)rc);
    } else {
        printf("network init ok\n");
    }

    char response[1024];

    while (aptMainLoop())
    {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) break;

        if (kDown & KEY_A)
        {
            printf("\x1b[8;1Hconnecting...                    \n");
            memset(response, 0, sizeof(response));

            rc = fetch_status(response, sizeof(response));
            if (R_FAILED(rc)) {
                printf("\x1b[9;1Hrequest failed: 0x%08lX          \n", (unsigned long)rc);
            } else {
                printf("\x1b[9;1Hresponse:                        \n");
                printf("\x1b[10;1H%s                              \n", response);
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