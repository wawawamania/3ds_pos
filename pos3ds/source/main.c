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

static Result post_order(char *out, size_t outSize) {
    Result rc;
    httpcContext context;
    u32 statuscode = 0;
    u32 downloaded = 0;

    const char *url = "http://192.168.11.63:8000/orders";
    const char *jsonBody = "{\"productId\":1,\"productName\":\"コーラ\",\"qty\":1}";

    memset(&context, 0, sizeof(context));
    memset(out, 0, outSize);

    rc = httpcOpenContext(&context, HTTPC_METHOD_POST, url, 1);
    if (R_FAILED(rc)) return rc;

    rc = httpcAddRequestHeaderField(&context, "Connection", "close");
    if (R_FAILED(rc)) goto cleanup;

    rc = httpcAddRequestHeaderField(&context, "User-Agent", "3DS POS Client");
    if (R_FAILED(rc)) goto cleanup;

    rc = httpcAddRequestHeaderField(&context, "Content-Type", "application/json");
    if (R_FAILED(rc)) goto cleanup;

    rc = httpcSetRequestBodyCopy(&context, (u8*)jsonBody, strlen(jsonBody));
    if (R_FAILED(rc)) goto cleanup;

    rc = httpcBeginRequest(&context);
    if (R_FAILED(rc)) goto cleanup;

    rc = httpcGetResponseStatusCode(&context, &statuscode);
    if (R_FAILED(rc)) goto cleanup;

    if (statuscode != 200) {
        snprintf(out, outSize, "HTTP %lu", (unsigned long)statuscode);
        rc = 0;
        goto cleanup;
    }

    rc = httpcDownloadData(&context, (u8*)out, outSize - 1, &downloaded);
    if (rc == HTTPC_RESULTCODE_DOWNLOADPENDING) {
        rc = 0;
    }

    if (R_FAILED(rc)) goto cleanup;

    out[downloaded] = '\0';
    rc = 0;

cleanup:
    httpcCloseContext(&context);
    return rc;
}

int main(int argc, char **argv)
{
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("3DS POS Client\n");
    printf("A: POST /orders\n");
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
            printf("\x1b[8;1Hposting order...                  \n");
            memset(response, 0, sizeof(response));

            rc = post_order(response, sizeof(response));
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