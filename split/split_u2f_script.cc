//---------------------------------------------------------------------------

#include "split_u2f_script.h"

//---------------------------------------------------------------------------

#if JAVELIN_U2F && JAVELIN_SPLIT
#include "../button_manager.h"

//---------------------------------------------------------------------------

SplitU2fScript SplitU2fScript::instance;

//---------------------------------------------------------------------------

void SplitU2fScript::UpdateBuffer(TxBuffer &buffer) {
  if (!dirty) {
    return;
  }
  dirty = false;

  buffer.Add(SplitHandlerId::U2F_SCRIPT, &data, sizeof(data));
}

void SplitU2fScript::OnDataReceived(const void *data, size_t length) {
  memcpy(&this->data, data, sizeof(data));
  if (this->data.triggerNotifyUserPresenceChange) {
    ButtonManager::ExecuteScript(ScriptId::U2F_STATUS_UPDATE);
  }
}

//---------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------