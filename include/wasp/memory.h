#pragma once

namespace wasp {

namespace memory {

const uint64_t _1MB = 1 * 1024 * 1024;
const uint64_t _1GB = 1 * 1024 * 1024 * 1024;
const uint64_t _256GB = 256 * _1GB;
const uint64_t _512GB = 512 * _1GB;

//
// Store the PML4 (page map level 4) right below the 512GB boundary
//
const uint64_t PML4_PHYSICAL_ADDRESS = _512GB - _1GB;

}
}