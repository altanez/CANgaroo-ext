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

