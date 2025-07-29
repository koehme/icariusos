#!/bin/bash

TODAY=$(date +"%Y-%m-%d")
read -p "(HH:MM, z.B. 20:30): " NEW_TIME

if [[ ! "$NEW_TIME" =~ ^([01][0-9]|2[0-3]):([0-5][0-9])$ ]]; then
    echo "❌"
    exit 1
fi

read -p "Commit-Text: " COMMIT_MSG


if [[ -z "$COMMIT_MSG" ]]; then
    echo "❌"
    exit 1
fi

DATETIME="${TODAY}T${NEW_TIME}:00"
GIT_AUTHOR_DATE="$DATETIME" GIT_COMMITTER_DATE="$DATETIME" git commit -m "$COMMIT_MSG"

echo "✅ $DATETIME"