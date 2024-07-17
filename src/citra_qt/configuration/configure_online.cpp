// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QIcon>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrentRun>
#include "citra_qt/configuration/configure_online.h"
#include "citra_qt/uisettings.h"
#include "network/network_settings.h"
#include "ui_configure_online.h"

ConfigureOnline::ConfigureOnline(QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureOnline>()) {
    ui->setupUi(this);

#ifndef USE_DISCORD_PRESENCE
    ui->discord_group->setEnabled(false);
#endif
    SetConfiguration();
}

ConfigureOnline::~ConfigureOnline() = default;

void ConfigureOnline::SetConfiguration() {

    ui->toggle_discordrpc->setChecked(UISettings::values.enable_discord_presence.GetValue());
}

void ConfigureOnline::ApplyConfiguration() {
    UISettings::values.enable_discord_presence = ui->toggle_discordrpc->isChecked();
}

void ConfigureOnline::RetranslateUI() {
    ui->retranslateUi(this);
}
