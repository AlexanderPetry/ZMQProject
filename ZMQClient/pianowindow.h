#ifndef PIANOWINDOW_H
#define PIANOWINDOW_H

#include <QObject>
#include <QWidget>
#include <QKeyEvent>
#include <set>
#include <map>

extern int vel;
void send_note(int state, int note, int velocity);

class PianoWindow : public QWidget
{
    Q_OBJECT
public:
    std::map<int, int> keyMap = {
        {Qt::Key_A, 60},
        {Qt::Key_S, 62},
        {Qt::Key_D, 64},
        {Qt::Key_F, 65},
        {Qt::Key_G, 67},
        {Qt::Key_H, 69},
        {Qt::Key_J, 71}
    };
    std::set<int> held;

    PianoWindow(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAttribute(Qt::WA_KeyCompression, false);
    }

protected:
    void keyPressEvent(QKeyEvent *event) override {
        if (event->isAutoRepeat()) return;
        int key = event->key();
        if (keyMap.find(key) != keyMap.end() && held.find(key) == held.end()) {
            held.insert(key);
            send_note(1, keyMap[key], vel);
        }
    }

    void keyReleaseEvent(QKeyEvent *event) override {
        if (event->isAutoRepeat()) return;
        int key = event->key();
        if (keyMap.find(key) != keyMap.end()) {
            send_note(0, keyMap[key], vel);
            held.erase(key);
        }
    }
};

#endif // PIANOWINDOW_H
