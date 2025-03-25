/**
 * @file input.h
 * @brief Input handling system
 *
 * This file defines the input system for the game,
 * handling keyboard, gamepad, and touch input across platforms.
 */

#ifndef MESSY_GAME_INPUT_H
#define MESSY_GAME_INPUT_H

#include <stdbool.h>
#include "raylib.h"

#define MAX_GAMEPADS 2

 /**
  * @brief Game actions enumeration
  *
  * Defines all possible game actions that can be triggered by input.
  */
typedef enum {
    ACTION_NONE = 0,
    ACTION_MOVE_UP,
    ACTION_MOVE_DOWN,
    ACTION_MOVE_LEFT,
    ACTION_MOVE_RIGHT,
    ACTION_ATTACK,
    ACTION_SPECIAL,
    ACTION_INTERACT,
    ACTION_PAUSE,
    ACTION_MENU,
    ACTION_RESET,
    // Add more actions as needed
    ACTION_COUNT
} GameAction;

/**
 * @brief Input device types
 *
 * Defines the types of input devices supported.
 */
typedef enum {
    INPUT_DEVICE_KEYBOARD,
    INPUT_DEVICE_GAMEPAD,
    INPUT_DEVICE_TOUCH,
    INPUT_DEVICE_MOUSE,
    // Add more device types as needed
    INPUT_DEVICE_COUNT
} InputDeviceType;

/**
 * @brief Input binding structure
 *
 * Maps a specific input to a game action.
 */
typedef struct {
    GameAction action;           // Game action
    InputDeviceType deviceType;  // Device type
    int deviceId;                // Device ID (e.g., gamepad number)
    int inputId;                 // Input ID (e.g., key, button, or axis)
    bool isAxis;                 // Whether input is an axis (vs button)
    float axisThreshold;         // Threshold for axis (if applicable)
    bool axisPositive;           // Whether positive axis triggers action
    // Add more binding attributes as needed
} InputBinding;

/**
 * @brief Input manager structure
 *
 * Manages input state and bindings.
 */
typedef struct {
    InputBinding* bindings;      // Array of input bindings
    int bindingCount;            // Number of bindings
    int bindingCapacity;         // Capacity of bindings array
    bool* actionStates;          // Current state of each action
    bool* prevActionStates;      // Previous state of each action
    float* actionValues;         // Analog values of actions (0.0-1.0)
    int gamepadsConnected;       // Number of connected gamepads
    bool touchSupported;         // Whether touch is supported
    bool keyboardConnected;      // Whether keyboard is connected
    // Add more input manager attributes as needed
} InputManager;


bool IsTouchAvailable(void);

/**
 * @brief Create a new input manager
 *
 * @param initialBindingCapacity Initial capacity for bindings
 * @return InputManager* Pointer to created input manager
 */
InputManager* InputManagerCreate(int initialBindingCapacity);

/**
 * @brief Destroy input manager and free resources
 *
 * @param manager Pointer to input manager
 */
void InputManagerDestroy(InputManager* manager);

/**
 * @brief Update input state
 *
 * @param manager Pointer to input manager
 */
void InputManagerUpdate(InputManager* manager);

/**
 * @brief Add an input binding
 *
 * @param manager Pointer to input manager
 * @param action Game action
 * @param deviceType Device type
 * @param deviceId Device ID
 * @param inputId Input ID
 * @param isAxis Whether input is an axis
 * @param axisThreshold Threshold for axis (if applicable)
 * @param axisPositive Whether positive axis triggers action
 * @return bool Whether binding was added successfully
 */
bool InputManagerAddBinding(
    InputManager* manager,
    GameAction action,
    InputDeviceType deviceType,
    int deviceId,
    int inputId,
    bool isAxis,
    float axisThreshold,
    bool axisPositive
);

/**
 * @brief Remove all bindings for an action
 *
 * @param manager Pointer to input manager
 * @param action Game action
 * @return int Number of bindings removed
 */
int InputManagerRemoveBindings(InputManager* manager, GameAction action);

/**
 * @brief Check if action is currently active
 *
 * @param manager Pointer to input manager
 * @param action Game action
 * @return bool Whether action is active
 */
bool InputManagerIsActionActive(InputManager* manager, GameAction action);

/**
 * @brief Check if action was just pressed
 *
 * @param manager Pointer to input manager
 * @param action Game action
 * @return bool Whether action was just pressed
 */
bool InputManagerIsActionJustPressed(InputManager* manager, GameAction action);

/**
 * @brief Check if action was just released
 *
 * @param manager Pointer to input manager
 * @param action Game action
 * @return bool Whether action was just released
 */
bool InputManagerIsActionJustReleased(InputManager* manager, GameAction action);

/**
 * @brief Get analog value of action
 *
 * @param manager Pointer to input manager
 * @param action Game action
 * @return float Analog value of action (0.0-1.0)
 */
float InputManagerGetActionValue(InputManager* manager, GameAction action);

/**
 * @brief Get movement vector from directional actions
 *
 * @param manager Pointer to input manager
 * @return Vector2 Normalized movement vector
 */
Vector2 InputManagerGetMovementVector(InputManager* manager);

/**
 * @brief Load default bindings
 *
 * @param manager Pointer to input manager
 */
void InputManagerLoadDefaultBindings(InputManager* manager);

/**
 * @brief Save bindings to file
 *
 * @param manager Pointer to input manager
 * @param filename Path to save file
 * @return bool Whether save was successful
 */
bool InputManagerSaveBindings(InputManager* manager, const char* filename);

/**
 * @brief Load bindings from file
 *
 * @param manager Pointer to input manager
 * @param filename Path to bindings file
 * @return bool Whether load was successful
 */
bool InputManagerLoadBindings(InputManager* manager, const char* filename);

InputManager* GetInputManager(void);

#endif // MESSY_GAME_INPUT_H