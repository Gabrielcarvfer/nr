#! /usr/bin/env python3

# Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
#
# SPDX-License-Identifier: GPL-2.0-only

import datetime
import os
import shutil

# Check for dependencies that are not in the standard Python libraries
try:
    import git.exc
    from git import Repo
except ImportError:
    raise Exception("Missing pip package 'GitPython'.")

if shutil.which("git") is None:
    raise Exception("Missing program 'git'.")

# Define global variables
NS3_REPOSITORY = "https://gitlab.com/nsnam/ns-3-dev"
NR_DIR = os.path.abspath(os.path.join(__file__, "../../"))
NS3_DIR = os.path.join(NR_DIR, "ns-3-dev")

# Clone ns-3-dev or ensure it is updated
if not os.path.exists(NS3_DIR):
    ns3_repo = Repo.clone_from(NS3_REPOSITORY, NS3_DIR, branch="master")
else:
    ns3_repo = Repo(NS3_DIR)
    for remote in ns3_repo.remotes:
        remote.fetch()

# Get sorted commits from latest to oldest
tags = list(sorted(ns3_repo.tags, key=lambda x: x.commit.committed_date, reverse=True))

# Check if we are still within the grace period of 45 days after the latest ns-3-dev release
# If yes, then checkout the latest stable release, otherwise, use master
delta_since_release = (
    datetime.datetime.now(datetime.timezone.utc) - tags[0].commit.committed_datetime
)
if delta_since_release.days < 45:
    ns3_repo.git.checkout(tags[0].name)
