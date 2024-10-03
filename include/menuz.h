#ifndef MENUZ_H
#define MENUZ_H

#include <Arduino.h>
#include <vector>
#include <stack>

typedef void (*MenuAction)();

typedef struct MenuItem {
    const char* label;
    std::vector<MenuItem> children;
    int* value;
    MenuAction actionUp;
    MenuAction actionDown;
} MenuItem;

class Menuz {
        std::stack<MenuItem> level;
        int currentIndex = 0;
        MenuItem back = {"Back", {}};
        boolean editing = false;
    public:

        Menuz(std::vector<MenuItem> items) {
            MenuItem root = {"Root", items};
            level.push(root);

            Serial.print("menuz init ");Serial.println(level.top().label);
            Serial.print("menuz init ");Serial.println(level.top().children.size());
        }

        void next() {
            Serial.print("menuz next ");Serial.println(level.top().label);
            Serial.print("menuz next ");Serial.println(level.top().children.size());
            if (!editing) {
                if (currentIndex < level.top().children.size()) {
                    currentIndex++;
                }
            } else {
                Serial.print("menuz actionUp");
                level.top().children.at(currentIndex).actionUp();
            }
            Serial.print("menuz currentIndex ");Serial.println(currentIndex);
        }

        void prev() {
            Serial.print("menuz prev ");Serial.println(level.top().label);
            Serial.print("menuz prev ");Serial.println(level.top().children.size());
            if (!editing) {
                if (currentIndex > 0) {
                    currentIndex--;
                }
            } else {
                Serial.print("menuz actionDown");
                level.top().children.at(currentIndex).actionDown();
            }
            Serial.print("menuz currentIndex ");Serial.println(currentIndex);
        }

        void select() {
            Serial.print("menuz select ");Serial.println(currentIndex);
            if (currentIndex < level.top().children.size()) {
                MenuItem* current = &level.top().children.at(currentIndex);
                if (current->children.size() > 0) {
                    // sub menu
                    level.push(*current);
                    currentIndex = 0;
                } else {
                    editing = !editing;
                }
            } else {
                Serial.print("menuz select back");
                // if (level.empty() ) {
                    Serial.print("menuz select size ");Serial.println(level.size());
                // }
                level.pop();
                currentIndex = 0;
            }
        }

        MenuItem* getCurrentItem() {
            if (currentIndex < level.top().children.size()) {
                return &level.top().children.at(currentIndex);
            } else {
                return &back;
            }
        }
};
#endif