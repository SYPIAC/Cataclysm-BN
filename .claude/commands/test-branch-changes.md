---
description: Run tests for changes on this branch
argument-hint: unit-tests
---

# Test Branch Changes

Tests changes made on the current branch by building and running relevant tests.

## Workflow

1. **Build test project**
   - Use MSBuild to compile: `Cataclysm-test-vcpkg-static`
   - Handle PDB locking issues by killing lingering processes if needed
   - Configuration: Release, Platform: x64

2. **Identify tests from branch**
   - Check git log for test files changed on current branch vs main
   - Look for `TEST_CASE` declarations in modified test files
   - Extract test tags from test case declarations

3. **Run relevant tests**
   - Execute `Cataclysm-test-vcpkg-static-Release-x64.exe` with appropriate filter
   - Use test tags found in step 2 (e.g., `[ramp]`, `[vehicle]`)
   - Report pass/fail status and assertion counts

## Commands

### Build tests
```powershell
# Kill any lingering build processes
Get-Process | Where-Object {$_.Name -match "(msbuild|cl)"} | ForEach-Object { Stop-Process -Id $_.Id -Force -ErrorAction SilentlyContinue }

# Build test project
& "E:\Progs\VSC 2026\MSBuild\Current\Bin\MSBuild.exe" ".\msvc-full-features\Cataclysm-vcpkg-static.sln" /t:Cataclysm-test-vcpkg-static /p:Configuration=Release /p:Platform=x64 /m /nologo
```

### Find test files changed on branch
```powershell
# Get test files modified on current branch
git diff --name-only main...HEAD | Select-String "^tests/"

# Or check recent commits
git log --oneline --name-status HEAD~10..HEAD | Select-String "test"
```

### Extract test tags
```powershell
# Find TEST_CASE declarations and their tags
Select-String "TEST_CASE" tests\<test_file>.cpp
```

### Run tests
```powershell
# Run by tag filter
.\Cataclysm-test-vcpkg-static-Release-x64.exe "[tag]"

# Run specific test case
.\Cataclysm-test-vcpkg-static-Release-x64.exe "test case name"
```

## Notes

- MSBuild path: `E:\Progs\VSC 2026\MSBuild\Current\Bin\MSBuild.exe`
- Test executable: `Cataclysm-test-vcpkg-static-Release-x64.exe` (in repo root)
- PDB locking errors: Kill `msbuild.exe` and `cl.exe` processes before retrying
- Incremental builds enabled via fixed timestamp in `msvc-full-features\prebuild.cmd`
- Test framework: Catch2
- Common tags: `[vehicle]`, `[grab]`, `[ramp]`, `[stairs]`, `[multi-tile]`

## Example Session

```
User: "Run tests for changes on this branch"

1. Build: MSBuild Cataclysm-test-vcpkg-static
2. Find tests: `git diff main...HEAD tests/` → `vehicle_grab_ramp_test.cpp`
3. Extract tags: `Select-String "TEST_CASE"` → found `[ramp]` tag
4. Run: `.\Cataclysm-test-vcpkg-static-Release-x64.exe "[ramp]"`
5. Report: "✅ All 8 tests passed (3893 assertions)"
```
