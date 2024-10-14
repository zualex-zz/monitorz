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

            SERIALZ.print("menuz init ");SERIALZ.println(level.top().label);
            SERIALZ.print("menuz init ");SERIALZ.println(level.top().children.size());
        }

        void next() {
            SERIALZ.print("menuz next ");SERIALZ.println(level.top().label);
            SERIALZ.print("menuz next ");SERIALZ.println(level.top().children.size());
            if (!editing) {
                if (currentIndex < level.top().children.size()) {
                    currentIndex++;
                }
            } else {
                SERIALZ.print("menuz actionUp");
                level.top().children.at(currentIndex).actionUp();
            }
            SERIALZ.print("menuz currentIndex ");SERIALZ.println(currentIndex);
        }

        void prev() {
            SERIALZ.print("menuz prev ");SERIALZ.println(level.top().label);
            SERIALZ.print("menuz prev ");SERIALZ.println(level.top().children.size());
            if (!editing) {
                if (currentIndex > 0) {
                    currentIndex--;
                }
            } else {
                SERIALZ.print("menuz actionDown");
                level.top().children.at(currentIndex).actionDown();
            }
            SERIALZ.print("menuz currentIndex ");SERIALZ.println(currentIndex);
        }

        void select() {
            SERIALZ.print("menuz select ");SERIALZ.println(currentIndex);
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
                SERIALZ.print("menuz select back");
                // if (level.empty() ) {
                    SERIALZ.print("menuz select size ");SERIALZ.println(level.size());
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