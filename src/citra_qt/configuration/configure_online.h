// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <QFutureWatcher>
#include <QWidget>

namespace Ui {
class ConfigureOnline;
}

class ConfigureOnline : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureOnline(QWidget* parent = nullptr);
    ~ConfigureOnline() override;

    void ApplyConfiguration();
    void RetranslateUI();
    void SetConfiguration();

private:
    std::unique_ptr<Ui::ConfigureOnline> ui;
};
