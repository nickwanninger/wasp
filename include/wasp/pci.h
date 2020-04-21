#ifndef __PCI_H__
#define __PCI_H__

#include <wasp/types.h>

typedef enum {
  PCI_BAR_IO,
  PCI_BAR_MEM24,
  PCI_BAR_MEM32,
  PCI_BAR_MEM64_LOW,
  PCI_BAR_MEM64_HIGH,
  PCI_BAR_PASSTHROUGH,
  PCI_BAR_NONE
} pci_bar_type_t;

typedef enum {
  PCI_STD_DEVICE,
  PCI_TO_PCI_BRIDGE,
  PCI_CARDBUS,
  PCI_MULTIFUNCTION,
  PCI_PASSTHROUGH
} pci_device_type_t;

struct pci_device;

struct v3_pci_bar {
  pci_bar_type_t type;

  union {
    struct {
      int num_pages;
      u64 default_base_addr;
      int (*mem_read)(u64 guest_addr, void* dst, u32 length,
                      void* private_data);
      int (*mem_write)(u64 guest_addr, void* src, u32 length,
                       void* private_data);
    };

    struct {
      int num_ports;
      u16 default_base_port;
      int (*io_read)(u16 port, void* dst, u32 length, void* private_data);
      int (*io_write)(u16 port, void* src, u16 length, void* private_data);
    };

    struct {
      int (*bar_init)(int bar_num, uint32_t* dst, void* private_data);
      int (*bar_write)(int bar_num, uint32_t* src, void* private_data);
    };
  };

  void* private_data;

  // Internal PCI data
  uint32_t val;
  int updated;
  uint32_t mask;
};

struct pci_config_header {
  uint16_t vendor_id;
  uint16_t device_id;

  uint16_t command;
  uint16_t status;

  uint8_t revision;
  uint8_t prog_if;
  uint8_t subclass;
  uint8_t class_id;

  uint8_t cache_line_size;
  uint8_t latency_time;
  uint8_t header_type;  // bits 6-0: 00: other, 01: pci-pci bridge, 02:
                        // pci-cardbus; bit 7: 1=multifunction
  uint8_t BIST;

  uint32_t BAR0;
  uint32_t BAR1;
  uint32_t BAR2;
  uint32_t BAR3;
  uint32_t BAR4;
  uint32_t BAR5;
  uint32_t cardbus_cis_pointer;
  uint16_t subsystem_vendor_id;
  uint16_t subsystem_id;
  uint32_t expansion_rom_address;
  uint8_t cap_ptr;  // capabilities list offset in config space
  uint8_t rsvd1[3];
  uint32_t rsvd2;
  uint8_t intr_line;    // 00=none, 01=IRQ1, etc.
  uint8_t intr_pin;     // 00=none, otherwise INTA# to INTD#
  uint8_t min_grant;    // min busmaster time - units of 250ns
  uint8_t max_latency;  // units of 250ns - busmasters
} __attribute__((packed));

#define PCI_IO_MASK 0xfffffffc
#define PCI_MEM_MASK 0xfffffff0
#define PCI_MEM24_MASK 0x000ffff0

#define PCI_IO_BASE(bar_val) (bar_val & PCI_IO_MASK)
#define PCI_MEM32_BASE(bar_val) (bar_val & PCI_MEM_MASK)
#define PCI_MEM24_BASE(bar_val) (bar_val & PCI_MEM24_MASK)

struct pci_device {
  pci_device_type_t type;

  union {
    uint8_t config_space[256];

    struct {
      struct pci_config_header config_header;
      uint8_t config_data[192];
    } __attribute__((packed));
  } __attribute__((packed));

  struct v3_pci_bar bar[6];

  struct rb_node dev_tree_node;

  u32 bus_num;

  union {
    uint8_t devfn;
    struct {
      uint8_t fn_num : 3;
      uint8_t dev_num : 5;
    } __attribute__((packed));
  } __attribute__((packed));

  char name[64];

  int (*config_update)(u32 reg_num, void* src, u32 length,
                       void* priv_data);

  int (*cmd_update)(struct pci_device* pci_dev, u8 io_enabled,
                    u8 mem_enabled);
  int (*ext_rom_update)(struct pci_device* pci_dev);

  int (*config_write)(u32 reg_num, void* src, u32 length,
                      void* private_data);
  int (*config_read)(u32 reg_num, void* dst, u32 length,
                     void* private_data);

  int ext_rom_update_flag;
  int bar_update_flag;

  void* priv_data;
};





#endif
