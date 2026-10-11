/*
  Copyright (c) 2026 Schildkroet

  This file is part of cangaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "core/ConfigurableWidget.h"
#include "core/Backend.h"

class QComboBox;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QLabel;

class CmdWindow : public ConfigurableWidget
{
    Q_OBJECT

public:
    explicit CmdWindow(QWidget *parent, Backend &backend);
    ~CmdWindow() override = default;

    bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root) override;
    bool loadXML(Backend &backend, QDomElement &el) override;

protected:
    void retranslateUi() override;

private slots:
    void onSend();
    void updateInterfaces();
    void onPresetSelected(int index);

private:
    Backend &_backend;
    QComboBox *_comboInterface;
    QComboBox *_comboPresets;
    QLineEdit *_editCmd;
    QLineEdit *_editP1;
    QLineEdit *_editP2;
    QLineEdit *_editP3;
    QPushButton *_btnSend;
    QPushButton *_btnClear;
    QTextEdit *_logText;

    void appendLog(const QString &msg, bool isError = false);
};

