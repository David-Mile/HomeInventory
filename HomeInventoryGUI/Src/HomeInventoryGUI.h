#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_HomeInventoryGUI.h"

class HomeInventoryGUI : public QMainWindow
{
    Q_OBJECT

public:
    HomeInventoryGUI(QWidget *parent = nullptr);
    ~HomeInventoryGUI();

private:
    Ui::HomeInventoryGUIClass ui;
};

