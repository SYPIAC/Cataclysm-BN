# Daily Work Routine Start

Start the daily work routine for Cataclysm: Bright Nights development.

## Steps

1. **Check current status**: Run `git status` to see if there are any uncommitted changes or conflicts
2. **Handle uncommitted changes**: If there are uncommitted changes, use `ask_user` tool to prompt with choices:
   - Commit changes
   - Stash changes
   - Keep uncommitted and pull anyway
   - Discard changes
   - Keep working on current branch
3. **Switch to main**: Run `git checkout main` (if not already on main)
4. **Pull latest changes**: Run `git pull origin main`
5. **Report status**: Show summary of updates and current branch status

## Important

- **Always use the `ask_user` tool** for user decisions, not plain text questions
- Provide clear choices when asking about uncommitted changes
- Check for conflicts during pull and handle appropriately

## Usage

When invoked, this skill will:
- Check for any leftover work from previous sessions
- Ensure you're on main branch with latest changes
- Provide a clean starting point for new work

## Conflict Resolution

If conflicts or uncommitted changes are detected, the skill will pause and ask the user for instructions rather than making assumptions.
