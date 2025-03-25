/**
 * @file input.c
 * @brief Implementation of input handling system
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "input.h"

bool IsTouchAvailable(void) {
    return false; // Stub implementation for desktop
}

 // Singleton instance for global access
static InputManager* gInputManager = NULL;

/**
 * @brief Get global input manager instance
 *
 * @return InputManager* Pointer to the global input manager
 */
InputManager* GetInputManager(void) {
    if (gInputManager == NULL) {
        TraceLog(LOG_WARNING, "Trying to access InputManager before initialization");
    }
    return gInputManager;
}

/**
 * @brief Set global input manager instance
 *
 * @param manager Pointer to input manager
 */
void SetInputManager(InputManager* manager) {
    gInputManager = manager;
}

/**
 * @brief Create a new input manager
 *
 * @param initialBindingCapacity Initial capacity for bindings
 * @return InputManager* Pointer to created input manager
 */
InputManager* InputManagerCreate(int initialBindingCapacity) {
    InputManager* manager = (InputManager*)malloc(sizeof(InputManager));
    if (!manager) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for input manager");
        return NULL;
    }

    // Allocate bindings array
    manager->bindings = (InputBinding*)malloc(sizeof(InputBinding) * initialBindingCapacity);
    if (!manager->bindings) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for input bindings");
        free(manager);
        return NULL;
    }

    // Allocate action state arrays
    manager->actionStates = (bool*)calloc(ACTION_COUNT, sizeof(bool));
    manager->prevActionStates = (bool*)calloc(ACTION_COUNT, sizeof(bool));
    manager->actionValues = (float*)calloc(ACTION_COUNT, sizeof(float));

    if (!manager->actionStates || !manager->prevActionStates || !manager->actionValues) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for action states");
        free(manager->bindings);
        free(manager->actionStates);
        free(manager->prevActionStates);
        free(manager->actionValues);
        free(manager);
        return NULL;
    }

    if (!manager->actionValues) {
        // Handle allocation failure
        TraceLog(LOG_ERROR, "Failed to allocate actionValues array");
        // Clean up other allocations
        free(manager->bindings);
        free(manager->actionStates);
        free(manager->prevActionStates);
        free(manager);
        return NULL;
    }

    // Initialize manager
    manager->bindingCount = 0;
    manager->bindingCapacity = initialBindingCapacity;
    manager->gamepadsConnected = 0;
    manager->touchSupported = IsTouchAvailable();
    manager->keyboardConnected = true; // Assume keyboard is always available

    // Set as global instance
    SetInputManager(manager);

    return manager;
}

/**
 * @brief Destroy input manager and free resources
 *
 * @param manager Pointer to input manager
 */
void InputManagerDestroy(InputManager* manager) {
    if (!manager) return;

    // Free resources
    free(manager->bindings);
    free(manager->actionStates);
    free(manager->prevActionStates);
    free(manager->actionValues);

    // Clear global reference if this is the current manager
    if (gInputManager == manager) {
        gInputManager = NULL;
    }

    // Free manager
    free(manager);
}

/**
 * @brief Update input state
 *
 * @param manager Pointer to input manager
 */
void InputManagerUpdate(InputManager* manager) {
    if (!manager) return;

    // Copy current states to previous states
    memcpy(manager->prevActionStates, manager->actionStates, sizeof(bool) * ACTION_COUNT);

    // Reset current states and values
    memset(manager->actionStates, 0, sizeof(bool) * ACTION_COUNT);
    memset(manager->actionValues, 0, sizeof(float) * ACTION_COUNT);

    // Update gamepad connection status
    manager->gamepadsConnected = 0;
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        if (IsGamepadAvailable(i)) {
            manager->gamepadsConnected++;
        }
    }

    // Process each binding
    for (int i = 0; i < manager->bindingCount; i++) {
        InputBinding* binding = &manager->bindings[i];
        bool isActive = false;
        float value = 0.0f;

        // Check input state based on device type
        switch (binding->deviceType) {
        case INPUT_DEVICE_KEYBOARD:
            if (binding->isAxis) {
                // Keyboard doesn't support axis directly, but we can simulate
                // (e.g., W/S for Y axis, A/D for X axis)
                if (binding->axisPositive) {
                    isActive = IsKeyDown(binding->inputId);
                }
                else {
                    isActive = IsKeyDown(binding->inputId);
                }
                value = isActive ? 1.0f : 0.0f;
            }
            else {
                isActive = IsKeyDown(binding->inputId);
                value = isActive ? 1.0f : 0.0f;
            }
            break;

        case INPUT_DEVICE_GAMEPAD:
            if (IsGamepadAvailable(binding->deviceId)) {
                if (binding->isAxis) {
                    float axisValue = GetGamepadAxisMovement(binding->deviceId, binding->inputId);

                    // Apply deadzone and threshold
                    if (binding->axisPositive && axisValue > binding->axisThreshold) {
                        isActive = true;
                        value = (axisValue - binding->axisThreshold) / (1.0f - binding->axisThreshold);
                    }
                    else if (!binding->axisPositive && axisValue < -binding->axisThreshold) {
                        isActive = true;
                        value = (-axisValue - binding->axisThreshold) / (1.0f - binding->axisThreshold);
                    }
                }
                else {
                    isActive = IsGamepadButtonDown(binding->deviceId, binding->inputId);
                    value = isActive ? 1.0f : 0.0f;
                }
            }
            break;

        case INPUT_DEVICE_TOUCH:
            if (manager->touchSupported) {
                // Touch handling would depend on your specific requirements
                // This is just a basic example
                if (GetTouchPointCount() > 0) {
                    // Check if touch is in a specific region
                    Vector2 touchPos = GetTouchPosition(0);

                    // Example: binding->inputId could represent a specific touch zone
                    // This is a simplified approach; real implementation would be more complex
                    Rectangle touchZone = { 0, 0, 0, 0 };

                    switch (binding->inputId) {
                    case 0: // Left zone
                        touchZone = (Rectangle){ 0, 0, (float)(GetScreenWidth() / 4), GetScreenHeight() };
                        break;
                    case 1: // Right zone
                        touchZone = (Rectangle){ GetScreenWidth() * 3 / 4, 0, GetScreenWidth() / 4, GetScreenHeight() };
                        break;
                    case 2: // Up zone
                        touchZone = (Rectangle){ 0, 0, GetScreenWidth(), GetScreenHeight() / 4 };
                        break;
                    case 3: // Down zone
                        touchZone = (Rectangle){ 0, GetScreenHeight() * 3 / 4, GetScreenWidth(), GetScreenHeight() / 4 };
                        break;
                        // Add more zones as needed
                    }

                    isActive = CheckCollisionPointRec(touchPos, touchZone);
                    value = isActive ? 1.0f : 0.0f;
                }
            }
            break;

        case INPUT_DEVICE_MOUSE:
            if (binding->isAxis) {
                switch (binding->inputId) {
                case 0: // X axis
                    value = GetMouseDelta().x / 10.0f; // Scale as needed
                    isActive = fabs(value) > binding->axisThreshold;
                    break;
                case 1: // Y axis
                    value = GetMouseDelta().y / 10.0f; // Scale as needed
                    isActive = fabs(value) > binding->axisThreshold;
                    break;
                case 2: // Wheel
                    value = GetMouseWheelMove();
                    isActive = fabs(value) > binding->axisThreshold;
                    break;
                }
            }
            else {
                isActive = IsMouseButtonDown(binding->inputId);
                value = isActive ? 1.0f : 0.0f;
            }
            break;

        default:
            break;
        }

        // Update action state if this input is active
        if (isActive) {
            manager->actionStates[binding->action] = true;

            // Update action value (take highest value if multiple bindings affect same action)
            if (fabs(value) > fabs(manager->actionValues[binding->action])) {
                manager->actionValues[binding->action] = value;
            }
        }
    }
}

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
) {
    if (!manager || action < 0 || action >= ACTION_COUNT) return false;

    // Check if we need to expand the bindings array
    if (manager->bindingCount >= manager->bindingCapacity) {
        int newCapacity = manager->bindingCapacity * 2;
        InputBinding* newBindings = (InputBinding*)realloc(
            manager->bindings,
            sizeof(InputBinding) * newCapacity
        );

        if (!newBindings) {
            TraceLog(LOG_ERROR, "Failed to expand bindings array");
            return false;
        }

        manager->bindings = newBindings;
        manager->bindingCapacity = newCapacity;
    }

    // Add new binding
    InputBinding* binding = &manager->bindings[manager->bindingCount];
    binding->action = action;
    binding->deviceType = deviceType;
    binding->deviceId = deviceId;
    binding->inputId = inputId;
    binding->isAxis = isAxis;
    binding->axisThreshold = axisThreshold;
    binding->axisPositive = axisPositive;

    manager->bindingCount++;
    return true;
}

/**
 * @brief Remove all bindings for an action
 *
 * @param manager Pointer to input manager
 * @param action Game action
 * @return int Number of bindings removed
 */
int InputManagerRemoveBindings(InputManager* manager, GameAction action) {
    if (!manager || action < 0 || action >= ACTION_COUNT) return 0;

    int removedCount = 0;
    int i = 0;

    // Iterate through bindings and remove any that match the action
    while (i < manager->bindingCount) {
        if (manager->bindings[i].action == action) {
            // Remove this binding by shifting the rest down
            for (int j = i; j < manager->bindingCount - 1; j++) {
                manager->bindings[j] = manager->bindings[j + 1];
            }

            manager->bindingCount--;
            removedCount++;
        }
        else {
            // Only increment if we didn't remove, since removal shifts everything
            i++;
        }
    }

    return removedCount;
}

/**
 * @brief Check if action is currently active
 *
 * @param manager Pointer to input manager
 * @param action Game action
 * @return bool Whether action is active
 */
bool InputManagerIsActionActive(InputManager* manager, GameAction action) {
    if (!manager || action < 0 || action >= ACTION_COUNT) return false;
    return manager->actionStates[action];
}

/**
 * @brief Check if action was just pressed
 *
 * @param manager Pointer to input manager
 * @param action Game action
 * @return bool Whether action was just pressed
 */
bool InputManagerIsActionJustPressed(InputManager* manager, GameAction action) {
    if (!manager || action < 0 || action >= ACTION_COUNT) return false;
    return manager->actionStates[action] && !manager->prevActionStates[action];
}

/**
 * @brief Check if action was just released
 *
 * @param manager Pointer to input manager
 * @param action Game action
 * @return bool Whether action was just released
 */
bool InputManagerIsActionJustReleased(InputManager* manager, GameAction action) {
    if (!manager || action < 0 || action >= ACTION_COUNT) return false;
    return !manager->actionStates[action] && manager->prevActionStates[action];
}

/**
 * @brief Get analog value of action
 *
 * @param manager Pointer to input manager
 * @param action Game action
 * @return float Analog value of action (0.0-1.0)
 */
float InputManagerGetActionValue(InputManager* manager, GameAction action) {
    if (!manager) return 0.0f;
    if (!manager->actionValues) return 0.0f;
    if (action < 0 || action >= ACTION_COUNT) return 0.0f;

    return manager->actionValues[action];
}

/**
 * @brief Get movement vector from directional actions
 *
 * @param manager Pointer to input manager
 * @return Vector2 Normalized movement vector
 */
Vector2 InputManagerGetMovementVector(InputManager* manager) {
    if (!manager) return (Vector2) { 0, 0 };

    // Get raw movement values
    Vector2 movement = {
        InputManagerGetActionValue(manager, ACTION_MOVE_RIGHT) -
        InputManagerGetActionValue(manager, ACTION_MOVE_LEFT),

        InputManagerGetActionValue(manager, ACTION_MOVE_DOWN) -
        InputManagerGetActionValue(manager, ACTION_MOVE_UP)
    };

    // Normalize if length > 1.0
    float length = sqrtf(movement.x * movement.x + movement.y * movement.y);
    if (length > 1.0f) {
        movement.x /= length;
        movement.y /= length;
    }

    return movement;
}

/**
 * @brief Load default bindings
 *
 * @param manager Pointer to input manager
 */
void InputManagerLoadDefaultBindings(InputManager* manager) {
    if (!manager) return;

    // Clear existing bindings
    manager->bindingCount = 0;

    // Keyboard bindings
    InputManagerAddBinding(manager, ACTION_MOVE_UP, INPUT_DEVICE_KEYBOARD, 0, KEY_W, false, 0, false);
    InputManagerAddBinding(manager, ACTION_MOVE_LEFT, INPUT_DEVICE_KEYBOARD, 0, KEY_A, false, 0, false);
    InputManagerAddBinding(manager, ACTION_MOVE_DOWN, INPUT_DEVICE_KEYBOARD, 0, KEY_S, false, 0, false);
    InputManagerAddBinding(manager, ACTION_MOVE_RIGHT, INPUT_DEVICE_KEYBOARD, 0, KEY_D, false, 0, false);
    InputManagerAddBinding(manager, ACTION_ATTACK, INPUT_DEVICE_KEYBOARD, 0, KEY_SPACE, false, 0, false);
    InputManagerAddBinding(manager, ACTION_SPECIAL, INPUT_DEVICE_KEYBOARD, 0, KEY_LEFT_SHIFT, false, 0, false);
    InputManagerAddBinding(manager, ACTION_INTERACT, INPUT_DEVICE_KEYBOARD, 0, KEY_E, false, 0, false);
    InputManagerAddBinding(manager, ACTION_PAUSE, INPUT_DEVICE_KEYBOARD, 0, KEY_ESCAPE, false, 0, false);
    InputManagerAddBinding(manager, ACTION_MENU, INPUT_DEVICE_KEYBOARD, 0, KEY_TAB, false, 0, false);
    InputManagerAddBinding(manager, ACTION_RESET, INPUT_DEVICE_KEYBOARD, 0, KEY_R, false, 0, false);

    // Arrow keys as alternative movement
    InputManagerAddBinding(manager, ACTION_MOVE_UP, INPUT_DEVICE_KEYBOARD, 0, KEY_UP, false, 0, false);
    InputManagerAddBinding(manager, ACTION_MOVE_LEFT, INPUT_DEVICE_KEYBOARD, 0, KEY_LEFT, false, 0, false);
    InputManagerAddBinding(manager, ACTION_MOVE_DOWN, INPUT_DEVICE_KEYBOARD, 0, KEY_DOWN, false, 0, false);
    InputManagerAddBinding(manager, ACTION_MOVE_RIGHT, INPUT_DEVICE_KEYBOARD, 0, KEY_RIGHT, false, 0, false);

    // Gamepad bindings (for first gamepad)
    float axisThreshold = 0.2f; // Ignore small movements

    // Left stick movement
    InputManagerAddBinding(manager, ACTION_MOVE_RIGHT, INPUT_DEVICE_GAMEPAD, 0, GAMEPAD_AXIS_LEFT_X, true, axisThreshold, true);
    InputManagerAddBinding(manager, ACTION_MOVE_LEFT, INPUT_DEVICE_GAMEPAD, 0, GAMEPAD_AXIS_LEFT_X, true, axisThreshold, false);
    InputManagerAddBinding(manager, ACTION_MOVE_DOWN, INPUT_DEVICE_GAMEPAD, 0, GAMEPAD_AXIS_LEFT_Y, true, axisThreshold, true);
    InputManagerAddBinding(manager, ACTION_MOVE_UP, INPUT_DEVICE_GAMEPAD, 0, GAMEPAD_AXIS_LEFT_Y, true, axisThreshold, false);

    // D-pad movement
    InputManagerAddBinding(manager, ACTION_MOVE_UP, INPUT_DEVICE_GAMEPAD, 0, GAMEPAD_BUTTON_LEFT_FACE_UP, false, 0, false);
    InputManagerAddBinding(manager, ACTION_MOVE_LEFT, INPUT_DEVICE_GAMEPAD, 0, GAMEPAD_BUTTON_LEFT_FACE_LEFT, false, 0, false);
    InputManagerAddBinding(manager, ACTION_MOVE_DOWN, INPUT_DEVICE_GAMEPAD, 0, GAMEPAD_BUTTON_LEFT_FACE_DOWN, false, 0, false);
    InputManagerAddBinding(manager, ACTION_MOVE_RIGHT, INPUT_DEVICE_GAMEPAD, 0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT, false, 0, false);

    // Other gamepad actions
    InputManagerAddBinding(manager, ACTION_ATTACK, INPUT_DEVICE_GAMEPAD, 0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN, false, 0, false);
    InputManagerAddBinding(manager, ACTION_SPECIAL, INPUT_DEVICE_GAMEPAD, 0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, false, 0, false);
    InputManagerAddBinding(manager, ACTION_INTERACT, INPUT_DEVICE_GAMEPAD, 0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT, false, 0, false);
    InputManagerAddBinding(manager, ACTION_PAUSE, INPUT_DEVICE_GAMEPAD, 0, GAMEPAD_BUTTON_MIDDLE_RIGHT, false, 0, false);
    InputManagerAddBinding(manager, ACTION_MENU, INPUT_DEVICE_GAMEPAD, 0, GAMEPAD_BUTTON_MIDDLE_LEFT, false, 0, false);
    InputManagerAddBinding(manager, ACTION_RESET, INPUT_DEVICE_GAMEPAD, 0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1, false, 0, false);

    // Mouse bindings
    InputManagerAddBinding(manager, ACTION_ATTACK, INPUT_DEVICE_MOUSE, 0, MOUSE_BUTTON_LEFT, false, 0, false);
    InputManagerAddBinding(manager, ACTION_SPECIAL, INPUT_DEVICE_MOUSE, 0, MOUSE_BUTTON_RIGHT, false, 0, false);

    // Touch bindings for mobile (very simplified example)
    if (manager->touchSupported) {
        // Define virtual touch zones (indices 0-3 for left, right, up, down)
        InputManagerAddBinding(manager, ACTION_MOVE_LEFT, INPUT_DEVICE_TOUCH, 0, 0, false, 0, false);
        InputManagerAddBinding(manager, ACTION_MOVE_RIGHT, INPUT_DEVICE_TOUCH, 0, 1, false, 0, false);
        InputManagerAddBinding(manager, ACTION_MOVE_UP, INPUT_DEVICE_TOUCH, 0, 2, false, 0, false);
        InputManagerAddBinding(manager, ACTION_MOVE_DOWN, INPUT_DEVICE_TOUCH, 0, 3, false, 0, false);
    }
}

/**
 * @brief Save bindings to file
 *
 * @param manager Pointer to input manager
 * @param filename Path to save file
 * @return bool Whether save was successful
 */
bool InputManagerSaveBindings(InputManager* manager, const char* filename) {
    if (!manager || !filename) return false;

    // Open file for writing
    FILE* file = fopen(filename, "wb");
    if (!file) {
        TraceLog(LOG_ERROR, "Failed to open file for writing: %s", filename);
        return false;
    }

    // Write binding count
    fwrite(&manager->bindingCount, sizeof(int), 1, file);

    // Write bindings
    fwrite(manager->bindings, sizeof(InputBinding), manager->bindingCount, file);

    // Close file
    fclose(file);

    return true;
}

/**
 * @brief Load bindings from file
 *
 * @param manager Pointer to input manager
 * @param filename Path to bindings file
 * @return bool Whether load was successful
 */
bool InputManagerLoadBindings(InputManager* manager, const char* filename) {
    if (!manager || !filename) return false;

    // Open file for reading
    FILE* file = fopen(filename, "rb");
    if (!file) {
        TraceLog(LOG_ERROR, "Failed to open file for reading: %s", filename);
        return false;
    }

    // Read binding count
    int bindingCount;
    if (fread(&bindingCount, sizeof(int), 1, file) != 1) {
        TraceLog(LOG_ERROR, "Failed to read binding count from file: %s", filename);
        fclose(file);
        return false;
    }

    // Check if we need to expand the bindings array
    if (bindingCount > manager->bindingCapacity) {
        InputBinding* newBindings = (InputBinding*)realloc(
            manager->bindings,
            sizeof(InputBinding) * bindingCount
        );

        if (!newBindings) {
            TraceLog(LOG_ERROR, "Failed to expand bindings array");
            fclose(file);
            return false;
        }

        manager->bindings = newBindings;
        manager->bindingCapacity = bindingCount;
    }

    // Read bindings
    if (fread(manager->bindings, sizeof(InputBinding), bindingCount, file) != bindingCount) {
        TraceLog(LOG_ERROR, "Failed to read bindings from file: %s", filename);
        fclose(file);
        return false;
    }

    // Update binding count
    manager->bindingCount = bindingCount;

    // Close file
    fclose(file);

    return true;
}