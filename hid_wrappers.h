#include <libusb.h>
#include <wchar.h>

typedef libusb_device_handle hid_device;

int hid_init(void);
int hid_exit(void);
const wchar_t* hid_error(hid_device *dev);
void hid_close(hid_device *dev);
hid_device* hid_open(uint16_t vendor_id, uint16_t product_id, const wchar_t *serial_number);
int hid_send_feature_report(hid_device *dev, const unsigned char *data, size_t length);
int hid_get_feature_report(hid_device *dev, unsigned char *data, size_t length);
