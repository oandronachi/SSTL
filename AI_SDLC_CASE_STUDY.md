# AI-Assisted SDLC Case Study: SSTL

## Goal
Build a static-allocation STL-like library for constrained C99/C++03 environments.

## Constraints
- No runtime dynamic allocation
- C99/C++03 compatibility
- Deterministic capacity handling
- Embedded/freestanding-friendly assumptions
- Tests and documentation required for public APIs

## AI Workflow
- PRD and test plan drafted with AI assistance
- Design reviewed through human-in-the-loop iteration
- Implementation generated incrementally
- Human review redirected outputs that violated constraints
- Tests and specifications used as guardrails

## Validation
- Build checks
- Unit/integration/end-to-end tests
- No-allocation design review
- API parity checks between C and C++

## Lessons for AI-native SDLC
- AI is useful only when constraints are explicit
- Context engineering matters more than prompting alone
- Guardrails must be executable where possible
- Human approval remains necessary for architecture-sensitive decisions
