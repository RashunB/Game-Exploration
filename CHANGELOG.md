# Changelog

All notable changes to this project. Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- M1 code core, first pass, not yet compiled: `ATakeoverGameMode`, `ATakeoverPlayerController` (mapping context per pawn), `UTakeoverInputConfig`, `ATakeoverVehiclePawn` (Enhanced Input driving, chase camera, reset), `UTakeoverHandlingProfile` (runtime Chaos setters), `FTakeoverScoringModel` and `UTakeoverScoringConfig` (drift, donut, burnout, proximity, combo), `UTakeoverScoringComponent` (vehicle sampling, collision and crowd hit detection), `ATakeoverArena` (crowd spline with placeholder meshes), `ATakeoverDebugHUD`, `Takeover.Scoring.Model` automation spec.
- M0 repo setup: `Takeover.uproject` targeting UE 5.8, `Takeover` C++ module skeleton, game and editor targets, folder layout, git and Git LFS config, Unreal MCP config, `CLAUDE.md`, smoke automation test.

### Changed

- `PLAN.md` refined. Engine target moved from 5.7 to 5.8. See the plan's revision history.
