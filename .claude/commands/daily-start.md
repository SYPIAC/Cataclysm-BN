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
6. **Review work options**: Use `ask_user` to present task sources:
   - Personal ideas: `C:\Users\SYPIAC\Documents\Personal\Bright Nights\Bright Nights Ideas.md`
   - In-progress todos: `C:\Users\SYPIAC\Documents\Personal\Bright Nights\ToDo.md`
   - GitHub issues: Browse and update tracking file
   - Something specific (will describe)
7. **If browsing GitHub issues**:
   - Fetch recent open bugs with `github-mcp-server-list_issues` (labels: ["bug"], state: OPEN, orderBy: UPDATED_AT)
   - Display list in readable format (numbered list without truncation)
   - Update `C:\Users\SYPIAC\Documents\Personal\Bright Nights\Issues to look into.md` with hyperlinked issues
   - Organize into sections: Unknown, Easy, Hard

## Important

- **Always use the `ask_user` tool** for user decisions, not plain text questions
- Provide clear choices when asking about uncommitted changes
- Check for conflicts during pull and handle appropriately
- After syncing, help user decide what to work on by reviewing their task sources

## Usage

When invoked, this skill will:
- Check for any leftover work from previous sessions
- Ensure you're on main branch with latest changes
- Provide a clean starting point for new work

## Conflict Resolution

If conflicts or uncommitted changes are detected, the skill will pause and ask the user for instructions rather than making assumptions.
