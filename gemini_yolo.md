# Инструкция: Включение автономного режима «YOLO Mode» в Antigravity / Gemini

Данная инструкция позволяет развернуть полноценный **YOLO-режим** (полная автономия агента без остановок, диалогов и запросов подтверждений) на любом компьютере.

---

## Что делает этот режим

1. **Команда `/yolo` в чате**: обучает модель действовать полностью автономно — без встречных вопросов (`ask_question`), самостоятельно выбирая технические решения, запуская сборку/тесты и исправляя ошибки до готового результата.
2. **Снятие подтверждений с терминала и файлов**: переключает политику среды в `CASCADE_COMMANDS_AUTO_EXECUTION_EAGER`, чтобы VS Code не показывал модальные окна «Approve / Run» на каждую команду.
3. **Авто-принятие правок кода**: автоматически применяет изменения без необходимости кликать мышкой по диффам.

---

## Вариант 1: Быстрая установка скриптом (Рекомендуется)

Откройте **PowerShell** на целевом компьютере и выполните скрипт (создаст глобальный скилл и обновит глобальный конфиг):

```powershell
# 1. Создаем папку для глобального скилла
$skillDir = "$env:USERPROFILE\.gemini\config\skills\yolo"
New-Item -ItemType Directory -Force -Path $skillDir | Out-Null

# 2. Создаем файл SKILL.md
@'
---
name: yolo
description: >-
  Autonomous execution mode (YOLO mode). Activates when the user types /yolo, yolo, or requests fully autonomous execution. Enforces non-interactive end-to-end task completion without asking clarifying questions or requesting manual confirmations.
---

# YOLO Autonomous Execution Mode

When this skill is activated (via `/yolo`, `yolo: <task>`, or user request for autonomous execution), follow these strict operational rules:

## 1. Zero-Interruption Execution
- **Never call `ask_question`**: Make competent, standard architectural and technical decisions autonomously based on codebase context.
- **Do not ask for confirmation**: Execute terminal commands, create/edit files, and apply fixes proactively.
- **Autonomous troubleshooting**: When encountering compilation, syntax, or runtime errors, diagnose and fix them immediately without waiting for user input.

## 2. End-to-End Workflow
1. **Understand & Inspect**: Examine relevant files, documentation, and dependencies without excessive chattiness.
2. **Execute Changes**: Directly write and edit the code. Maintain existing code style and conventions.
3. **Verify Proactively**: Run builds, compilers, tests, or linters to prove the implementation works. If something fails, iterate and fix it.
4. **Final Summary**: Only return control to the user when the task is fully completed or in a working, verified state. Provide a concise, clear summary of what was accomplished.
'@ | Out-File -FilePath "$skillDir\SKILL.md" -Encoding utf8

# 3. Включаем eager auto-execution в ~/.gemini/config/config.json
$configPath = "$env:USERPROFILE\.gemini\config\config.json"
if (Test-Path $configPath) {
    $cfg = Get-Content $configPath -Raw | ConvertFrom-Json
    if (-not $cfg.userSettings) { $cfg | Add-Member -NotePropertyName "userSettings" -NotePropertyValue ([PSCustomObject]@{}) }
    $cfg.userSettings.autoExecutionPolicy = "CASCADE_COMMANDS_AUTO_EXECUTION_EAGER"
    $cfg.userSettings.enableTerminalSandbox = $false
    $cfg | ConvertTo-Json -Depth 10 | Set-Content $configPath -Encoding utf8
}
Write-Host "✅ YOLO mode успешно настроен на этом ПК!" -ForegroundColor Green
```

---

## Вариант 2: Ручная пошаговая настройка

### Шаг 1. Глобальный навык `/yolo`
Создайте файл:
`~/.gemini/config/skills/yolo/SKILL.md` (на Windows: `%USERPROFILE%\.gemini\config\skills\yolo\SKILL.md`)

С содержимым:
```markdown
---
name: yolo
description: >-
  Autonomous execution mode (YOLO mode). Activates when the user types /yolo, yolo, or requests fully autonomous execution. Enforces non-interactive end-to-end task completion without asking clarifying questions or requesting manual confirmations.
---

# YOLO Autonomous Execution Mode

When this skill is activated (via `/yolo`, `yolo: <task>`, or user request for autonomous execution), follow these strict operational rules:

## 1. Zero-Interruption Execution
- **Never call `ask_question`**: Make competent, standard architectural and technical decisions autonomously based on codebase context.
- **Do not ask for confirmation**: Execute terminal commands, create/edit files, and apply fixes proactively.
- **Autonomous troubleshooting**: When encountering compilation, syntax, or runtime errors, diagnose and fix them immediately without waiting for user input.

## 2. End-to-End Workflow
1. **Understand & Inspect**: Examine relevant files, documentation, and dependencies without excessive chattiness.
2. **Execute Changes**: Directly write and edit the code. Maintain existing code style and conventions.
3. **Verify Proactively**: Run builds, compilers, tests, or linters to prove the implementation works. If something fails, iterate and fix it.
4. **Final Summary**: Only return control to the user when the task is fully completed or in a working, verified state. Provide a concise, clear summary of what was accomplished.
```

*(Также можно положить этот файл в репозиторий проекта по пути `.agents/skills/yolo/SKILL.md`, тогда он автоматически переедет вместе с `git pull`).*

---

### Шаг 2. Глобальная политика выполнения команд
В файле `~/.gemini/config/config.json` убедитесь, что в секции `userSettings` прописано:
```json
"userSettings": {
  "autoExecutionPolicy": "CASCADE_COMMANDS_AUTO_EXECUTION_EAGER",
  "enableTerminalSandbox": false
}
```

---

### Шаг 3. Политика конкретного проекта (если проект уже был открыт ранее)
Если проект уже был открыт в IDE, Antigravity создает для него файл в:
`~/.gemini/config/projects/<project-id>.json`.

Проверьте блок `"settings"` в этом файле. Он должен быть заполнен:
```json
  "settings": {
    "fileAccessPolicy": "AGENT_SETTING_POLICY_ALLOW",
    "sandboxMode": false,
    "autoExecutionPolicy": "CASCADE_COMMANDS_AUTO_EXECUTION_EAGER"
  }
```
*(Если `"settings": {}` пустой, проект будет повторно переспрашивать разрешения для новых команд).*

---

### Шаг 4. Настройки VS Code
В `.vscode/settings.json` проекта (или в глобальный `settings.json` VS Code):
```json
{
  "antigravity.autoAcceptOnChat": true
}
```

---

## Горячие клавиши (Hotkeys) в VS Code

Если какой-то шаг или операция всё же заблокировалась:
* **`Alt + Enter`** — мгновенно подтвердить текущий шаг агента (`antigravity.dynamic.acceptAgentStep`). Не нужно кликать мышью по кнопке.
* **`Alt + Shift + Enter`** — отклонить шаг агента (`antigravity.dynamic.rejectAgentStep`).
* **`Esc`** — прервать выполнение агента (`antigravity.dynamic.interruptAgent`).

При желании переназначить в `keybindings.json` (например, на `F5`):
```json
{
  "key": "f5",
  "command": "antigravity.dynamic.acceptAgentStep",
  "when": "antigravity.stepPending"
}
```

---

## Как использовать

Просто напишите в чате:
```text
/yolo добавь переключение каналов для SLCAN и собери проект
```
или
```text
yolo: реализуй шлюз согласно файлу TASK.md
```

Агент распознает команду, подключит навык `yolo` и выполнит задачу автономно от начала до конца.

