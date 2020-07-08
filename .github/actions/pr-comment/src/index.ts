import * as core from "@actions/core";
import * as github from "@actions/github";

async function run() {
  const token = core.getInput("token");

  const octokit = new github.GitHub(token);
  const context = github.context;

  await octokit.issues.createComment({
    ...context.repo,
    issue_number: context.payload.pull_request!.number,
    body: `${core.getInput("comment")}`
  });
}

run();
