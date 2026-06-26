#include "hid_wrappers.h"

static libusb_context *g_usb_ctx = NULL;

int hid_init(void) {
    return libusb_init(&g_usb_ctx);
}

int hid_exit(void) {
    if (g_usb_ctx) {
        libusb_exit(g_usb_ctx);
        g_usb_ctx = NULL;
    }
    return 0;
}

const wchar_t* hid_error(hid_device *dev) {
    return L"USB Control Transfer Error (Run utility with sudo)";
}

void hid_close(hid_device *dev) {
    if (dev) {
        libusb_release_interface(dev, 0);
        libusb_close(dev);
    }
}

hid_device* hid_open(uint16_t vendor_id, uint16_t product_id, const wchar_t *serial_number) {
    if (!g_usb_ctx) {
        if (hid_init() != 0) return NULL;
    }
    libusb_device_handle *handle = libusb_open_device_with_vid_pid(g_usb_ctx, vendor_id, product_id);
    if (!handle) return NULL;

    // Claim interface 0 (Sonix bootloaders expose flash capabilities here)
    if (libusb_claim_interface(handle, 0) < 0) {
        libusb_close(handle);
        return NULL;
    }
    return handle;
}

int hid_send_feature_report(hid_device *dev, const unsigned char *data, size_t length) {
    if (length == 0) return -1;
    unsigned char report_id = data[0];
    const unsigned char *payload = data;
    size_t payload_len = length;

    // Strip report ID 0 if present, matching hidapi's standard behavior
    if (report_id == 0x00) {
        payload++;
        payload_len--;
    }

    int res = libusb_control_transfer(
        dev,
        0x21,                    // bmRequestType: Class | Interface | Out
        0x09,                    // bRequest: SET_REPORT
        (0x03 << 8) | report_id, // wValue: Feature Report Type (0x03) | Report ID
        0,                       // wIndex: Interface 0
        (unsigned char *)payload,
        payload_len,
        1000                     // Timeout in ms
    );

    if (res >= 0) {
        return (report_id == 0x00) ? (res + 1) : res;
    }
    return -1;
}

int hid_get_feature_report(hid_device *dev, unsigned char *data, size_t length) {
    if (length == 0) return -1;
    unsigned char report_id = data[0];

    int res = libusb_control_transfer(
        dev,
        0xA1,                    // bmRequestType: Class | Interface | In
        0x01,                    // bRequest: GET_REPORT
        (0x03 << 8) | report_id, // wValue: Feature Report Type (0x03) | Report ID
        0,                       // wIndex: Interface 0
        data + 1,                // Offset payload by 1 to preserve report ID byte position
        length - 1,
        1000                     // Timeout in ms
    );

    if (res >= 0) {
        data[0] = report_id;
        return res + 1;
    }
    return -1;
}
