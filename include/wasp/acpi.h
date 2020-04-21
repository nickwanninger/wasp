/* 
 * This file is part of the Nautilus AeroKernel developed
 * by the Hobbes and V3VEE Projects with funding from the 
 * United States National  Science Foundation and the Department of Energy.  
 *
 * The V3VEE Project is a joint project between Northwestern University
 * and the University of New Mexico.  The Hobbes Project is a collaboration
 * led by Sandia National Laboratories that includes several national 
 * laboratories and universities. You can find out more at:
 * http://www.v3vee.org  and
 * http://xstack.sandia.gov/hobbes
 *
 * Copyright (c) 2015, Kyle C. Hale <kh@u.northwestern.edu>
 * Copyright (c) 2015, The V3VEE Project  <http://www.v3vee.org> 
 *                     The Hobbes Project <http://xstack.sandia.gov/hobbes>
 * All rights reserved.
 *
 * Author: Kyle C. Hale <kh@u.northwestern.edu>
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "LICENSE.txt".
 */
#ifndef __MOBO_ACPI_H__
#define __MOBO_ACPI_H__

#include <stdint.h>


#define MP_TAB_TYPE_CPU    0
#define MP_TAB_TYPE_BUS    1
#define MP_TAB_TYPE_IOAPIC 2
#define MP_TAB_TYPE_IO_INT 3
#define MP_TAB_TYPE_LINT   4

#define MP_TAB_CPU_LEN     20
#define MP_TAB_BUS_LEN     8
#define MP_TAB_IOAPIC_LEN  8
#define MP_TAB_IO_INT_LEN  8
#define MP_TAB_LINT_LEN    8


#define __packed __attribute__ ((packed))


struct mp_floating_pointer_structure {
    uint32_t signature;
    uint32_t configuration_table;
    uint8_t length; // In 16 bytes (e.g. 1 = 16 bytes, 2 = 32 bytes)
    uint8_t mp_specification_revision;
    uint8_t checksum; // This value should make all bytes in the table equal 0 when added together
    uint8_t default_configuration; // If this is not zero then configuration_table should be 
                                   // ignored and a default configuration should be loaded instead
    uint32_t features; // If bit 7 is then the IMCR is present and PIC mode is being used, otherwise 
                       // virtual wire mode is; all other bits are reserved
};

struct mp_configuration_table {
    uint32_t signature; // "PCMP"
    uint16_t length;
    uint8_t mp_specification_revision;
    uint8_t checksum; // Again, the byte should be all bytes in the table add up to 0
    char oem_id[8];
    char product_id[12];
    uint32_t oem_table;
    uint16_t oem_table_size;
    uint16_t entry_count; // This value represents how many entries are following this table
    uint32_t lapic_address; // This is the memory mapped address of the local APICs 
    uint16_t extended_table_length;
    uint8_t extended_table_checksum;
    uint8_t reserved;
};

struct mp_entry_processor {
    uint8_t type; // Always 0
    uint8_t local_apic_id;
    uint8_t local_apic_version;
    uint8_t flags; // If bit 0 is clear then the processor must be ignored
                   // If bit 1 is set then the processor is the bootstrap processor
    uint32_t signature;
    uint32_t feature_flags;
    uint64_t reserved;
};

struct entry_io_apic {
    uint8_t type; // Always 2
    uint8_t id;
    uint8_t version;
    uint8_t flags; // If bit 0 is set then the entry should be ignored
    uint32_t address; // The memory mapped address of the IO APIC is memory
};


enum acpi_irq_model_id {
	ACPI_IRQ_MODEL_PIC = 0,
	ACPI_IRQ_MODEL_IOAPIC,
	ACPI_IRQ_MODEL_IOSAPIC,
	ACPI_IRQ_MODEL_PLATFORM,
	ACPI_IRQ_MODEL_COUNT
};

extern enum acpi_irq_model_id	acpi_irq_model;

enum acpi_interrupt_id {
	ACPI_INTERRUPT_PMI	= 1,
	ACPI_INTERRUPT_INIT,
	ACPI_INTERRUPT_CPEI,
	ACPI_INTERRUPT_COUNT
};

#define	ACPI_SPACE_MEM		0

enum acpi_srat_entry_id {
	ACPI_SRAT_PROCESSOR_AFFINITY = 0,
	ACPI_SRAT_MEMORY_AFFINITY,
    ACPI_SRAT_X2APIC_CPU_AFFINITY,
	ACPI_SRAT_ENTRY_COUNT
};

enum acpi_address_range_id {
	ACPI_ADDRESS_RANGE_MEMORY = 1,
	ACPI_ADDRESS_RANGE_RESERVED = 2,
	ACPI_ADDRESS_RANGE_ACPI = 3,
	ACPI_ADDRESS_RANGE_NVS	= 4,
	ACPI_ADDRESS_RANGE_COUNT
};


#endif
