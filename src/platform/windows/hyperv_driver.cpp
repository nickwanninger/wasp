#include <winnt.h>
#include <WinHvPlatform.h>
#include <winerror.h>
#include <stdexcept>
#include "compiler_defs.h"
#include "platform/windows/hyperv_vcpu.h"
#include "platform/windows/hyperv_driver.h"

using namespace mobo;

hyperv_driver::hyperv_driver(uint32_t num_cpus)
    : num_cpus_(num_cpus)
    , cpus_()
{
  ensure_capability_or_throw();

  WHV_PARTITION_HANDLE handle = create_partition();
  set_num_cpus(handle, num_cpus);
  setup_partition(handle);

  handle_ = handle;

  for (uint32_t i = 0; i < num_cpus; i++) {
    hyperv_vcpu cpu(handle_, i);
    cpus_.push_back(cpu);
  }
}

uint32_t hyperv_driver::num_cpus()
{
  return num_cpus_;
}

mobo::vcpu::ptr hyperv_driver::cpu(uint32_t index)
{
  if (index >= num_cpus_) {
    throw std::out_of_range("index out of range of number of cpus");
  }

  return std::make_shared<mobo::vcpu>(cpus_[index]);
}

void hyperv_driver::ensure_capability_or_throw()
{
  WHV_CAPABILITY capability = {};
  HRESULT hr;
  WHV_CAPABILITY_CODE code;
  uint32_t bytes_written;

  code = WHvCapabilityCodeHypervisorPresent;
  hr = WHvGetCapability(code, &capability, sizeof(capability), &bytes_written);
  if (FAILED(hr)) {
    throw std::runtime_error("failed to create Hyper-V driver: feature not enabled");
  }

  code = WHvCapabilityCodeFeatures;
  hr = WHvGetCapability(code, &capability, sizeof(capability), &bytes_written);
  if (FAILED(hr)) {
    throw std::runtime_error("failed to create Hyper-V driver: feature not enabled");
  }
}

WHV_PARTITION_HANDLE hyperv_driver::create_partition()
{
  WHV_PARTITION_HANDLE handle;
  HRESULT hr = WHvCreatePartition(&handle);
  if (FAILED(hr)) {
    throw std::runtime_error("failed to create Hyper-V driver: WHvCreatePartition failed");
  }

  return handle;
}

void hyperv_driver::setup_partition(WHV_PARTITION_HANDLE handle)
{
  HRESULT hr = WHvSetupPartition(handle);
  if (FAILED(hr)) {
    throw std::runtime_error("failed to create Hyper-V driver: WHvSetupPartition failed");
  }
}

void hyperv_driver::set_num_cpus(WHV_PARTITION_HANDLE handle, uint32_t num_cpus)
{
  WHV_PARTITION_PROPERTY_CODE code = WHvPartitionPropertyCodeProcessorCount;
  WHV_PARTITION_PROPERTY property = { .ProcessorCount = num_cpus };
  set_partition_property(handle, code, property);
}

void hyperv_driver::set_partition_property(
    WHV_PARTITION_HANDLE handle,
    WHV_PARTITION_PROPERTY_CODE code,
    WHV_PARTITION_PROPERTY &property)
{
  HRESULT hr = WHvSetPartitionProperty(handle, code, (const void *) &property, sizeof(property));
  if (FAILED(hr)) {
    throw std::runtime_error("failed to set Hyper-V machine property: WHvSetPartitionProperty failed");
  }
}
