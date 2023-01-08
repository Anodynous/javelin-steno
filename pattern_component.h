//---------------------------------------------------------------------------

#pragma once
#include "pool_allocate.h"
#include <assert.h>

//---------------------------------------------------------------------------

constexpr size_t PATTERN_COMPONENT_BLOCK_SIZE = 1024;

//---------------------------------------------------------------------------

struct PatternContext {
  const char *start;
  const char **captureList;
};

//---------------------------------------------------------------------------

class PatternComponent
    : public PoolAllocate<PatternComponent, PATTERN_COMPONENT_BLOCK_SIZE> {
public:
  virtual bool Match(const char *p, PatternContext &context) const = 0;
  virtual bool IsEpsilon() const { return false; }

  virtual void RemoveEpsilon();
  virtual uint32_t CreateAccelerator(uint32_t v = 0) const;

  static void *operator new(size_t size);

protected:
  bool CallNext(const char *p, PatternContext &context) const;
  const PatternComponent *GetNext() const { return next; }

private:
  PatternComponent *next = nullptr;

  friend class Pattern;
  friend class BranchPatternComponent;
  friend class AlternatePatternComponent;
};

class EpsilonPatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;
  virtual bool IsEpsilon() const { return true; }
};

class AnyPatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;
};

class AnyStarPatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;
};

class BackReferencePatternComponent : public PatternComponent {
public:
  BackReferencePatternComponent(int index) : index(index) {}

  virtual bool Match(const char *p, PatternContext &context) const;

private:
  int index;
};

class CharacterSetComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;

private:
  uint8_t mask[16] = {};

  void SetBit(int index) {
    assert(index < 128);
    int offset = index / 8;
    int bit = 1 << (index & 7);
    mask[offset] |= bit;
  }

  friend class Pattern;
};

class BranchPatternComponent : public PatternComponent {
public:
  BranchPatternComponent(PatternComponent *branch) : branch(branch) {}
  virtual bool Match(const char *p, PatternContext &context) const;
  virtual void RemoveEpsilon() final;

private:
  PatternComponent *branch;
  bool processed = false;
};

class StartOfLinePatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;
};

class EndOfLinePatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;
};

class CapturePatternComponent : public PatternComponent {
public:
  CapturePatternComponent(size_t index) : index(index) {}

  virtual bool Match(const char *p, PatternContext &context) const final;

private:
  size_t index;
};

class LiteralPatternComponent : public PatternComponent {
public:
  LiteralPatternComponent(const char *text) : text(text) {}

  virtual uint32_t CreateAccelerator(uint32_t v = 0) const;

  virtual bool Match(const char *p, PatternContext &context) const final;

private:
  const char *text;
};

class ContainerPatternComponent : public PatternComponent {
public:
  ContainerPatternComponent() : componentCount(0), components(nullptr) {}
  ContainerPatternComponent(PatternComponent *initialComponent);

  void Add(PatternComponent *component);
  virtual void RemoveEpsilon() final;

protected:
  size_t componentCount;
  PatternComponent **components;
};

class AlternatePatternComponent : public ContainerPatternComponent {
public:
  AlternatePatternComponent(PatternComponent *initialComponent)
      : ContainerPatternComponent(initialComponent) {}

  virtual bool Match(const char *p, PatternContext &context) const final;
};

//---------------------------------------------------------------------------
