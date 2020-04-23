#pragma once

#include "elfio/elfio.hpp"
#include "./machine.h"

namespace wasp::loader {

class WASP_API binary_loader {
public:
  binary_loader() = default;
  virtual ~binary_loader() = default;

  virtual bool inject(wasp::machine &vm) = 0;
};


class WASP_API elf_loader : public binary_loader {
  ELFIO::elfio reader;

public:
  elf_loader(std::string path);
  bool inject(wasp::machine &vm) override;
};

class WASP_API flatbin_loader : public binary_loader {
  std::string path;

public:
  explicit flatbin_loader(std::string path);
  bool inject(wasp::machine &vm) override;
};

}