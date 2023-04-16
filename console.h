//---------------------------------------------------------------------------

#pragma once
#include "writer.h"
#include <stdlib.h>
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
    IWriter::WriteToStackTop(data, length);
  }
  static void WriteAsJson(const char *data, char *buffer);
  static void WriteAsJson(const char *data);

#if JAVELIN_PLATFORM_ZEPHYR_SDK
  static void Printf(const char *format, ...);
#else
  static void Printf(const char *format, ...) __printflike(1, 2);
#endif

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
