<!--
Sync Impact Report
- Version change: template -> 1.0.0
- Modified principles:
	- Principle 1 template slot -> I. KISS-First Engineering
	- Principle 2 template slot -> II. Code Quality Baseline
	- Principle 3 template slot -> III. Test Evidence for Every Change
	- Principle 4 template slot -> IV. Consistent User Experience
	- Principle 5 template slot -> V. Performance Within Hardware Limits
- Added sections:
	- Project Constraints (Hobby Scope)
	- Development Workflow & Quality Gates
- Removed sections:
	- None
- Templates requiring updates:
	- ✅ .specify/templates/plan-template.md
	- ✅ .specify/templates/spec-template.md
	- ✅ .specify/templates/tasks-template.md
	- ✅ .github/prompts/speckit.analyze.prompt.md (reviewed, no change needed)
	- ✅ .github/prompts/speckit.checklist.prompt.md (reviewed, no change needed)
	- ✅ .github/prompts/speckit.clarify.prompt.md (reviewed, no change needed)
	- ✅ .github/prompts/speckit.constitution.prompt.md (reviewed, no change needed)
	- ✅ .github/prompts/speckit.implement.prompt.md (reviewed, no change needed)
	- ✅ .github/prompts/speckit.plan.prompt.md (reviewed, no change needed)
	- ✅ .github/prompts/speckit.specify.prompt.md (reviewed, no change needed)
	- ✅ .github/prompts/speckit.tasks.prompt.md (reviewed, no change needed)
	- ✅ .github/prompts/speckit.taskstoissues.prompt.md (reviewed, no change needed)
	- ✅ README.md (reviewed, no change needed)
- Follow-up TODOs:
	- None
-->

# ClockClock24 Replica Constitution

## Core Principles

### I. KISS-First Engineering
All implementation decisions MUST prefer the simplest design that satisfies current
requirements for this hobby project. New abstractions, layers, or dependencies MUST
be introduced only when a concrete and present maintenance problem exists. Any
complexity added MUST be justified in the plan's complexity tracking section.
Rationale: the project spans embedded firmware and a web UI; unnecessary complexity
raises failure risk and slows iteration.

### II. Code Quality Baseline
Changes MUST keep code readable, consistent, and maintainable across master, slave,
and web components. New code MUST use clear naming, bounded function size, and avoid
duplicate logic when a small shared helper is sufficient. Compiler warnings and
linting issues introduced by a change MUST be resolved before merge.
Rationale: quality issues compound quickly in mixed embedded/frontend codebases.

### III. Test Evidence for Every Change
Every behavior change MUST include verification evidence before merge. At minimum,
contributors MUST provide one of: automated tests, simulation checks, or documented
hardware/manual validation steps with expected outcomes. Bug fixes SHOULD include a
regression test when feasible without disproportionate setup overhead.
Rationale: hardware-coupled work cannot always be fully unit-tested, but every
change still needs explicit proof of correctness.

### IV. Consistent User Experience
User-visible behavior in the web interface and clock operation MUST remain coherent:
terminology, controls, and mode behavior should not drift between releases. Any
intentional UX change MUST update relevant documentation and include acceptance
criteria that describe the new behavior.
Rationale: predictability is essential for calibration, configuration, and daily use.

### V. Performance Within Hardware Limits
Changes MUST preserve responsive animation and control loops on target hardware.
Implementations MUST define measurable performance expectations (for example update
latency, frame smoothness, or loop timing) and verify they are met during validation.
Performance optimizations MUST stay simple and avoid speculative micro-optimizations.
Rationale: constrained microcontrollers require discipline, but KISS still applies.

## Project Constraints (Hobby Scope)

This repository is maintained as a personal, non-commercial hobby project.
Contributors MUST optimize for reliability and clarity over enterprise-level process
overhead. Preferred tools and patterns are those that are easy to run locally on
macOS/Linux with PlatformIO and basic web tooling.

## Development Workflow & Quality Gates

Planning artifacts (spec, plan, tasks) MUST explicitly cover code quality checks,
testing approach, UX impact, and performance validation for each feature.

Before implementation starts, the plan's Constitution Check MUST pass these gates:
- KISS justification documented for any new dependency or abstraction.
- Quality strategy documented (format/lint/build checks).
- Test evidence strategy documented per user story.
- UX consistency impact assessed for any user-facing behavior.
- Performance expectations and validation method defined.

Before merge, the implementation MUST provide:
- Traceable completion of required checks from the approved plan.
- Updated docs for any changed setup, UX flow, or operating behavior.
- A short validation summary in the PR/commit notes.

## Governance

This constitution supersedes informal local practices for feature planning and
delivery in this repository.

Amendment process:
- Amendments MUST be proposed in a pull request with rationale and affected templates.
- The project maintainer MUST explicitly approve the amendment.
- A Sync Impact Report MUST be included in the constitution update commit.

Versioning policy:
- MAJOR: incompatible governance or principle removals/redefinitions.
- MINOR: new principle/section or materially expanded guidance.
- PATCH: wording clarifications and non-semantic refinements.

Compliance review expectations:
- Every feature plan MUST include and pass a Constitution Check before implementation.
- Every pull request SHOULD be reviewed against all five core principles.
- Violations MUST be documented in Complexity Tracking with a time-bound follow-up.

**Version**: 1.0.0 | **Ratified**: 2026-03-30 | **Last Amended**: 2026-03-30
