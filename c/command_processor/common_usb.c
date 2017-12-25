/*
 * Copyright (C) 2017 SilkID
 *
 * common_usb.c: Low-leve usb access functions, currently unused
 *
 * Author: Prem Mallappa <prem@silkid.com>
 */

#include <usb.h>

#include "common.h"
#include "cmd.h"
#include "common_usb.h"


#define SILKFP_VID              0x1B55
#define SILK20R                 0x0120
#define FVS100                  0x0200

#define SZ_KB   (1024)
#define SZ_MB   (1024*1024)

#define TIME_OUT        8000                           /* in msec */

unsigned char buf[(1 * SZ_MB)];

struct slk_conf {
    struct usb_device *dev;
    usb_dev_handle *handle;
    struct ep {                                 /* Endpoint addresses */
        unsigned char in;                       /* Input Endpoint */
        unsigned char out;                      /* Output Endpoint */
    } ep;
};

struct slk_usb_req {
    unsigned char  cmd;
    unsigned char  sub_cmd;
    unsigned char  windex;
    unsigned char  *ptr;
    unsigned char  size;
    unsigned short time_out;
    unsigned char  write;
};

static void slk_read_usb_string_simple(struct usb_device *dev,
                                       usb_dev_handle *dh)
{
#define STR_SIZE 64
    unsigned char str[STR_SIZE];
    int ret;

    fprintf(stderr, "Reading string from device\n");
    ret = usb_get_string_simple(dh, 1, str, sizeof(str));
    if (ret < 0) {
        fprintf(stderr, "usb_get_string_simple() failed with:%d\n", ret);
        return;
    }

    str[STR_SIZE] = '\0';                /* Safer */
    fprintf(stderr, "String: %s\n", str);

}

static void slk_find_device(struct slk_conf *conf)
{
    struct usb_bus *bus;
    struct usb_device *dev;
    usb_dev_handle *dh = NULL;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    for (bus = usb_get_busses(); bus; bus = bus -> next) {
        for (dev = bus -> devices; dev; dev = dev -> next) {

            if (dev->descriptor.idVendor == SILKFP_VID &&
                dev->descriptor.idProduct == SILK20R) {
                fprintf(stderr, "Found SilkID device\n");

                dh = usb_open(dev);
                if(!dh) {
                    fprintf(stderr, "Unable to open USB device, consider using SUDO\n");
                    usb_close(dh);
                    goto out;
                }

                slk_read_usb_string_simple(dev, dh);

                conf->dev = dev;
                conf->handle = dh;
                 /* We found what were looking for, exit */
                goto out;
            }
        } /* for (dev) */
    } /* for (bus) */

out:
    return;
}

static int slk_usb_send_ctrl_cmd(struct slk_conf *conf,
                            struct slk_usb_req *req)
{
    unsigned int req_type = USB_TYPE_VENDOR | USB_RECIP_DEVICE;

    req_type |= (req->write) ? USB_ENDPOINT_IN : USB_ENDPOINT_OUT;

    fprintf(stderr, "Sending USB CONTROL Command %x\n", req->cmd);

    return usb_control_msg(conf->handle, req_type, req->cmd,
                           req->sub_cmd, req->windex, req->ptr,
                           req->size, req->time_out);
}

static int slk_init_device(struct slk_conf *conf)
{
    int ret, num_ep, i;
    unsigned int request_type;
    struct slk_usb_req req = {0,};
    struct usb_device *dev = conf->dev;
    struct usb_endpoint_descriptor *ep;

    fprintf(stderr, "\t Setting Configuration 1\n");
    ret = usb_set_configuration(conf->handle, 1);
    if (ret < 0)
        goto set_config_fail;

    fprintf(stderr, "\t Claiming interface\n");
    ret = usb_claim_interface(conf->handle, 0);
    if (ret < 0)
        goto claim_interface_fail;

    ep = dev->config->interface->altsetting->endpoint;
    num_ep = dev->config->interface->altsetting->bNumEndpoints;

    fprintf(stderr, "\t num_ep: %d\n", num_ep);
    for(i = 0; i < num_ep ; i++) {
        if(ep->bmAttributes == USB_ENDPOINT_TYPE_BULK) {
            fprintf(stderr, "\t Device:%d Found attr: USB_ENDPOINT_TYPE_BULK\n", i);
            if(1 == (ep->bEndpointAddress >> 7)) {
                fprintf(stderr, "\t Endpoint Address <-IN\n");
                conf->ep.in = ep->bEndpointAddress;
            } else {
                fprintf(stderr, "\t Endpoint Address ->OUT\n");
                conf->ep.out = ep->bEndpointAddress;
            }
        }
        ep++;
    }

    req = (struct slk_usb_req) {
        .cmd = CMD_INIT,
        .time_out = TIME_OUT,
    };

    fprintf(stderr, "Sending INIT command\n");
    ret = slk_usb_send_ctrl_cmd(conf, &req);
    if (ret < 0) {
	perror("slk_usb_send_cmd:");
        goto ctrl_msg_fail;
    }

    return 0;

ctrl_msg_fail:
    usb_release_interface(conf->handle, 0);
claim_interface_fail:
    usb_set_configuration(conf->handle, 0);
set_config_fail:
    usb_close(conf->handle);

    return -1;
}

static void read_print_fw_version(struct slk_conf *conf)
{
    char str[2] = {0, 0};
    int ret;
    struct slk_usb_req req = {
        .cmd = CMD_GET_GPIO,
        .ptr = str,
        .windex = 0x55,                         /* This needs to be changed if the corresponding number changes */
        .size = sizeof(str),
        .time_out = TIME_OUT,
        .write = 1,
    };

    ret = slk_usb_send_ctrl_cmd(conf, &req);

    if(ret > 0) {
        fprintf(stderr, "Fw Version: %d.%02d\n", str[1], str[0]);
    }
}

static void read_print_global_var(struct slk_conf *conf)
{
    unsigned char str[2] = {0, 0};
    int ret;
    struct slk_usb_req req = {
        .cmd = CMD_GET_GPIO,
        .ptr = str,
        .windex = 0xAA,                         /* This needs to be changed if the corresponding number changes */
        .size = sizeof(str),
        .time_out = TIME_OUT,
        .write = 1,
    };

    ret = slk_usb_send_ctrl_cmd(conf, &req);

    if(ret > 0) {
        fprintf(stderr, "variable:%ld 1:%d 0:%u\n", str[1]<<8 | str[0], str[1], str[0]);
    }
}


static void read_print_build_version(struct slk_conf *conf)
{
    int ret = 0;
    char str[64] = {'J', 'u', 'n', 'k', '\0',};
    struct slk_usb_req req = {
        .cmd = CMD_GET_GPIO,
        .ptr = str,
        .windex = 0x57,                         /* This needs to be changed if the corresponding number changes */
        .size = sizeof(str),
        .time_out = TIME_OUT,
        .write = 1,
    };

    ret = slk_usb_send_ctrl_cmd(conf, &req);

    if(ret > 0) {
        str[sizeof(str)] = '\0';                /* safer bet */
        fprintf(stderr, "FW Build version: %s\n", str);
    }
}


static void read_print_debug_msg(struct slk_conf *conf)
{
    int ret;
    usb_dev_handle *h = conf->handle;
    unsigned char data[5] = {0,};
    unsigned char *buffer = malloc(1 * SZ_MB);
    struct slk_usb_req req = {
        .cmd = CMD_GET_DEBUG_BUF,
        .time_out = TIME_OUT,
    };

    ret = slk_init_device(conf);
    if (ret)
        goto error_out;

    ret = slk_usb_send_ctrl_cmd(conf, &req);
    if (ret < 0) {
        perror("slk_usb_send_cmd");
        goto error_out;
    }

    fprintf(stderr, "preparing for debug buffer read\n");

    if(!ret) {
            ret = usb_bulk_read(h, conf->ep.in, (char*)buffer, 1 * SZ_MB, TIME_OUT);
            if (ret < 0) {
                fprintf(stderr, "usb_bulk_read failed with : %d\n", ret);
                goto error_out;
            }
    }

    fprintf(stderr, "usb_bulk_read %d bytes \n", ret);
    buffer[ret] = '\0';
    fprintf(stderr, "%s\n", buffer);
    fflush(stderr);

    buffer[ret] = '\0';                         /* to be safer */

    return;
error_out:
    fprintf(stderr, "slk_usb_send_cmd failed with : %d\n", ret);
}

static void cleanup(struct slk_conf *conf)
{
    if (conf->handle) {
        fprintf(stderr, "Cleaning up \n");
        usb_release_interface(conf->handle, 0);
        usb_set_configuration(conf->handle, 0);
        usb_close(conf->handle);
    }
}

int sensor_init(struct slk_conf *conf)
{
    
}

#if 0
int main(int argc, char *argv[])
{
    int i;
    struct slk_conf *conf = malloc(sizeof(struct slk_conf));

    if (!conf) {
        fprintf(stderr, "Unable to allocate memory, Exiting...\n");
        return EXIT_FAILURE;
    }

    slk_find_device(conf);

    if (!conf->dev) {
        fprintf(stderr,
                "Unable to find USB device vendorid:%x devid:%x\n",
                ZKFP_VID, SILK20R);
        goto out;
    }

    sensor_init(conf);
    read_print_fw_version(conf);
    read_print_build_version(conf);
    //read_print_debug_msg(conf);
    for (i = 0; i < 10; i++) {
        printf("%d: ", i);
        read_print_global_var(conf);
        fflush(stdout);
    }

    cleanup(conf);

    return EXIT_SUCCESS;

out:
    return EXIT_FAILURE;
}
#endif

