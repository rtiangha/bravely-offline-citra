#!/bin/bash -ex

GITDATE="`git show -s --date=short --format='%ad' | sed 's/-//g'`"
GITREV="`git show -s --format='%h'`"

# Determine the name of the release being built.
if [[ "$GITHUB_REF_TYPE" == "tag" ]]; then
    RELEASE_NAME="citra-unified-source-$GITHUB_REF_NAME"
else
    RELEASE_NAME="citra-unified-source-${GITDATE}-${GITREV}"
fi

COMPAT_LIST='dist/compatibility_list/compatibility_list.json'

mkdir artifacts

pip3 install git-archive-all
touch "${COMPAT_LIST}"
git describe --abbrev=0 --always HEAD > GIT-COMMIT
git describe --tags HEAD > GIT-TAG || echo 'unknown' > GIT-TAG
git archive-all --include "${COMPAT_LIST}" --include GIT-COMMIT --include GIT-TAG --force-submodules artifacts/"${RELEASE_NAME}.tar"

cd artifacts/
xz -T0 -9 "${RELEASE_NAME}.tar"
sha256sum "${RELEASE_NAME}.tar.xz" > "${RELEASE_NAME}.tar.xz.sha256sum"
cd ..
