#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "common/tusb_common.h"
#include "device/usbd.h"

enum {
    REPORT_ID_JOYSTICK = 1,
    REPORT_ID_LIGHTS,
};

// because they are missing from tusb_hid.h
#define HID_STRING_INDEX(x) HID_REPORT_ITEM(x, 7, RI_TYPE_LOCAL, 1)
#define HID_STRING_INDEX_N(x, n) HID_REPORT_ITEM(x, 7, RI_TYPE_LOCAL, n)
#define HID_STRING_MINIMUM(x) HID_REPORT_ITEM(x, 8, RI_TYPE_LOCAL, 1)
#define HID_STRING_MINIMUM_N(x, n) HID_REPORT_ITEM(x, 8, RI_TYPE_LOCAL, n)
#define HID_STRING_MAXIMUM(x) HID_REPORT_ITEM(x, 9, RI_TYPE_LOCAL, 1)
#define HID_STRING_MAXIMUM_N(x, n) HID_REPORT_ITEM(x, 9, RI_TYPE_LOCAL, n)

#define MUSEC_PICO_REPORT_DESC_JOYSTICK                                 \
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),                                  \
    HID_USAGE(HID_USAGE_DESKTOP_JOYSTICK),                                   \
    HID_COLLECTION(HID_COLLECTION_APPLICATION),                              \
        HID_REPORT_ID(REPORT_ID_JOYSTICK)                                    \
        HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),                               \
        HID_USAGE_MIN(1), HID_USAGE_MAX(8),                                  \
        HID_LOGICAL_MIN(0), HID_LOGICAL_MAX(1),                              \
        HID_REPORT_COUNT(8), HID_REPORT_SIZE(1),                             \
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),                   \
        HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),                              \
        HID_LOGICAL_MIN(0x00), HID_LOGICAL_MAX_N(0x00ff, 2),                 \
        HID_USAGE(HID_USAGE_DESKTOP_X), HID_USAGE(HID_USAGE_DESKTOP_Y),      \
        HID_USAGE(HID_USAGE_DESKTOP_Z),                                      \
        HID_USAGE(HID_USAGE_DESKTOP_RX), HID_USAGE(HID_USAGE_DESKTOP_RY),    \
        HID_USAGE(HID_USAGE_DESKTOP_RZ),                                     \
        HID_REPORT_COUNT(6), HID_REPORT_SIZE(8),                             \
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),                   \
    HID_COLLECTION_END

#define MUSEC_PICO_REPORT_DESC_LIGHTS                                      \
  HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),                                  \
  HID_USAGE(0x00),                                                         \
  HID_COLLECTION(HID_COLLECTION_APPLICATION),                              \
      HID_REPORT_ID(REPORT_ID_LIGHTS)                                      \
      HID_REPORT_COUNT(27), HID_REPORT_SIZE(8),                            \
      HID_LOGICAL_MIN(0x00), HID_LOGICAL_MAX_N(0x00ff, 2),                 \
      HID_USAGE_PAGE(HID_USAGE_PAGE_ORDINAL),                              \
      HID_STRING_MINIMUM(6), HID_STRING_MAXIMUM(32),                       \
      HID_USAGE_MIN(1), HID_USAGE_MAX(255),                                \
      HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),                  \
      HID_REPORT_COUNT(1),                                                 \
      HID_REPORT_SIZE(8), /*Padding*/                                      \
      HID_INPUT(HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE),               \
      HID_COLLECTION_END

#endif /* USB_DESCRIPTORS_H_ */
