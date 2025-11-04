#!/bin/bash
# Auto sync script - commit and push all changes

echo "Auto syncing to GitHub..."

# Add all changes
git add .

# Check if there are changes to commit
if git diff --staged --quiet; then
    echo "No changes to commit"
    exit 0
fi

# Commit with timestamp
commit_msg="Auto-sync: $(date '+%Y-%m-%d %H:%M:%S')"
git commit -m "$commit_msg"

# Push to GitHub
git push origin main

echo "Successfully synced to GitHub!"