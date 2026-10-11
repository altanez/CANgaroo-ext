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

#include "CmdWindow.h"
#include "driver/SLCANDriver/SLCANInterface.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QIntValidator>
#include <QTime>
#include <QDomDocument>
#include <QDomElement>
#include <QFont>

CmdWindow::CmdWindow(QWidget *parent, Backend &backend)
    : ConfigurableWidget(parent)
    , _backend(backend)
{
    setWindowTitle(tr("CMD View"));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    // ── Row 1: Interface & Presets ───────────────────────────────────────────
    auto *topLayout = new QHBoxLayout();
    topLayout->setSpacing(8);

    topLayout->addWidget(new QLabel(tr("Interface:"), this));
    _comboInterface = new QComboBox(this);
    _comboInterface->setMinimumWidth(180);
    topLayout->addWidget(_comboInterface);

    topLayout->addWidget(new QLabel(tr("Preset:"), this));
    _comboPresets = new QComboBox(this);
    _comboPresets->addItem(tr("(Select or enter custom)"), -1);
    _comboPresets->addItem(tr("0: WAKEUP"), 0);
    _comboPresets->addItem(tr("1: PCLOSE (Close Pack)"), 1);
    _comboPresets->addItem(tr("2: POPEN (Open Pack)"), 2);
    _comboPresets->addItem(tr("3: HSEARCH"), 3);
    _comboPresets->addItem(tr("4: SEARCH(P1)"), 4);
    _comboPresets->addItem(tr("7: BLOCK(P1)"), 7);
    _comboPresets->addItem(tr("8: SHARING"), 8);
    _comboPresets->addItem(tr("9: WOPEN(P1)"), 9);
    _comboPresets->addItem(tr("10: WCLOSE"), 10);
    _comboPresets->addItem(tr("14: AUTOCHARGE(P1)"), 14);
    _comboPresets->addItem(tr("15: TRCLOSE"), 15);
    _comboPresets->addItem(tr("16: TROPEN"), 16);
    _comboPresets->addItem(tr("17: CCON(P1)"), 17);
    _comboPresets->addItem(tr("18: CCNORM"), 18);
    _comboPresets->addItem(tr("19: CCCOOL"), 19);
    _comboPresets->addItem(tr("20: CCWARM"), 20);
    _comboPresets->addItem(tr("21: CCOFF"), 21);
    _comboPresets->addItem(tr("22: CCSETTEMP(P1)"), 22);
    _comboPresets->addItem(tr("25: CSYNC(P1)"), 25);
    _comboPresets->addItem(tr("26: CAUTO(P1)"), 26);
    _comboPresets->addItem(tr("27: MAXFRONT(P1)"), 27);
    _comboPresets->addItem(tr("30: STWHEAT(P1)"), 30);
    _comboPresets->addItem(tr("33: SHFL(P1)"), 33);
    _comboPresets->addItem(tr("34: SHFR(P1)"), 34);
    _comboPresets->addItem(tr("35: SFANFL(P1)"), 35);
    _comboPresets->addItem(tr("36: SFANFR(P1)"), 36);
    _comboPresets->addItem(tr("53: SHRL(P1)"), 53);
    _comboPresets->addItem(tr("54: SHRR(P1)"), 54);
    _comboPresets->addItem(tr("60: DEFROST(P1)"), 60);
    _comboPresets->addItem(tr("61: BCLOSE"), 61);
    _comboPresets->addItem(tr("62: BOPEN"), 62);
    _comboPresets->addItem(tr("74: OPENFD"), 74);
    _comboPresets->addItem(tr("75: OPENCD(P1)"), 75);
    _comboPresets->addItem(tr("92: CHBREAK"), 92);
    _comboPresets->addItem(tr("100: CHARGE_EN(P1)"), 100);
    _comboPresets->addItem(tr("121: CANRESET"), 121);
    _comboPresets->addItem(tr("222: Reboot to DFU"), 222);
    _comboPresets->addItem(tr("254: POWEROFF"), 254);
    topLayout->addWidget(_comboPresets, 1);

    mainLayout->addLayout(topLayout);

    // ── Row 2: Fields |CMD|P1|P2|P3| and |SEND| ──────────────────────────────
    auto *grpCmd = new QGroupBox(tr("Command Parameters (0..255)"), this);
    auto *cmdLayout = new QHBoxLayout(grpCmd);
    cmdLayout->setSpacing(8);

    auto *validator = new QIntValidator(0, 255, this);

    cmdLayout->addWidget(new QLabel(tr("CMD:"), grpCmd));
    _editCmd = new QLineEdit(grpCmd);
    _editCmd->setValidator(validator);
    _editCmd->setPlaceholderText("0");
    _editCmd->setAlignment(Qt::AlignCenter);
    _editCmd->setMinimumWidth(55);
    cmdLayout->addWidget(_editCmd);

    cmdLayout->addWidget(new QLabel(tr("P1:"), grpCmd));
    _editP1 = new QLineEdit(grpCmd);
    _editP1->setValidator(validator);
    _editP1->setPlaceholderText("0");
    _editP1->setAlignment(Qt::AlignCenter);
    _editP1->setMinimumWidth(55);
    cmdLayout->addWidget(_editP1);

    cmdLayout->addWidget(new QLabel(tr("P2:"), grpCmd));
    _editP2 = new QLineEdit(grpCmd);
    _editP2->setValidator(validator);
    _editP2->setPlaceholderText("0");
    _editP2->setAlignment(Qt::AlignCenter);
    _editP2->setMinimumWidth(55);
    cmdLayout->addWidget(_editP2);

    cmdLayout->addWidget(new QLabel(tr("P3:"), grpCmd));
    _editP3 = new QLineEdit(grpCmd);
    _editP3->setValidator(validator);
    _editP3->setPlaceholderText("0");
    _editP3->setAlignment(Qt::AlignCenter);
    _editP3->setMinimumWidth(55);
    cmdLayout->addWidget(_editP3);

    _btnSend = new QPushButton(tr("SEND"), grpCmd);
    _btnSend->setCursor(Qt::PointingHandCursor);
    _btnSend->setStyleSheet("font-weight: bold; min-width: 70px; padding: 4px 14px;");
    cmdLayout->addWidget(_btnSend);

    mainLayout->addWidget(grpCmd);

    // ── Row 3: Log output ────────────────────────────────────────────────────
    _logText = new QTextEdit(this);
    _logText->setReadOnly(true);
    QFont mono("Monospace");
    mono.setStyleHint(QFont::TypeWriter);
    _logText->setFont(mono);
    mainLayout->addWidget(_logText, 1);

    // ── Row 4: Clear log button ──────────────────────────────────────────────
    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    _btnClear = new QPushButton(tr("Clear Log"), this);
    bottomLayout->addWidget(_btnClear);
    mainLayout->addLayout(bottomLayout);

    // ── Connections ──────────────────────────────────────────────────────────
    connect(_btnSend, &QPushButton::clicked, this, &CmdWindow::onSend);
    connect(_btnClear, &QPushButton::clicked, _logText, &QTextEdit::clear);
    connect(_comboPresets, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CmdWindow::onPresetSelected);

    connect(_editCmd, &QLineEdit::returnPressed, this, &CmdWindow::onSend);
    connect(_editP1, &QLineEdit::returnPressed, this, &CmdWindow::onSend);
    connect(_editP2, &QLineEdit::returnPressed, this, &CmdWindow::onSend);
    connect(_editP3, &QLineEdit::returnPressed, this, &CmdWindow::onSend);

    connect(&_backend, &Backend::onSetupChanged, this, &CmdWindow::updateInterfaces);

    updateInterfaces();
}

void CmdWindow::updateInterfaces()
{
    _comboInterface->blockSignals(true);
    _comboInterface->clear();

    BusInterfaceId preferredId = 0;
    for (BusInterfaceId id : _backend.getInterfaceList())
    {
        BusInterface *intf = _backend.getInterfaceById(id);
        if (intf)
        {
            QString label = QString("%1 (%2)").arg(intf->getName(), intf->getDriver()->getName());
            _comboInterface->addItem(label, QVariant(id));
            if (intf->getDriver()->getName() == "SLCAN" && preferredId == 0)
            {
                preferredId = id;
            }
        }
    }

    if (preferredId != 0)
    {
        int idx = _comboInterface->findData(preferredId);
        if (idx >= 0)
            _comboInterface->setCurrentIndex(idx);
    }

    _comboInterface->blockSignals(false);
}

void CmdWindow::onPresetSelected(int index)
{
    int cmdVal = _comboPresets->itemData(index).toInt();
    if (cmdVal >= 0)
    {
        _editCmd->setText(QString::number(cmdVal));
    }
}

void CmdWindow::onSend()
{
    const uint8_t cmd = _editCmd->text().trimmed().isEmpty()
        ? 0 : static_cast<uint8_t>(qBound(0, _editCmd->text().trimmed().toInt(), 255));
    const uint8_t p1 = _editP1->text().trimmed().isEmpty()
        ? 0 : static_cast<uint8_t>(qBound(0, _editP1->text().trimmed().toInt(), 255));
    const uint8_t p2 = _editP2->text().trimmed().isEmpty()
        ? 0 : static_cast<uint8_t>(qBound(0, _editP2->text().trimmed().toInt(), 255));
    const uint8_t p3 = _editP3->text().trimmed().isEmpty()
        ? 0 : static_cast<uint8_t>(qBound(0, _editP3->text().trimmed().toInt(), 255));

    BusInterfaceId ifId = static_cast<BusInterfaceId>(_comboInterface->currentData().toUInt());
    BusInterface *intf = _backend.getInterfaceById(ifId);

    // If selected interface is not SLCAN, search for any available SLCAN interface
    SLCANInterface *slcan = dynamic_cast<SLCANInterface*>(intf);
    if (!slcan)
    {
        for (BusInterfaceId id : _backend.getInterfaceList())
        {
            auto *candidate = dynamic_cast<SLCANInterface*>(_backend.getInterfaceById(id));
            if (candidate)
            {
                slcan = candidate;
                break;
            }
        }
    }

    if (!slcan)
    {
        appendLog(tr("Error: No SLCAN interface available to send command"), true);
        return;
    }

    const QString timeStr = QTime::currentTime().toString("HH:mm:ss.zzz");
    const QString cmdStr = QString("K %1 %2 %3 %4").arg(cmd).arg(p1).arg(p2).arg(p3);

    if (slcan->sendGatewayCmd(cmd, p1, p2, p3))
    {
        appendLog(QString("[%1] %2 -> Sent via %3 [OK]").arg(timeStr, cmdStr, slcan->getName()));
    }
    else
    {
        appendLog(QString("[%1] %2 -> Failed to send via %3").arg(timeStr, cmdStr, slcan->getName()), true);
    }
}

void CmdWindow::appendLog(const QString &msg, bool isError)
{
    if (isError)
    {
        _logText->append(QString("<span style='color: #e53935;'>%1</span>").arg(msg.toHtmlEscaped()));
    }
    else
    {
        _logText->append(QString("<span style='color: #43a047;'>%1</span>").arg(msg.toHtmlEscaped()));
    }
}

void CmdWindow::retranslateUi()
{
    setWindowTitle(tr("CMD View"));
}

bool CmdWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    Q_UNUSED(backend);
    Q_UNUSED(xml);
    root.setAttribute("cmd", _editCmd->text());
    root.setAttribute("p1", _editP1->text());
    root.setAttribute("p2", _editP2->text());
    root.setAttribute("p3", _editP3->text());
    return true;
}

bool CmdWindow::loadXML(Backend &backend, QDomElement &el)
{
    Q_UNUSED(backend);
    if (el.hasAttribute("cmd"))
        _editCmd->setText(el.attribute("cmd"));
    if (el.hasAttribute("p1"))
        _editP1->setText(el.attribute("p1"));
    if (el.hasAttribute("p2"))
        _editP2->setText(el.attribute("p2"));
    if (el.hasAttribute("p3"))
        _editP3->setText(el.attribute("p3"));
    return true;
}

