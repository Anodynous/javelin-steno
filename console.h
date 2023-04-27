//---------------------------------------------------------------------------

#pragma once
#include "writer.h"
#include <stddef.h>
#if RUN_TESTS
#include <vector>
#endif

//---------------------------------------------------------------------------

struct ConsoleCommand {
  const char *command;
  const char *description;
  void (*handler)(void *context, const char *line);
  void *context;
};

//---------------------------------------------------------------------------

class ConsoleWriter final : public IWriter {
public:
  virtual void Write(const char *data, size_t length);

  static void WriteToActive(const char *data, size_t length) {
    classData.active->Write(data, length);
  }

  static void VprintfToActive(const char *p, va_list args) {
    classData.active->Vprintf(p, args);
  }

  static IWriter *GetActiveWriter() { return classData.active; }

  static void Push(IWriter *writer);
  static void Pop();

  static ConsoleWriter instance;

private:
  struct ClassData {
    IWriter *data[4];
    size_t count;
    IWriter *active;
  };
  static ClassData classData;
};

//---------------------------------------------------------------------------

class Console {
public:
  void HandleInput(const char *data, size_t length);
  static void SendOk();

  // Returns true if successful.
  static bool RunCommand(const char *command, IWriter &writer);

  static void RegisterCommand(const ConsoleCommand &command);
  static void RegisterCommand(const char *command, const char *description,
                              void (*handler)(void *context, const char *line),
                              void *context);

  static void HelloCommand(void *context, const char *line);
  static void HelpCommand(void *context, const char *line);

  static void Write(const char *data, size_t length) {
    ConsoleWriter::WriteToActive(data, length);
  }
  static void WriteAsJson(const char *data, char *buffer);
  static void WriteAsJson(const char *data);

  static void Printf(const char *format, ...)
      __attribute__((format(printf, 1, 2)));

  static void Flush();

#if RUN_TESTS
  static std::vector<char> history;
#endif

  static Console instance;

private:
  size_t lineBufferCount = 0;
  char lineBuffer[256];

  void ProcessLineBuffer();

  static const ConsoleCommand *GetCommand(const char *command);
};

//---------------------------------------------------------------------------
