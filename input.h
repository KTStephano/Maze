#ifndef I_INPUT_H
#define I_INPUT_H

#define MAX_KEYS 255 // must be unique

typedef struct
{
  int keyID;
  char *bindings[2]; // commands to use with the cmd system
  char bIsDown;
  char bKeyUp; // if this generated a key up event in the last frame
  char bNoEvent; // if there was no noted change in the keystate
} i_key_t;

void i_Init(void);
void i_Shutdown(void);
void i_BindKey(int keyID, char *keyDown, char *keyUp); // make sure the bindings are registered with the cmd system
void i_RebindKey(int oldKey, int newKey); // will find the registered key and rebind it
void i_UnbindKey(int keyID);
void i_ProcessKeyInput(void); // call this once per frame
void i_KeyEvent(void);

#endif // I_INPUT_H
